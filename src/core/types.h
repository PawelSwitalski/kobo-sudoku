#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace sudoku::core {

enum class Difficulty : uint8_t { Easy = 0, Medium = 1, Hard = 2 };

// Row-major 9x9 grid; 0 = empty, 1..9 = digit.
using Grid = std::array<uint8_t, 81>;

// Indices of cells whose rendering changed after a mutation, so the UI can
// limit e-ink partial refreshes to exactly those cells (Constitution II).
struct ChangeSet {
    std::vector<int> cells;
    bool empty() const { return cells.empty(); }
    bool contains(int i) const {
        for (int c : cells)
            if (c == i) return true;
        return false;
    }
};

const char* difficultyName(Difficulty d);  // "easy" | "medium" | "hard"
bool difficultyFromString(std::string_view s, Difficulty& out);

}  // namespace sudoku::core
