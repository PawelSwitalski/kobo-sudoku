#include "doctest/doctest.h"

#include "core/board.h"
#include "core/session.h"
#include "fixtures.h"

using namespace sudoku::core;

TEST_CASE("peer computation: row/col/box helpers and peer table") {
    CHECK(rowOf(0) == 0);
    CHECK(colOf(0) == 0);
    CHECK(boxOf(0) == 0);
    CHECK(rowOf(80) == 8);
    CHECK(colOf(80) == 8);
    CHECK(boxOf(80) == 8);
    CHECK(boxOf(40) == 4);   // center cell
    CHECK(boxOf(2 * 9 + 5) == 1);

    const auto& peers = peersOf(40);  // r4c4
    CHECK(peers.size() == 20);
    // no self, no duplicates
    for (size_t a = 0; a < peers.size(); ++a) {
        CHECK(peers[a] != 40);
        for (size_t b = a + 1; b < peers.size(); ++b)
            CHECK(peers[a] != peers[b]);
    }
    // every peer shares a unit
    for (int p : peers)
        CHECK((rowOf(p) == 4 || colOf(p) == 4 || boxOf(p) == 4));
    // spot checks
    const auto& p0 = peersOf(0);
    auto has = [&](int i) {
        for (int p : p0)
            if (p == i) return true;
        return false;
    };
    CHECK(has(8));    // same row
    CHECK(has(72));   // same column
    CHECK(has(20));   // same box (r2c2)
    CHECK(!has(30));  // unrelated
}

TEST_CASE("given cells are immutable (FR-004)") {
    Session s(fixturePuzzle());
    int given = 0;  // fixture cell 0 is a given '5'
    REQUIRE(s.cell(given).kind == CellKind::Given);

    CHECK(s.commitDigit(given, 7).empty());
    CHECK(s.clearCell(given).empty());
    CHECK(s.toggleMark(given, 3).empty());
    CHECK(s.cell(given).value == 5);
    CHECK(s.cell(given).kind == CellKind::Given);
}

TEST_CASE("value set and clear on empty cells") {
    Session s(fixturePuzzle());
    int i = 2;  // empty in fixture; solution digit is 4
    REQUIRE(s.cell(i).kind == CellKind::Empty);

    ChangeSet cs = s.commitDigit(i, 4);
    CHECK(cs.contains(i));
    CHECK(s.cell(i).kind == CellKind::Entry);
    CHECK(s.cell(i).value == 4);

    // committing the identical digit again changes nothing
    CHECK(s.commitDigit(i, 4).empty());

    // overwrite with a different digit
    cs = s.commitDigit(i, 8);
    CHECK(cs.contains(i));
    CHECK(s.cell(i).value == 8);

    cs = s.clearCell(i);
    CHECK(cs.contains(i));
    CHECK(s.cell(i).kind == CellKind::Empty);
    CHECK(s.cell(i).value == 0);

    // clearing an already-empty, unmarked cell is a no-op
    CHECK(s.clearCell(i).empty());

    // out-of-range guards
    CHECK(s.commitDigit(-1, 5).empty());
    CHECK(s.commitDigit(81, 5).empty());
    CHECK(s.commitDigit(i, 0).empty());
    CHECK(s.commitDigit(i, 10).empty());
}

TEST_CASE("completion detection (FR-009)") {
    Puzzle p = fixturePuzzle();
    Session s(p);
    CHECK(!s.isComplete());

    for (int i = 0; i < 81; ++i)
        if (s.cell(i).kind == CellKind::Empty) s.commitDigit(i, p.solution[i]);
    CHECK(s.isComplete());

    // one wrong digit breaks completion
    int entry = 2;
    s.commitDigit(entry, p.solution[entry] == 9 ? 1 : 9);
    CHECK(!s.isComplete());
}

TEST_CASE("error marking is against the solution (FR-007)") {
    Puzzle p = fixturePuzzle();
    Session s(p);
    int i = 2;  // solution 4

    CHECK(!s.isError(i));  // empty is never an error
    s.commitDigit(i, 4);
    CHECK(!s.isError(i));  // correct entry
    s.commitDigit(i, 8);   // 8 might be rule-consistent, but it is wrong
    CHECK(s.isError(i));
    s.clearCell(i);
    CHECK(!s.isError(i));
    CHECK(!s.isError(0));  // givens are never errors
}

TEST_CASE("hints fill one correct cell (FR-008)") {
    Puzzle p = fixturePuzzle();
    Session s(p);

    SUBCASE("hint fills the first empty cell when nothing is wrong") {
        ChangeSet cs = s.applyHint();
        REQUIRE(cs.cells.size() >= 1);
        int t = cs.cells[0];
        CHECK(s.cell(t).kind == CellKind::Entry);
        CHECK(s.cell(t).value == p.solution[t]);
        CHECK(s.cell(t).fromHint);
        CHECK(s.hintsUsed() == 1);
    }

    SUBCASE("hint prefers an erroneous cell") {
        int wrong = 10;  // empty in fixture (givens[10]==0)
        REQUIRE(p.givens[wrong] == 0);
        s.commitDigit(wrong, p.solution[wrong] == 9 ? 1 : 9);
        REQUIRE(s.isError(wrong));
        ChangeSet cs = s.applyHint();
        CHECK(cs.contains(wrong));
        CHECK(s.cell(wrong).value == p.solution[wrong]);
        CHECK(!s.isError(wrong));
        CHECK(s.cell(wrong).fromHint);
    }

    SUBCASE("hint on a complete board is a no-op") {
        for (int i = 0; i < 81; ++i)
            if (s.cell(i).kind == CellKind::Empty) s.commitDigit(i, p.solution[i]);
        REQUIRE(s.isComplete());
        uint8_t before = s.hintsUsed();
        CHECK(s.applyHint().empty());
        CHECK(s.hintsUsed() == before);
    }

    SUBCASE("hint never targets a correct entry") {
        int i = 2;
        s.commitDigit(i, p.solution[i]);
        ChangeSet cs = s.applyHint();
        CHECK(!cs.contains(i));
        CHECK(!s.cell(i).fromHint);
    }
}

TEST_CASE("manual overwrite of a hint cell clears fromHint") {
    Puzzle p = fixturePuzzle();
    Session s(p);
    ChangeSet cs = s.applyHint();
    int t = cs.cells[0];
    s.commitDigit(t, p.solution[t] == 9 ? 1 : 9);
    CHECK(!s.cell(t).fromHint);
}
