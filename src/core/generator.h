#pragma once
#include "core/types.h"

namespace sudoku::core {

struct Puzzle {
    Grid givens{};
    Grid solution{};
    Difficulty difficulty = Difficulty::Easy;
    uint64_t seed = 0;
};

// Generates a puzzle with exactly one solution at the requested difficulty
// (SC-004, FR-002). Deterministic for a given (difficulty, seed) pair.
// Must stay well under the 5 s budget (SC-001; target < 500 ms typical).
Puzzle generate(Difficulty d, uint64_t seed);

}  // namespace sudoku::core
