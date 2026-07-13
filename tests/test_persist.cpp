#include "doctest/doctest.h"

#include <cstdio>
#include <filesystem>

#include "core/session.h"
#include "core/stats.h"
#include "persist/store.h"
#include "fixtures.h"

using namespace sudoku::core;
namespace fs = std::filesystem;

namespace {
Session makeDirtySession() {
    Session s(fixturePuzzle());
    s.commitDigit(2, 4);            // correct entry
    s.commitDigit(3, 9);            // wrong entry (solution has 6)
    s.toggleMark(10, 1);
    s.toggleMark(10, 4);
    s.toggleMark(10, 8);
    s.applyHint();                  // fixes cell 3 (the error), sets fromHint
    s.tick(754);
    return s;
}
}  // namespace

TEST_CASE("session JSON round-trip is lossless (FR-010)") {
    Session a = makeDirtySession();
    std::string text = a.toJson("2026-07-13T10:00:00Z");
    Session b = Session::fromJson(text);

    CHECK(b.puzzle().givens == a.puzzle().givens);
    CHECK(b.puzzle().solution == a.puzzle().solution);
    CHECK(b.puzzle().difficulty == a.puzzle().difficulty);
    CHECK(b.puzzle().seed == a.puzzle().seed);
    CHECK(b.elapsedSeconds() == 754);
    CHECK(b.hintsUsed() == 1);
    for (int i = 0; i < 81; ++i) {
        CAPTURE(i);
        CHECK(a.cell(i) == b.cell(i));
    }
    // a second round-trip is byte-stable
    CHECK(Session::fromJson(b.toJson("2026-07-13T10:00:00Z")).toJson("x") == b.toJson("x"));
}

TEST_CASE("malformed save files are rejected (FR-018)") {
    Session good = makeDirtySession();
    std::string text = good.toJson();

    auto rejects = [](const std::string& t) {
        CHECK_THROWS_AS(Session::fromJson(t), std::exception);
    };

    rejects("");                       // empty
    rejects("{");                      // truncated JSON
    rejects("[]");                     // wrong root type
    rejects("{\"schemaVersion\":2}");  // unknown schema

    // structured corruption via targeted edits of a valid file
    auto corrupt = [&](const char* from, const char* to) {
        std::string t = text;
        size_t pos = t.find(from);
        REQUIRE(pos != std::string::npos);
        t.replace(pos, std::string(from).size(), to);
        return t;
    };

    // givens string wrong length
    rejects(corrupt("\"givens\":\"53", "\"givens\":\"5"));
    // non-digit in solution
    rejects(corrupt("\"solution\":\"53", "\"solution\":\"5x"));
    // cell value out of range
    rejects(corrupt("\"v\":4", "\"v\":12"));
    // difficulty not a known band
    rejects(corrupt("\"difficulty\":\"easy\"", "\"difficulty\":\"brutal\""));

    // entry at a given index
    {
        Grid gv = fixturePuzzle().givens;
        REQUIRE(gv[0] != 0);
        std::string t = text;
        size_t pos = t.find("\"cells\":[");
        REQUIRE(pos != std::string::npos);
        t.insert(pos + 9, "{\"i\":0,\"v\":5},");
        rejects(t);
    }
    // v and m together
    {
        std::string t = text;
        size_t pos = t.find("\"cells\":[");
        REQUIRE(pos != std::string::npos);
        t.insert(pos + 9, "{\"i\":11,\"v\":5,\"m\":[1,2]},");
        rejects(t);
    }
    // solution inconsistent with givens
    {
        // fixture given 5 at index 0; claim the solution starts with 6 there
        std::string t = corrupt("\"solution\":\"534", "\"solution\":\"634");
        rejects(t);
    }
}

TEST_CASE("atomic write: correct contents, tmp cleaned up, overwrite works") {
    fs::path dir = fs::temp_directory_path() / "kobo-sudoku-test";
    fs::create_directories(dir);
    std::string path = (dir / "save.json").string();

    CHECK(sudoku::persist::saveFileAtomic(path, "first"));
    auto r1 = sudoku::persist::loadFile(path);
    REQUIRE(r1.has_value());
    CHECK(*r1 == "first");
    CHECK(!fs::exists(path + ".tmp"));

    CHECK(sudoku::persist::saveFileAtomic(path, "second, longer contents"));
    auto r2 = sudoku::persist::loadFile(path);
    REQUIRE(r2.has_value());
    CHECK(*r2 == "second, longer contents");
    CHECK(!fs::exists(path + ".tmp"));

    sudoku::persist::removeFile(path);
    CHECK(!fs::exists(path));
    CHECK(!sudoku::persist::loadFile(path).has_value());  // missing => nullopt
    sudoku::persist::removeFile(path);                    // idempotent
    fs::remove_all(dir);
}

TEST_CASE("stats: completion-only updates and best/average math (FR-011)") {
    Stats st;
    CHECK(st.at(Difficulty::Easy).completed == 0);
    CHECK(st.at(Difficulty::Easy).bestSeconds == 0);

    st.recordCompletion(Difficulty::Easy, 600);
    st.recordCompletion(Difficulty::Easy, 340);   // better
    st.recordCompletion(Difficulty::Easy, 900);   // worse
    CHECK(st.at(Difficulty::Easy).completed == 3);
    CHECK(st.at(Difficulty::Easy).bestSeconds == 340);
    CHECK(st.at(Difficulty::Easy).totalSeconds == 1840);
    // other difficulties untouched
    CHECK(st.at(Difficulty::Medium).completed == 0);
    CHECK(st.at(Difficulty::Hard).completed == 0);

    // round-trip
    Stats st2 = Stats::fromJson(st.toJson());
    CHECK(st2.at(Difficulty::Easy).completed == 3);
    CHECK(st2.at(Difficulty::Easy).bestSeconds == 340);
    CHECK(st2.at(Difficulty::Easy).totalSeconds == 1840);

    // invalid input throws (callers fall back to defaults, FR-018)
    CHECK_THROWS_AS(Stats::fromJson("nonsense"), std::exception);
    CHECK_THROWS_AS(Stats::fromJson("{\"schemaVersion\":99}"), std::exception);
}

TEST_CASE("settings: defaults, round-trip, unknown-key preservation") {
    Settings def;
    CHECK(def.showTimer == true);

    Settings s = Settings::fromJson("{\"schemaVersion\":1,\"showTimer\":false}");
    CHECK(s.showTimer == false);
    Settings s2 = Settings::fromJson(s.toJson());
    CHECK(s2.showTimer == false);

    // missing showTimer key => default
    Settings s3 = Settings::fromJson("{\"schemaVersion\":1}");
    CHECK(s3.showTimer == true);

    // unknown keys survive a load-save cycle
    Settings s4 = Settings::fromJson(
        "{\"schemaVersion\":1,\"showTimer\":false,\"futureOption\":\"keep-me\"}");
    std::string out = s4.toJson();
    CHECK(out.find("futureOption") != std::string::npos);
    CHECK(out.find("keep-me") != std::string::npos);

    CHECK_THROWS_AS(Settings::fromJson("{broken"), std::exception);
}
