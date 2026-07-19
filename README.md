# Kobo Sudoku

A native, fully offline sudoku game for Kobo e-readers.

Built for the Kobo Libra Colour and portable across the Kobo lineup — one
ARM binary, with screen geometry computed at runtime. Puzzles are generated
on the device with a guaranteed unique solution and technique-graded
difficulty (Easy / Medium / Hard). No network, no accounts, no ads.

> **Status:** launching from the **NickelMenu** entry is tested on a real
> device (Kobo Libra Colour, firmware 4.5). The bundled **KFMon** launcher
> is not yet tested on hardware.

## Features

- Cell-first touch input sized for e-ink screens
- Pencil marks with a fixed in-cell layout (1–3 / 4–6 / 7–9), auto-removed
  from peers when you place a digit
- Immediate error marking (checked against the actual solution) and hints
- Auto-save after every move; resume any time with **Continue**
- Optional timer and per-difficulty statistics
- E-ink friendly: partial refreshes for moves, configurable full refreshes
  against ghosting, no per-second redraws

## Quick start

1. Install [NickelMenu](https://pgaskin.github.io/NickelMenu/) (one-time,
   per firmware update).
2. Download `kobo-sudoku.zip` from the
   [latest release](https://github.com/PawelSwitalski/kobo-sudoku/releases/latest)
   and extract it onto the Kobo's USB drive root.
3. Eject, open **More** on the device, tap **Sudoku**.

Full steps, the KFMon alternative and uninstall: [docs/installation.md](docs/installation.md).

## Documentation

| Document | Contents |
|---|---|
| [Installation](docs/installation.md) | Install, launchers (NickelMenu / KFMon), uninstall |
| [Playing the game](docs/gameplay.md) | Controls, pencil marks, hints, how puzzles are generated and graded |
| [Settings](docs/settings.md) | In-game settings, launcher options, touch calibration, device files |
| [Building](docs/building.md) | Host tests, desktop simulator, Kobo cross-build, CI and releases |

## Building in short

```bash
# unit tests + desktop simulator (Windows/Linux/macOS, CMake + C++17)
cmake -B build/sim -DSUDOKU_BACKEND=sdl && cmake --build build/sim --config Release

# device binary (Linux/WSL2 + koxtoolchain)
tools/build-fbink.sh
cmake -B build/kobo -DCMAKE_TOOLCHAIN_FILE=cmake/kobo-toolchain.cmake -DSUDOKU_BACKEND=fbink
cmake --build build/kobo && tools/package.sh build/kobo
```

Details in [docs/building.md](docs/building.md).

## License

[MIT](LICENSE). Vendored third-party components keep their own licenses:
FBInk, nlohmann/json, doctest, stb_truetype, SDL2 (simulator only) and the
DejaVu Sans fonts (`dist/.adds/sudoku/assets/FONT-LICENSE.txt`).
