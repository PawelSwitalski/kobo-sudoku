#include "core/stats.h"

#include <stdexcept>

#include "core/board.h"
#include "nlohmann/json.hpp"

namespace sudoku::core {

using nlohmann::json;

void Stats::recordCompletion(Difficulty d, uint32_t seconds) {
    DiffStats& r = at(d);
    ++r.completed;
    r.totalSeconds += seconds;
    if (r.bestSeconds == 0 || seconds < r.bestSeconds) r.bestSeconds = seconds;
}

std::string Stats::toJson() const {
    json diffs;
    for (int d = 0; d < 3; ++d) {
        const DiffStats& r = byDifficulty[d];
        diffs[difficultyName(static_cast<Difficulty>(d))] = {
            {"completed", r.completed},
            {"bestSeconds", r.bestSeconds},
            {"totalSeconds", r.totalSeconds}};
    }
    return json{{"schemaVersion", 1}, {"difficulties", std::move(diffs)}}.dump();
}

Stats Stats::fromJson(const std::string& text) {
    json j = json::parse(text);
    if (!j.is_object() || j.value("schemaVersion", 0) != 1)
        throw std::runtime_error("invalid stats: schemaVersion");
    Stats s;
    const json& diffs = j.at("difficulties");
    for (int d = 0; d < 3; ++d) {
        const char* key = difficultyName(static_cast<Difficulty>(d));
        if (!diffs.contains(key)) continue;  // missing difficulty keeps defaults
        const json& r = diffs.at(key);
        DiffStats& out = s.byDifficulty[d];
        out.completed = r.at("completed").get<uint32_t>();
        out.bestSeconds = r.at("bestSeconds").get<uint32_t>();
        out.totalSeconds = r.at("totalSeconds").get<uint64_t>();
    }
    return s;
}

std::string Settings::toJson() const {
    json j = json::object();
    if (!extraJson_.empty()) {
        json extra = json::parse(extraJson_, nullptr, false);
        if (extra.is_object()) j = std::move(extra);  // unknown keys preserved
    }
    j["schemaVersion"] = 1;
    j["showTimer"] = showTimer;
    return j.dump();
}

Settings Settings::fromJson(const std::string& text) {
    json j = json::parse(text);
    if (!j.is_object() || j.value("schemaVersion", 0) != 1)
        throw std::runtime_error("invalid settings: schemaVersion");
    Settings s;
    s.showTimer = j.value("showTimer", true);
    j.erase("schemaVersion");
    j.erase("showTimer");
    if (!j.empty()) s.extraJson_ = j.dump();
    return s;
}

}  // namespace sudoku::core
