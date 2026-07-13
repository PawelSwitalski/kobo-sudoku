#pragma once
#include "core/types.h"

namespace sudoku::core {

enum class CellKind : uint8_t { Empty, Given, Entry };

struct Cell {
    CellKind kind = CellKind::Empty;
    uint8_t value = 0;    // 0 iff kind == Empty
    uint16_t marks = 0;   // bit d-1 set = candidate d penciled in
    bool fromHint = false;

    bool hasMark(int d) const { return (marks >> (d - 1)) & 1; }
    bool operator==(const Cell& o) const {
        return kind == o.kind && value == o.value && marks == o.marks && fromHint == o.fromHint;
    }
};

inline int rowOf(int i) { return i / 9; }
inline int colOf(int i) { return i % 9; }
inline int boxOf(int i) { return (i / 27) * 3 + (i % 9) / 3; }

// Fixed in-cell pencil-mark layout (FR-005): digit d always renders at
// row 0..2 / col 0..2 of the in-cell 3x3 grid — 1-3 top, 4-6 middle, 7-9 bottom.
constexpr int markSlotRow(int d) { return (d - 1) / 3; }
constexpr int markSlotCol(int d) { return (d - 1) % 3; }

// The 20 distinct peers of a cell (same row, column, or box; excludes self).
const std::array<int, 20>& peersOf(int i);

// True if the completed-or-partial grid violates no sudoku rule.
bool isValidGrid(const Grid& g);
// True if g is completely filled and valid.
bool isSolvedGrid(const Grid& g);

}  // namespace sudoku::core
