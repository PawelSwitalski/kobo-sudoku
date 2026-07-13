#include "doctest/doctest.h"

#include "core/board.h"
#include "core/session.h"
#include "fixtures.h"

using namespace sudoku::core;

TEST_CASE("fixed mark-slot layout: 1-3 top, 4-6 middle, 7-9 bottom (FR-005)") {
    for (int d = 1; d <= 9; ++d) {
        CHECK(markSlotRow(d) == (d - 1) / 3);
        CHECK(markSlotCol(d) == (d - 1) % 3);
    }
    CHECK(markSlotRow(1) == 0); CHECK(markSlotCol(1) == 0);
    CHECK(markSlotRow(3) == 0); CHECK(markSlotCol(3) == 2);
    CHECK(markSlotRow(5) == 1); CHECK(markSlotCol(5) == 1);
    CHECK(markSlotRow(7) == 2); CHECK(markSlotCol(7) == 0);
    CHECK(markSlotRow(9) == 2); CHECK(markSlotCol(9) == 2);
}

TEST_CASE("toggling marks on and off, several at once (FR-005)") {
    Session s(fixturePuzzle());
    int i = 2;  // empty
    REQUIRE(s.cell(i).kind == CellKind::Empty);

    CHECK(s.toggleMark(i, 2).contains(i));
    CHECK(s.toggleMark(i, 5).contains(i));
    CHECK(s.toggleMark(i, 9).contains(i));
    CHECK(s.cell(i).hasMark(2));
    CHECK(s.cell(i).hasMark(5));
    CHECK(s.cell(i).hasMark(9));
    CHECK(!s.cell(i).hasMark(1));
    CHECK(s.cell(i).marks == ((1u << 1) | (1u << 4) | (1u << 8)));

    // toggle one off, others stay
    CHECK(s.toggleMark(i, 5).contains(i));
    CHECK(!s.cell(i).hasMark(5));
    CHECK(s.cell(i).hasMark(2));
    CHECK(s.cell(i).hasMark(9));

    // digit guards
    CHECK(s.toggleMark(i, 0).empty());
    CHECK(s.toggleMark(i, 10).empty());
}

TEST_CASE("marks live only in empty cells") {
    Session s(fixturePuzzle());
    CHECK(s.toggleMark(0, 3).empty());  // given
    int i = 2;
    s.commitDigit(i, 4);
    CHECK(s.toggleMark(i, 3).empty());  // entry
    CHECK(s.cell(i).marks == 0);
}

TEST_CASE("committing clears the cell's own marks (FR-006)") {
    Session s(fixturePuzzle());
    int i = 2;
    s.toggleMark(i, 2);
    s.toggleMark(i, 7);
    s.commitDigit(i, 4);
    CHECK(s.cell(i).marks == 0);
    // clearing the entry does not resurrect marks
    s.clearCell(i);
    CHECK(s.cell(i).marks == 0);
}

TEST_CASE("clearCell on an empty cell erases its marks") {
    Session s(fixturePuzzle());
    int i = 2;
    s.toggleMark(i, 2);
    s.toggleMark(i, 7);
    ChangeSet cs = s.clearCell(i);
    CHECK(cs.contains(i));
    CHECK(s.cell(i).marks == 0);
}

TEST_CASE("peer auto-cleanup on commit, with exact ChangeSet (FR-006a)") {
    Session s(fixturePuzzle());
    int target = 2;     // r0c2, box 0; committed digit 4
    int rowPeer = 8;    // r0c8 - same row only, empty in fixture
    int colPeer = 47;   // r5c2 - same column only, empty in fixture
    int boxPeer = 18;   // r2c0 - same box only, empty in fixture
    int farCell = 40;   // r4c4 - shares no unit with the target

    REQUIRE(fixturePuzzle().givens[rowPeer] == 0);
    REQUIRE(fixturePuzzle().givens[colPeer] == 0);
    REQUIRE(fixturePuzzle().givens[boxPeer] == 0);

    s.toggleMark(rowPeer, 4);
    s.toggleMark(colPeer, 4);
    s.toggleMark(boxPeer, 4);
    s.toggleMark(rowPeer, 6);  // a different candidate must survive
    s.toggleMark(farCell, 4);  // not a peer: must survive

    ChangeSet cs = s.commitDigit(target, 4);
    CHECK(cs.contains(target));
    CHECK(cs.contains(rowPeer));
    CHECK(cs.contains(colPeer));
    CHECK(cs.contains(boxPeer));
    CHECK(!cs.contains(farCell));
    CHECK(cs.cells.size() == 4);  // target + exactly the three affected peers

    CHECK(!s.cell(rowPeer).hasMark(4));
    CHECK(s.cell(rowPeer).hasMark(6));
    CHECK(!s.cell(colPeer).hasMark(4));
    CHECK(!s.cell(boxPeer).hasMark(4));
    CHECK(s.cell(farCell).hasMark(4));
}

TEST_CASE("peers without the committed candidate are not in the ChangeSet") {
    Session s(fixturePuzzle());
    int target = 2, peer = 8;
    s.toggleMark(peer, 6);  // candidate != committed digit
    ChangeSet cs = s.commitDigit(target, 4);
    CHECK(!cs.contains(peer));
    CHECK(s.cell(peer).hasMark(6));
}

TEST_CASE("hints also clean peer marks") {
    Puzzle p = fixturePuzzle();
    Session s(p);
    int firstEmpty = 2;  // hint target; solution digit 4
    int peer = 8;
    s.toggleMark(peer, static_cast<int>(p.solution[firstEmpty]));
    ChangeSet cs = s.applyHint();
    CHECK(cs.contains(firstEmpty));
    CHECK(cs.contains(peer));
    CHECK(!s.cell(peer).hasMark(p.solution[firstEmpty]));
}
