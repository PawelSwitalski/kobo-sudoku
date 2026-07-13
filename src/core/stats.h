#pragma once
#include "core/types.h"
#include <string>

namespace sudoku::core {

struct DiffStats {
    uint32_t completed = 0;
    uint32_t bestSeconds = 0;   // 0 = no completion yet
    uint64_t totalSeconds = 0;  // average = totalSeconds / completed
};

// Per-difficulty play statistics (stats.json, FR-011). Updated only on
// completion; completions with hints or hidden timer still count (FR-012).
struct Stats {
    std::array<DiffStats, 3> byDifficulty{};

    DiffStats& at(Difficulty d) { return byDifficulty[static_cast<int>(d)]; }
    const DiffStats& at(Difficulty d) const { return byDifficulty[static_cast<int>(d)]; }

    void recordCompletion(Difficulty d, uint32_t seconds);

    std::string toJson() const;
    static Stats fromJson(const std::string& text);  // throws on invalid
};

// settings.json (FR-012). Unknown keys are preserved across rewrites.
struct Settings {
    bool showTimer = true;

    std::string toJson() const;
    static Settings fromJson(const std::string& text);  // throws on invalid

private:
    std::string extraJson_;  // unknown keys from the loaded file, re-merged on save
};

}  // namespace sudoku::core
