# Playing Kobo Sudoku

## Controls

Input is **cell-first**: tap a cell on the board, then tap a digit on the
pad below.

- **Pencil** — toggles pencil mode. In pencil mode, digits become small
  candidate marks inside the cell, in a fixed layout: 1–3 in the top row,
  4–6 in the middle, 7–9 at the bottom. Tap the same digit again to remove
  the mark. When you place a real digit, that candidate is automatically
  cleared from every peer cell in the same row, column and box.
- **Clear** — empties the selected cell (or removes its pencil marks).
- **Hint** — fixes one wrong cell, or fills one empty cell, and marks it
  with a small square. Hints used are shown on the completion screen.
- **Menu** — pause menu: continue, new game, settings, statistics, exit.

Wrong digits are shaded immediately — entries are checked against the
puzzle's actual solution, not just against visible conflicts.

## Saving and resuming

The game auto-saves after **every single action**. You can exit (or let the
idle timer exit for you) at any moment and pick the game up later with
**Continue** on the main menu. Progress is stored on the device in
`.adds/sudoku/`.

## Timer and statistics

While playing, the timer shows whole minutes (e-ink friendly — no
per-second redraws); your exact time is shown on completion. The timer can
be hidden in **Settings**. Per-difficulty statistics (games completed, best
and average times) are tracked on the **Stats** screen.

## About the puzzles

Puzzles are **generated on the device** — there is no fixed puzzle list and
no network connection, ever. Every generated puzzle has a **guaranteed
unique solution**.

Difficulty is graded by the solving techniques a puzzle actually requires:

- **Easy** — solvable with singles only (naked and hidden singles).
- **Medium** — additionally requires locked candidates (pointing/claiming)
  or naked pairs.
- **Hard** — requires more than those techniques.

## E-ink behavior

The game is designed around e-ink refresh characteristics: moves use fast
partial refreshes, and a full flashing refresh runs periodically to clear
ghosting (configurable — see [Settings](settings.md#in-game-settings)).
