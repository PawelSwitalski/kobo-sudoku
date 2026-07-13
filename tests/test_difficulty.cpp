#include "doctest/doctest.h"

#include "core/difficulty.h"
#include "core/generator.h"
#include "core/solver.h"
#include "fixtures.h"

using namespace sudoku::core;

TEST_CASE("technique ladder ordering") {
    CHECK(Difficulty::Easy < Difficulty::Medium);
    CHECK(Difficulty::Medium < Difficulty::Hard);
}

TEST_CASE("a nearly-full grid grades Easy (naked singles only)") {
    Puzzle p = fixturePuzzle();
    Grid g = p.solution;
    // Punch a few isolated holes: each remaining cell pins them as naked singles.
    g[0] = g[40] = g[80] = g[12] = 0;
    bool usedTier1 = true;
    CHECK(logicalSolve(g, 1, &usedTier1));
    CHECK(!usedTier1);
    CHECK(grade(g) == Difficulty::Easy);
}

TEST_CASE("known hard fixture is beyond the Medium ladder (Inkala 2012)") {
    // Arto Inkala's 2012 "world's hardest sudoku" - requires far more than
    // singles/locked candidates/naked pairs.
    Grid g = gridFromString(
        "800000000003600000070090200050007000000045700000100030001000068008500010090000400");
    REQUIRE(countSolutions(g, 2) == 1);  // guards against fixture typos
    CHECK(!logicalSolve(g, 1));
    CHECK(grade(g) == Difficulty::Hard);
}

TEST_CASE("grading is consistent with the logical solver") {
    // For each band: Easy solves with tier 0; Medium needs tier 1 but solves
    // with it; Hard fails even tier 1.
    for (uint64_t seed = 1; seed <= 25; ++seed) {
        CAPTURE(seed);
        Puzzle e = generate(Difficulty::Easy, seed);
        CHECK(logicalSolve(e.givens, 0));

        Puzzle m = generate(Difficulty::Medium, seed);
        CHECK(!logicalSolve(m.givens, 0));
        bool usedTier1 = false;
        CHECK(logicalSolve(m.givens, 1, &usedTier1));
        CHECK(usedTier1);

        Puzzle h = generate(Difficulty::Hard, seed);
        CHECK(!logicalSolve(h.givens, 1));
    }
}

TEST_CASE("generated puzzles grade in-band (FR-002)") {
    for (Difficulty d : {Difficulty::Easy, Difficulty::Medium, Difficulty::Hard})
        for (uint64_t seed = 100; seed < 150; ++seed) {
            CAPTURE(static_cast<int>(d));
            CAPTURE(seed);
            CHECK(generate(d, seed).difficulty == d);
            CHECK(grade(generate(d, seed).givens) == d);
        }
}
