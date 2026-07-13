# Feature Specification: Kobo Sudoku

**Feature Branch**: `001-kobo-sudoku`

**Created**: 2026-07-12

**Status**: Draft

**Input**: User description: "I would like to create sudoku game for my e-reader kobo (kobo libra color). Also if it's possible this game should works on other kobo. I actually have connected the e-reader to the computer disc d:. Probably the best language will be cpp. I think the best way will by use graphic libraries that offers kobo for it's own UI."

## Clarifications

### Session 2026-07-12

- Q: When should a placed digit be marked as an error? → A: Check against solution — any digit that differs from the puzzle's unique solution is flagged immediately, even without a visible row/column/box conflict.
- Q: Which digit-entry interaction model should the game use? → A: Cell-first — tap a cell to select it, then tap a digit on an always-visible 1–9 pad to place it.
- Q: When a full-size digit is placed, should matching pencil marks in the same row, column, and box be removed automatically? → A: Yes — placing a digit auto-removes that candidate from all cells in the same row, column, and 3×3 box.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Play a complete game of sudoku (Priority: P1)

The player opens the game on their Kobo e-reader, starts a new puzzle at a chosen difficulty, fills in digits by tapping cells and picking numbers, and completes the puzzle. The game recognizes the completed, correct solution and congratulates the player.

**Why this priority**: This is the game. Without the ability to start, play, and finish a puzzle, nothing else has value.

**Independent Test**: Can be fully tested by launching the game on the device, starting a new puzzle, solving it to completion, and observing the completion acknowledgement.

**Acceptance Scenarios**:

1. **Given** the game is open at the main menu, **When** the player chooses "New game" and a difficulty, **Then** a fresh 9×9 sudoku puzzle with exactly one solution is displayed, with the given (pre-filled) digits visually distinct from empty cells.
2. **Given** a puzzle is on screen with its always-visible 1–9 number pad, **When** the player taps an empty cell (selecting it) and then taps a digit on the pad, **Then** that digit appears in the selected cell as the player's entry.
3. **Given** a puzzle is on screen, **When** the player taps a cell containing their own entry and chooses to clear it, **Then** the cell becomes empty again. Given digits can never be changed or cleared.
4. **Given** all 81 cells are filled and every row, column, and 3×3 box contains the digits 1–9 exactly once, **When** the last correct digit is placed, **Then** the game shows a completion message (and the solve time, if the timer was enabled).

---

### User Story 2 - Pencil marks: note candidate digits in a cell (Priority: P2)

While solving, the player wants to note which digits are still possible in a cell before committing to one. In any empty cell they can enter several small candidate digits at once. Candidates render as small digits arranged in a fixed 3×3 layout inside the cell: 1–3 across the top, 4–6 across the middle, 7–9 across the bottom, each digit always in its own fixed position.

**Why this priority**: Pencil marks are essential to how most people solve medium and hard puzzles; without them the game is only playable at easy levels. It builds directly on the P1 entry mechanics.

**Independent Test**: Can be tested by opening any puzzle, adding multiple candidate digits to an empty cell, verifying their fixed positions in the cell, toggling one off, and then overwriting them with a normal (big) digit.

**Acceptance Scenarios**:

1. **Given** an empty cell is selected in pencil-mark mode, **When** the player selects digits 2, 5, and 9, **Then** the cell shows small digits 2 (top row), 5 (middle row), and 9 (bottom row), each at its fixed position for that digit.
2. **Given** a cell shows candidate digits, **When** the player selects an already-present candidate digit again, **Then** that candidate is removed while the others remain.
3. **Given** a cell shows candidate digits, **When** the player enters a normal (full-size) digit in that cell, **Then** the candidates in that cell are replaced by the full-size digit.
4. **Given** any cell with candidates, **When** the cell is displayed, **Then** candidate digits are clearly smaller than committed digits and all nine positions follow the 1–3 / 4–6 / 7–9 row layout.
5. **Given** several cells in a row, column, or 3×3 box show candidate digit 5, **When** the player commits a full-size 5 anywhere in that row, column, or box, **Then** the candidate 5s in those cells are removed automatically (other candidates remain).

---

### User Story 3 - Error highlighting and hints (Priority: P3)

When the player places a digit that does not match the puzzle's solution, the game visibly marks it as an error so the mistake is caught early (this covers both visible conflicts — the same digit twice in a row/column/box — and wrong-but-consistent digits). When the player is stuck, they can request a hint and the game fills in one correct digit in an empty (or incorrectly filled) cell.

**Why this priority**: These assists make the game friendly and self-correcting, but a playable game exists without them.

**Independent Test**: Can be tested by deliberately placing a conflicting digit and observing the highlight, then requesting a hint on a fresh puzzle and verifying one correct cell is filled.

**Acceptance Scenarios**:

1. **Given** a puzzle in progress, **When** the player places a digit that differs from the solution for that cell, **Then** that cell is visibly marked as an error (even if it creates no visible row/column/box conflict).
2. **Given** an error is highlighted, **When** the player removes or changes the offending digit to the correct one, **Then** the error marking disappears.
3. **Given** a puzzle in progress, **When** the player requests a hint, **Then** exactly one cell is filled with a digit from the true solution, and that cell is marked as hint-provided.
4. **Given** a completed, fully correct puzzle, **When** the player requests a hint, **Then** the game indicates no hint is needed.

---

### User Story 4 - Interrupt and resume play (Priority: P2)

E-reader sessions are casual: the player closes the game, the device sleeps or powers off, and days later they return. The game automatically preserves the in-progress puzzle — entries, pencil marks, elapsed time — and offers to continue exactly where they left off.

**Why this priority**: On an e-reader, sessions are short and interruptions are the norm. Losing a half-solved hard puzzle would make the game feel broken. Ranked with P2 because it's about retention of the P1 experience.

**Independent Test**: Can be tested by partially solving a puzzle, exiting the game (and power-cycling the device), relaunching, choosing "Continue", and verifying the board state matches exactly.

**Acceptance Scenarios**:

1. **Given** a puzzle in progress, **When** the player exits the game by any normal means, **Then** on next launch the main menu offers "Continue" restoring all entries, pencil marks, and elapsed time.
2. **Given** a puzzle in progress, **When** the device loses power unexpectedly, **Then** on next launch no more than the last move is lost.
3. **Given** a saved game exists, **When** the player instead starts a new game, **Then** the game asks for confirmation before discarding the saved puzzle.

---

### User Story 5 - Timer and play statistics (Priority: P3)

The player can optionally see how long the current puzzle is taking, and review simple lifetime statistics: puzzles completed and best/average solve times per difficulty. The timer display can be hidden for relaxed play while still being recorded.

**Why this priority**: Nice motivational layer on top of a complete game; entirely optional to the core experience.

**Independent Test**: Can be tested by solving one puzzle with the timer shown and one with it hidden, then checking the statistics screen reflects both completions with plausible times.

**Acceptance Scenarios**:

1. **Given** the timer display is enabled in settings, **When** a puzzle is in progress, **Then** the elapsed time is visible without disturbing play (it need not tick every second).
2. **Given** the timer display is disabled, **When** the player completes a puzzle, **Then** the solve time is still recorded in statistics.
3. **Given** at least one puzzle has been completed at a difficulty, **When** the player opens the statistics screen, **Then** it shows puzzles completed, best time, and average time for that difficulty.

---

### User Story 6 - Runs on the player's Kobo, and on other Kobo models where possible (Priority: P1)

The player installs the game onto their Kobo Libra Colour by copying files to the device over USB, then launches it from the device and plays comfortably: elements are sized for finger taps, the display is crisp on e-ink, and screen updates don't leave distracting ghosting. The same package should run on other common Kobo models (different screen sizes, monochrome screens) without modification where feasible.

**Why this priority**: If the game doesn't install, launch, and render well on the actual device, nothing else matters. Equal-first with User Story 1.

**Independent Test**: Can be tested by installing the package on a Kobo Libra Colour via USB, launching it on the device, and playing one full puzzle; portability is tested by repeating on at least one other model or screen size.

**Acceptance Scenarios**:

1. **Given** the game package and install instructions, **When** the player copies it to the device over USB and follows the launch steps, **Then** the game starts on the device without requiring a computer-side toolchain or developer knowledge.
2. **Given** the game is running on the Kobo Libra Colour, **When** any screen is shown, **Then** all interactive elements are comfortably tappable with a finger and all digits are legible at arm's length.
3. **Given** the player finishes the game session, **When** they exit, **Then** the device returns to its normal reading interface.
4. **Given** a Kobo model with a different screen size or a monochrome screen, **When** the game runs on it, **Then** the board scales to the screen and remains fully playable (color is never the only carrier of information).

---

### Edge Cases

- What happens when the player taps a given (pre-filled) digit? The cell may highlight for reference (e.g., showing same-digit cells) but must never be editable.
- What happens if puzzle generation at the requested difficulty takes noticeably long on the device's modest hardware? The game must show a busy indication and still deliver a puzzle within the success-criteria time bound.
- What happens when the player fills the last cell incorrectly (board full but wrong)? The wrong cells are already marked by solution-based error highlighting, and the game must not show the completion acknowledgement until every cell matches the solution.
- What happens when the device sleeps mid-game (e-ink sleep screen replaces the display)? On wake, the game screen must redraw correctly and the timer must not count sleep time as solving time.
- What happens if the saved-game file is missing or corrupted? The game must start cleanly at the main menu without crashing, offering a new game.
- What happens on very small Kobo screens (6")? The pencil-mark digits must remain distinguishable, or the layout must adapt while keeping the fixed 1–3/4–6/7–9 arrangement.
- What happens when hints are used to finish most of the puzzle? Completion is still acknowledged, but the completion message/statistics may flag the number of hints used.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST generate 9×9 sudoku puzzles on the device, each with exactly one valid solution.
- **FR-002**: System MUST offer three difficulty levels (Easy, Medium, Hard), where difficulty observably affects the number of givens and the solving techniques required.
- **FR-003**: System MUST display the 9×9 board with clearly visible 3×3 box boundaries, rendering given digits visually distinct from player-entered digits.
- **FR-004**: The play screen MUST use a cell-first input model: the player taps a cell to select it (with visible selection feedback), then taps a digit 1–9 on an always-visible number pad to place it. Players MUST be able to clear or change their own entries; given digits MUST be immutable.
- **FR-005**: Players MUST be able to enter multiple small candidate digits (pencil marks) in an empty cell, including several in one interaction; each candidate MUST render at a fixed position in a 3×3 arrangement within the cell — 1–3 in the top row, 4–6 in the middle row, 7–9 in the bottom row.
- **FR-006**: System MUST toggle a candidate digit off when it is selected again, and MUST replace a cell's candidates when a full-size digit is committed to that cell.
- **FR-006a**: When a full-size digit is committed (by entry or hint), the system MUST automatically remove that digit from the pencil-mark candidates of every other cell in the same row, column, and 3×3 box.
- **FR-007**: System MUST visually mark any player-entered digit that differs from the puzzle's solution as soon as it is placed, and remove the marking when the digit is corrected or cleared. (Because every puzzle has exactly one solution, this also covers all row/column/box conflicts.)
- **FR-008**: Players MUST be able to request a hint; the system fills exactly one cell with the correct digit from the solution and marks it as hint-provided.
- **FR-009**: System MUST detect completion (all cells filled correctly) automatically and present a completion acknowledgement including solve time when the timer is enabled and the number of hints used.
- **FR-010**: System MUST automatically persist the in-progress game (entries, pencil marks, elapsed time, difficulty) so that play resumes exactly after the game is closed, the device sleeps, or the device is powered off.
- **FR-011**: System MUST record play statistics per difficulty — puzzles completed, best time, average time — viewable on a statistics screen, and MUST record solve time even when the timer display is hidden.
- **FR-012**: System MUST provide a setting to show or hide the in-game timer.
- **FR-013**: System MUST be installable by copying files to the device over a standard USB connection, and launchable on the device without any computer-side tools at play time.
- **FR-014**: System MUST render legibly and remain fully playable on monochrome e-ink screens; color, where available, may enhance but MUST NOT be the sole carrier of any information (e.g., conflict marking must work in grayscale).
- **FR-015**: System MUST scale its layout to different Kobo screen sizes, keeping all touch targets comfortably tappable and all digits (including pencil marks) legible.
- **FR-016**: System MUST manage e-ink screen updates so that normal play does not accumulate distracting ghosting, and MUST redraw correctly after device sleep/wake.
- **FR-017**: System MUST allow the player to exit cleanly back to the device's normal reading interface.
- **FR-018**: System MUST start cleanly at the main menu if saved data is missing or unreadable, without crashing.
- **FR-019**: System MUST ask for confirmation before an action discards a saved in-progress puzzle (e.g., starting a new game).

### Key Entities

- **Puzzle**: A generated 9×9 grid; attributes: the givens (initial digits), the unique solution, and the difficulty level.
- **Game in progress**: The player's current state on a puzzle: committed entries per cell, pencil marks per cell, elapsed solving time, hints used; linked to exactly one Puzzle.
- **Statistics record**: Aggregated results per difficulty level: count of completed puzzles, best solve time, average solve time.
- **Settings**: Player preferences: timer visibility, and any future options; persisted across sessions.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A new puzzle at any difficulty is generated and displayed on the device in under 5 seconds.
- **SC-002**: A first-time user can install the game onto the device and start their first puzzle in under 10 minutes using only the provided instructions and a USB cable.
- **SC-003**: Every touch interaction (select cell, enter digit, toggle pencil mark) produces visible feedback on screen in under 1 second on the target device.
- **SC-004**: 100% of generated puzzles have exactly one solution (verifiable by automated checking of generated puzzles).
- **SC-005**: After closing and relaunching the game — including a full device power cycle — the resumed board matches the pre-close state in 100% of cases (at most the final move may be lost after sudden power loss).
- **SC-006**: The game is fully playable (all functionality, all information distinguishable) on a monochrome e-ink screen, verified by a grayscale playthrough of one complete puzzle.
- **SC-007**: A complete puzzle can be played through on the Kobo Libra Colour with no crashes, no unreadable text, and no touch target that requires more than one attempt to hit for an average adult finger.

## Assumptions

- The primary target device is the Kobo Libra Colour (7" color e-ink touchscreen); other Kobo models are a portability goal, best-effort, verified opportunistically rather than a hard release gate for v1.
- Classic 9×9 sudoku only in v1; variants (6×6, 16×16, killer, etc.) are out of scope.
- One in-progress game at a time; starting a new game (after confirmation) replaces the saved one. Multiple simultaneous saved games are out of scope for v1.
- Undo/redo is out of scope for v1 (the player can clear/change their own entries manually).
- Touch is the primary input; the device's physical page-turn buttons are not required for play.
- The game is fully offline — no network features, accounts, or sync.
- Installation via USB file copy is acceptable to the user (they already connect the device as a USB drive); a one-time device preparation step (e.g., enabling a way to launch third-party content) is acceptable if clearly documented.
- English-only interface for v1; the UI is digit- and icon-heavy, so text is minimal anyway.
- The user's stated technology preferences (C++, the device's native UI/graphics capabilities) are recorded for the planning phase and intentionally not part of this specification.
