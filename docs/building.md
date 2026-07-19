# Building from source

The project is plain CMake + C++17 with all dependencies vendored (see
[Dependencies](#dependencies)). There are three build flavors selected with
`-DSUDOKU_BACKEND=`: `none` (core + tests only), `sdl` (desktop simulator)
and `fbink` (Kobo device binary).

## Host unit tests

Works on Windows/Linux/macOS with any C++17 compiler:

```bash
cmake -B build/host -DSUDOKU_BACKEND=none -DBUILD_TESTS=ON
cmake --build build/host --config Release
ctest --test-dir build/host -C Release
```

## Desktop simulator

Runs the full game in an SDL2 window — the same UI code as the device, so
it's the fastest way to iterate. SDL2 is fetched automatically on MSVC; on
Linux/macOS install it from your package manager.

```bash
cmake -B build/sim -DSUDOKU_BACKEND=sdl
cmake --build build/sim --config Release
build/sim/Release/sudoku --width 1264 --height 1680 --dpi 300
```

The `--width/--height/--dpi` above simulate a Kobo Libra Colour screen.

## Kobo device build (cross-compile)

Needs Linux or WSL2 and the
[koxtoolchain](https://github.com/koreader/koxtoolchain) prebuilt
`arm-kobo-linux-gnueabihf` toolchain unpacked into `~/x-tools/`:

```bash
tools/build-fbink.sh                       # fetch + cross-build FBInk (once)
cmake -B build/kobo -DCMAKE_TOOLCHAIN_FILE=cmake/kobo-toolchain.cmake \
      -DSUDOKU_BACKEND=fbink
cmake --build build/kobo
tools/package.sh build/kobo                # -> dist/kobo-sudoku.zip
```

`dist/kobo-sudoku.zip` is the install package described in
[installation.md](installation.md).

## Continuous integration and releases

`.github/workflows/build.yml` runs on every push and pull request: host
tests first, then the Kobo cross-build, uploading `kobo-sudoku.zip` as a
workflow artifact.

Pushing a tag matching `v*` additionally publishes a GitHub release with
the zip attached:

```bash
git tag v1.0.0
git push origin v1.0.0
```

## Dependencies

The dependency set is deliberately tiny and fully vendored:

- **FBInk** — e-ink framebuffer rendering on the device (built by
  `tools/build-fbink.sh`)
- **nlohmann/json** — save/settings persistence
- **doctest** — unit tests
- **stb_truetype** — text rasterization for the shared software canvas used
  by both backends (keeps simulator and device rendering pixel-identical)
- **SDL2** — host simulator only
- **DejaVu Sans** fonts (`dist/.adds/sudoku/assets/FONT-LICENSE.txt`)

## Source layout

```
src/core/       Board, solver, generator, difficulty grader, session, stats
src/persist/    JSON save/settings/stats storage and device paths
src/platform/   Renderer + touch abstractions; kobo/ (FBInk, evdev) and sdl/ backends
src/ui/         Screens (menu, game, settings, stats), widgets, theme
tests/          doctest unit tests (host)
dist/           Device package contents (launcher script, NickelMenu/KFMon config, fonts)
tools/          FBInk build + device packaging scripts
specs/          Design documents the project was built from
```
