#pragma once
#include "core/board.h"
#include "core/generator.h"
#include <string>

namespace sudoku::core {

// The single in-progress game. Pure logic — no I/O, no OS calls; persistence
// happens through toJson()/fromJson() driven by the persist layer.
class Session {
public:
    Session() = default;
    explicit Session(Puzzle p);

    // Cell-first mutators (FR-004). Mutations on Given cells are no-ops that
    // return an empty ChangeSet. See contracts/core-model.md.
    ChangeSet commitDigit(int cell, int digit);  // clears own+peer marks (FR-006/006a)
    ChangeSet clearCell(int cell);               // Entry value, or marks of an Empty cell
    ChangeSet toggleMark(int cell, int digit);   // Empty cells only (FR-005)
    ChangeSet applyHint();                       // fills one empty-or-wrong cell (FR-008)

    bool isError(int i) const;   // Entry value != solution (FR-007)
    bool isComplete() const;     // every cell matches the solution (FR-009)

    const Cell& cell(int i) const { return cells_[i]; }
    const Puzzle& puzzle() const { return puzzle_; }
    uint32_t elapsedSeconds() const { return elapsedSeconds_; }
    void tick(uint32_t activeSeconds) { elapsedSeconds_ += activeSeconds; }
    uint8_t hintsUsed() const { return hintsUsed_; }

    // save-format.md schema v1. savedAt is injected by the caller so the core
    // stays clock-free; empty means "omit".
    std::string toJson(const std::string& savedAtIso = "") const;
    // Throws std::runtime_error on any malformed/inconsistent input (FR-018:
    // the caller treats that as "no save").
    static Session fromJson(const std::string& text);

private:
    Puzzle puzzle_{};
    std::array<Cell, 81> cells_{};
    uint32_t elapsedSeconds_ = 0;
    uint8_t hintsUsed_ = 0;
};

}  // namespace sudoku::core
