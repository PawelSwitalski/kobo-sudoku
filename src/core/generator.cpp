#include "core/generator.h"
#include "core/board.h"
#include "core/difficulty.h"
#include "core/solver.h"

namespace sudoku::core {

namespace {

// Stop digging once this many givens remain (keeps Easy gentle, lets Hard
// dig deep). Indexed by Difficulty.
constexpr int kClueFloor[3] = {38, 30, 22};

}  // namespace

Puzzle generate(Difficulty target, uint64_t seed) {
    const int floor = kClueFloor[static_cast<int>(target)];

    for (uint64_t attempt = 0;; ++attempt) {
        // Fresh deterministic stream per attempt.
        uint64_t rng = seed ^ (0xA5A5A5A5A5A5A5A5ULL + attempt * 0x9E3779B97F4A7C15ULL);

        Grid solution{};
        if (!fillGrid(solution, rng)) continue;  // cannot happen from empty, but stay safe

        Grid puzzle = solution;
        int order[81];
        for (int i = 0; i < 81; ++i) order[i] = i;
        for (int k = 80; k > 0; --k) {
            int j = static_cast<int>(nextRandom(rng) % static_cast<uint64_t>(k + 1));
            int t = order[k]; order[k] = order[j]; order[j] = t;
        }

        int clues = 81;
        Difficulty current = Difficulty::Easy;  // grade of a nearly-full grid
        for (int k = 0; k < 81; ++k) {
            if (clues <= floor && current == target) break;
            int idx = order[k];
            uint8_t saved = puzzle[idx];
            puzzle[idx] = 0;
            if (countSolutions(puzzle, 2) != 1) {
                puzzle[idx] = saved;
                continue;
            }
            if (target != Difficulty::Hard) {
                Difficulty g = grade(puzzle);
                if (g > target) {  // this removal would overshoot the band
                    puzzle[idx] = saved;
                    continue;
                }
                current = g;
            }
            --clues;
        }

        if (grade(puzzle) == target)
            return Puzzle{puzzle, solution, target, seed};
        // Off-band (e.g. a full dig still solvable by the Medium ladder when
        // Hard was requested): retry with the next deterministic sub-stream.
    }
}

}  // namespace sudoku::core
