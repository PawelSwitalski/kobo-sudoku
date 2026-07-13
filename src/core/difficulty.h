#pragma once
#include "core/types.h"

namespace sudoku::core {

// Logical-technique ladder (research R6):
//   tier 0: naked singles, hidden singles            -> Easy
//   tier 1: + locked candidates, naked pairs         -> Medium
//   beyond the ladder (unique but not tier-solvable) -> Hard
//
// Tries to solve g using techniques up to maxTier (0 or 1). Returns true if
// fully solved; if usedTier1 is non-null it reports whether any tier-1
// technique was required.
bool logicalSolve(Grid g, int maxTier, bool* usedTier1 = nullptr);

// Grades by the hardest technique required. Assumes the puzzle has a solution.
Difficulty grade(const Grid& givens);

}  // namespace sudoku::core
