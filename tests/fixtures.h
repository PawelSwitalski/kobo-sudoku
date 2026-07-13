#pragma once
#include "core/generator.h"
#include <cstring>

// Shared test fixture: the classic puzzle from contracts/save-format.md.
inline sudoku::core::Puzzle fixturePuzzle() {
    static const char* kGivens =
        "530070000600195000098000060800060003400803001700020006060000280000419005000080079";
    static const char* kSolution =
        "534678912672195348198342567859761423426853791713924856961537284287419635345286179";
    sudoku::core::Puzzle p;
    for (int i = 0; i < 81; ++i) {
        p.givens[i] = static_cast<uint8_t>(kGivens[i] - '0');
        p.solution[i] = static_cast<uint8_t>(kSolution[i] - '0');
    }
    p.difficulty = sudoku::core::Difficulty::Easy;
    p.seed = 4242;
    return p;
}

inline sudoku::core::Grid gridFromString(const char* s) {
    sudoku::core::Grid g{};
    for (int i = 0; i < 81; ++i)
        g[i] = static_cast<uint8_t>(s[i] - '0');
    return g;
}
