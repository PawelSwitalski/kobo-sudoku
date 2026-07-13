# Kobo Sudoku

A native, fully offline sudoku game for Kobo e-readers. Built for the Kobo
Libra Colour, portable across the Kobo lineup (one ARM binary, geometry
computed from the screen). Puzzles are generated on the device with a
guaranteed unique solution and technique-graded difficulty (Easy / Medium /
Hard).

Features: cell-first touch input, pencil marks (fixed 1-3 / 4-6 / 7-9 in-cell
layout, auto-removed from peers when you place a digit), immediate error
marking against the solution, hints, auto-save after every move with resume,
optional timer and per-difficulty statistics. E-ink friendly: partial
refreshes for moves, periodic full refreshes against ghosting, no per-second
timer redraws.

## Install on your Kobo

**One-time prerequisite — KFMon** (the launcher; survives until a firmware
update, then just redo this step):

1. Download the latest `KoboRoot.tgz` from
   [KFMon releases](https://github.com/NiLuJe/kfmon/releases) (the
   "uninstaller-less" package is fine).
2. Connect the Kobo over USB, copy `KoboRoot.tgz` into the hidden `.kobo`
   folder on the device, eject safely. The Kobo reboots and installs it.

**Install the game:**

1. Connect the Kobo over USB (it shows up as a drive, e.g. `D:`).
2. Extract `kobo-sudoku.zip` onto the drive root (merge the `.adds` folder;
   `kfmon-sudoku.png` lands at the root).
3. Eject safely and let the Kobo finish importing.
4. A book called **kfmon-sudoku** appears in your library. Tap its cover —
   the game starts. Tap `Menu → Exit` to go back to your books.

**Uninstall:** delete `.adds/sudoku/`, `.adds/kfmon/config/sudoku.ini` and
`kfmon-sudoku.png` from the device.

## How to play

- Tap a cell, then tap a digit on the pad (cell-first input).
- **Pencil**: toggles pencil mode — digits become small candidate marks
  (1-3 top row, 4-6 middle, 7-9 bottom of the cell). Tap a mark's digit again
  to remove it. Placing a real digit clears that candidate from the row,
  column and box automatically.
- **Clear**: empties the selected cell (or its marks).
- **Hint**: fixes one wrong cell, or fills one empty cell, and marks it with a
  small square. Hints are counted on the completion screen.
- Wrong digits are shaded immediately (checked against the actual solution).
- Your game is saved after every single action — power off, sleep, or exit
  any time and pick it up with **Continue**.
- Timer shows whole minutes while playing (exact time on completion); hide it
  under **Settings**.

If taps land in the wrong place on your model, see the calibration variables
in `.adds/sudoku/start.sh` (`SUDOKU_TOUCH_SWAP_XY` / `MIRROR_X` / `MIRROR_Y`).

## Build from source

Host tests and desktop simulator (Windows/Linux/macOS, needs CMake + C++17):

```bash
# unit tests
cmake -B build/host -DSUDOKU_BACKEND=none -DBUILD_TESTS=ON
cmake --build build/host --config Release
ctest --test-dir build/host -C Release

# desktop simulator (SDL2; fetched automatically on MSVC)
cmake -B build/sim -DSUDOKU_BACKEND=sdl
cmake --build build/sim --config Release
build/sim/Release/sudoku --width 1264 --height 1680 --dpi 300
```

Device build (in WSL2/Linux, needs
[koxtoolchain](https://github.com/koreader/koxtoolchain)):

```bash
tools/build-fbink.sh                       # fetch + cross-build FBInk (once)
cmake -B build/kobo -DCMAKE_TOOLCHAIN_FILE=cmake/kobo-toolchain.cmake \
      -DSUDOKU_BACKEND=fbink
cmake --build build/kobo
tools/package.sh build/kobo                # -> dist/kobo-sudoku.zip
```

## Dependencies

Per the project constitution (`.specify/memory/constitution.md`), the
dependency set is deliberately tiny, all vendored: FBInk (device rendering),
nlohmann/json (persistence), doctest (tests), SDL2 (host simulator only).
Addition: **stb_truetype** (single header, public domain) rasterizes text for
the shared software canvas used by *both* backends — FBInk already bundles the
same header internally, and one shared text path keeps simulator and device
rendering pixel-identical instead of maintaining two divergent ones.

Fonts: DejaVu Sans (see `dist/.adds/sudoku/assets/FONT-LICENSE.txt`).
