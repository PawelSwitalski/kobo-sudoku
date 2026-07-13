#include "doctest/doctest.h"

#include "core/board.h"
#include "core/generator.h"
#include "core/solver.h"

using namespace sudoku::core;

namespace {
constexpr int kSeedsPerDifficulty = 500;  // quickstart scenario 1 / SC-004
}

TEST_CASE("solver: countSolutions basics") {
    Grid empty{};
    CHECK(countSolutions(empty, 2) == 2);  // early exit at the limit

    // a full valid grid has exactly one solution (itself)
    uint64_t rng = 7;
    Grid full{};
    REQUIRE(fillGrid(full, rng));
    CHECK(isSolvedGrid(full));
    CHECK(countSolutions(full, 2) == 1);

    // a contradictory grid has none
    Grid bad{};
    bad[0] = 5;
    bad[1] = 5;
    CHECK(countSolutions(bad, 2) == 0);
}

TEST_CASE("generator: unique solution, givens subset, valid solution (SC-004)") {
    for (Difficulty d : {Difficulty::Easy, Difficulty::Medium, Difficulty::Hard}) {
        CAPTURE(static_cast<int>(d));
        for (uint64_t seed = 1; seed <= kSeedsPerDifficulty; ++seed) {
            CAPTURE(seed);
            Puzzle p = generate(d, seed);

            REQUIRE(isSolvedGrid(p.solution));
            // every given matches the solution
            int clues = 0;
            for (int i = 0; i < 81; ++i) {
                if (!p.givens[i]) continue;
                ++clues;
                REQUIRE(p.givens[i] == p.solution[i]);
            }
            CHECK(clues >= 17);  // fewer than 17 clues can never be unique
            // exactly one solution
            REQUIRE(countSolutions(p.givens, 2) == 1);
            CHECK(p.difficulty == d);
            CHECK(p.seed == seed);
        }
    }
}

TEST_CASE("generator: deterministic per (difficulty, seed)") {
    for (Difficulty d : {Difficulty::Easy, Difficulty::Medium, Difficulty::Hard}) {
        for (uint64_t seed : {1ULL, 42ULL, 987654321ULL, 0xDEADBEEFULL}) {
            Puzzle a = generate(d, seed);
            Puzzle b = generate(d, seed);
            CHECK(a.givens == b.givens);
            CHECK(a.solution == b.solution);
        }
    }
}

TEST_CASE("generator: different seeds give different puzzles") {
    Puzzle a = generate(Difficulty::Easy, 1);
    Puzzle b = generate(Difficulty::Easy, 2);
    CHECK(a.givens != b.givens);
}
