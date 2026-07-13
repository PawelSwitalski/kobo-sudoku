# Quickstart: Build, Install & Validate

**Feature**: [spec.md](./spec.md) | Contracts: [install-layout](./contracts/install-layout.md), [save-format](./contracts/save-format.md)

## Prerequisites

- **Windows 11 dev machine** with **WSL2** (Ubuntu) — used only for the device (cross) build.
- **koxtoolchain** installed in WSL2 (`arm-kobo-linux-gnueabihf`): follow https://github.com/koreader/koxtoolchain (build once, ~30 min).
- **CMake ≥ 3.20** + a host C++17 compiler (host tests/simulator build natively on Windows or in WSL2; SDL2 needed for the simulator only).
- **Kobo device** with [KFMon](https://github.com/NiLuJe/kfmon) installed (one-time: copy its `KoboRoot.tgz` to `.kobo/` and eject; survives until a firmware update, then re-copy).

## Build

```bash
# Host: unit tests (fast inner loop)
cmake -B build/host -DSUDOKU_BACKEND=none -DBUILD_TESTS=ON
cmake --build build/host && ctest --test-dir build/host --output-on-failure

# Host: desktop simulator (UI iteration without the device)
cmake -B build/sim -DSUDOKU_BACKEND=sdl
cmake --build build/sim && ./build/sim/sudoku --width 1264 --height 1680 --dpi 300

# Device (in WSL2): armhf binary + install zip
cmake -B build/kobo -DCMAKE_TOOLCHAIN_FILE=cmake/kobo-toolchain.cmake -DSUDOKU_BACKEND=fbink
cmake --build build/kobo && tools/package.sh build/kobo   # → dist/kobo-sudoku.zip
```

## Install on device

1. Connect the Kobo over USB (mounts as `D:` on this machine).
2. Extract `kobo-sudoku.zip` onto `D:\` (merges `.adds/`, adds `kfmon-sudoku.png`).
3. Eject safely. The Kobo imports "Kobo Sudoku" as a book in the library.
4. Tap its cover → KFMon launches the game.

## Validation scenarios (map to Success Criteria)

| # | Scenario | Expected | SC |
|---|----------|----------|----|
| 1 | Host: `ctest` generator suite (500 seeded puzzles/difficulty) | All unique-solution, all grade in-band | SC-004 |
| 2 | Device: New Game at each difficulty, stopwatch generation | Board visible < 5 s | SC-001 |
| 3 | Device: tap cells/digits/pencil toggles across the board | Visible feedback < 1 s each; every target hit first try | SC-003, SC-007 |
| 4 | Device: solve half a puzzle → exit → power off → relaunch → Continue | Identical board incl. marks & timer | SC-005 |
| 5 | Device: full playthrough of one Easy puzzle | Completion screen, stats updated, no crash | SC-007 |
| 6 | Device: play a session with deliberate errors, hint, mark auto-cleanup | Solution-based error marks appear/clear; hint fills one correct cell; peer marks auto-removed | FR-006a/7/8 |
| 7 | Grayscale check (or monochrome device): scenario 6 again | All states distinguishable without colour | SC-006 |
| 8 | Fresh user test: this doc + zip + USB cable, no help | Playing in < 10 min | SC-002 |
| 9 | Device: corrupt `save.json` by hand → launch | Clean menu, no Continue, no crash | FR-018 |
| 10 | Device: exit game | Back in Nickel library, device usable | FR-017 |
| 11 | Device: let device sleep mid-game → wake | Game screen redraws fully (no sleep-screen remnants); sleep time not counted as solving time | FR-016 |

Corrupted-file, round-trip, and rules edge cases are additionally covered host-side in `tests/` (run in scenario 1's `ctest` invocation).
