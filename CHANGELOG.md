# Changelog

## v1.0.0 — 2026-07-19

First release.

- Full sudoku game for Kobo e-readers: on-device puzzle generation with
  guaranteed unique solutions, technique-graded difficulty (Easy / Medium /
  Hard), pencil marks, hints, error marking, auto-save/resume, timer and
  per-difficulty statistics.
- E-ink aware rendering via FBInk: partial refreshes for moves, configurable
  full-refresh cadence against ghosting.
- **NickelMenu launcher** (a **Sudoku** entry in the **More** menu) — tested
  on a real Kobo Libra Colour, firmware 4.5.
- **KFMon launcher** (disguised library cover) included in the package but
  not yet tested on real hardware.
- Launch wrapper pauses Nickel and the `sickel` watchdog during play and
  resumes them on exit; the game auto-exits after 5 minutes idle so the
  device's normal sleep handling returns.
- Touch calibration via environment variables, with confirmed settings for
  the Kobo Libra Colour.
- Desktop SDL2 simulator sharing the exact device UI code, plus a host unit
  test suite; CI cross-builds and packages the device zip.
