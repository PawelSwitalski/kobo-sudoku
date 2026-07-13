#pragma once
#include "core/types.h"

namespace sudoku::core {

// Deterministic PRNG step (splitmix64). All core randomness goes through this
// so puzzle generation is reproducible across platforms (std::shuffle is not).
uint64_t nextRandom(uint64_t& state);

// Completes g into a full valid grid (randomized backtracking).
// Returns false if the partial grid is unsolvable.
bool fillGrid(Grid& g, uint64_t& rngState);

// Number of solutions of g, stopping early once `limit` is reached.
int countSolutions(const Grid& g, int limit = 2);

}  // namespace sudoku::core
