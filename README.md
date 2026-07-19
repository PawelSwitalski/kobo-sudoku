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

**One-time prerequisite — NickelMenu** (lets Nickel show a custom entry in
**More**; survives until a firmware update, then just redo this step — the
same one-time-per-firmware-update pattern as the legacy KFMon setup below,
if that's familiar from an earlier install):

1. Download the latest `KoboRoot.tgz` from the
   [NickelMenu project](https://pgaskin.github.io/NickelMenu/) (firmware
   4.x only — confirmed working on a Kobo Libra Colour, firmware 4.5;
   other firmware versions are best-effort, see the legacy alternative
   below).
2. Connect the Kobo over USB, copy `KoboRoot.tgz` into the hidden `.kobo`
   folder on the device, eject safely. The Kobo reboots and installs it.

**Install the game:**

1. Connect the Kobo over USB (it shows up as a drive, e.g. `D:`).
2. Extract `kobo-sudoku.zip` onto the drive root (merge the `.adds` folder).
3. Eject safely and let the Kobo finish importing.
4. Open **More** (bottom navigation) — a **Sudoku** entry appears alongside
   Settings and your other items. Tap it to start the game. Tap
   `Menu → Exit` to go back to your books.

**Uninstall:** delete `.adds/sudoku/`, `.adds/nm/kobo-sudoku`,
`.adds/kfmon/config/sudoku.ini` and `kfmon-sudoku.png` from the device.

### Legacy alternative: KFMon cover-tap

Kobo Sudoku can also be launched the way earlier versions worked, via
[KFMon](https://github.com/NiLuJe/kfmon) and a disguised library cover. No
extra install step is needed beyond the steps above — the `.adds/kfmon/`
config and `kfmon-sudoku.png` cover ship in the same zip — only the KFMon
prerequisite below is new if you don't already have it set up.

1. **One-time prerequisite — KFMon**: download the latest `KoboRoot.tgz`
   from [KFMon releases](https://github.com/NiLuJe/kfmon/releases) (the
   "uninstaller-less" package is fine), copy it into `.kobo` on the device,
   eject. The Kobo reboots and installs it.
2. After installing the game (steps above), a book called **kfmon-sudoku**
   appears in your library. Tap its cover — the game starts.

Both methods are independent and can be installed at the same time: either
one launches the same game and resumes the same saved progress.

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
- Your game is saved after every single action — exit any time and pick it
  up with **Continue**.
- Timer shows whole minutes while playing (exact time on completion); hide it
  under **Settings**.

**Sleeping the device:** while the game is running, Nickel (and the power
button / inactivity sleep it handles) is paused so it can't draw over the
game or steal input — tap **Menu → Exit** first, then sleep normally. This
also means the device won't auto-sleep from inactivity while a game is open;
exit if you're setting it down for a while.

### Touch calibration

Kobo touch panels don't all report coordinates in the same orientation as
the screen. If taps land in the wrong place (e.g. the right edge behaves
like the bottom), set the matching variables in `.adds/sudoku/start.sh`
before `./sudoku` runs, uncommenting the ones you need:

```sh
export SUDOKU_TOUCH_SWAP_XY=1    # swap raw x/y first
export SUDOKU_TOUCH_MIRROR_X=1   # mirror x after swap
export SUDOKU_TOUCH_MIRROR_Y=1   # mirror y after swap
```

Set `SUDOKU_TOUCH_DEBUG=1` too and check `.adds/sudoku/crash.log` after a
few taps (`tap raw=(..) -> (..)`) to work out which combination you need —
raw coordinates that span the screen's *height* on what should be the *x*
axis is the usual sign that `SWAP_XY` is needed.

Confirmed working settings by model (please share yours if you test a new
one):

| Model | Settings |
|---|---|
| Kobo Libra Colour (FW 4.5) | `SUDOKU_TOUCH_SWAP_XY=1`, `SUDOKU_TOUCH_MIRROR_Y=1` |

**Known limitation:** the calibration above assumes the device is held in
its normal (non-inverted) portrait orientation — the game doesn't read the
accelerometer, so holding it upside-down currently maps taps incorrectly.

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
