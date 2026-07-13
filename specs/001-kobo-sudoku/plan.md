# Implementation Plan: Kobo Sudoku

**Branch**: `001-kobo-sudoku` | **Date**: 2026-07-12 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `/specs/001-kobo-sudoku/spec.md`

## Summary

A native, fully offline sudoku game for Kobo e-readers (primary target: Kobo Libra Colour; portable across the lineup). Puzzles are generated on device with a guaranteed unique solution and technique-graded difficulty. Play uses a cell-first touch model with pencil marks (fixed 1–3/4–6/7–9 in-cell layout), solution-based error highlighting, hints, auto-save/resume, and optional timer + statistics.

Technical approach (from [research.md](./research.md)): a portable C++17 core (game logic, UI layout, persistence) behind small `Renderer`/`TouchInput` interfaces, with an FBInk + evdev backend for the device and an SDL2 backend as a desktop simulator. Cross-compiled with koxtoolchain (one armhf binary for all Kobo models), launched via KFMon (works on firmware 4.x and 5.x), installed by USB file copy into `.adds/sudoku/`.

## Technical Context

**Language/Version**: C++17

**Primary Dependencies**: FBInk (e-ink framebuffer rendering, vendored/static), nlohmann/json (header-only, persistence), doctest (header-only, tests), SDL2 (host-only, desktop simulator). Device input via raw evdev — no library.

**Storage**: JSON files (`save.json`, `stats.json`, `settings.json`) in `.adds/sudoku/` on device internal storage; atomic write-then-rename

**Testing**: doctest unit tests for the core, built/run on host (Windows native or WSL2); manual on-device validation scenarios in quickstart.md

**Target Platform**: Kobo e-readers running stock firmware (Linux/armhf, e-ink framebuffer), firmware 4.x and 5.x; primary device Kobo Libra Colour (7" Kaleido 3, 1264×1680). Dev host: Windows 11 + WSL2/Docker for cross-compilation

**Project Type**: Embedded/desktop-style native application (single binary, full-screen, touch)

**Performance Goals**: Puzzle generation+display < 5 s worst case (target < 500 ms typical); touch-to-visible-feedback < 1 s (target < 250 ms via partial e-ink refresh); save-after-move imperceptible (< 50 ms)

**Constraints**: Fully offline; monochrome-first rendering (colour as accent only); e-ink refresh discipline (partial refreshes + periodic full refresh to clear ghosting); modest CPU/RAM (single-core-class budget, < 50 MB RSS); installable by plain USB file copy; must exit cleanly back to Nickel

**Scale/Scope**: Single user, 4 screens (menu, game, statistics, settings) + completion dialog; 9×9 classic sudoku only; ~5–8 KLOC estimated

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Evaluated against Constitution v1.0.0 (ratified 2026-07-12):

| Principle | Status | Evidence in this plan |
|-----------|--------|----------------------|
| I. Portable core, thin platform layer | ✅ Pass | `src/core`/`src/ui` are OS-free; device code confined to `src/platform/kobo` behind `Renderer`/`TouchInput` ([contracts/platform-abstraction.md](./contracts/platform-abstraction.md)) |
| II. E-ink-first, grayscale-first UX | ✅ Pass | Refresh policy in FbinkRenderer (research R8); minute-ceiling timer; DPI-relative metrics in `ui/theme`; grayscale-first styles |
| III. Host-testable correctness | ✅ Pass | doctest suite covers generator uniqueness, grading, rules, mark cleanup, persistence round-trip + corruption (plan `tests/`, quickstart scenario 1) |
| IV. Firmware-agnostic device integration | ✅ Pass | No libnickel; FBInk + evdev + KFMon only (research R1–R3); single armhf binary; USB file-copy install |
| V. Never lose the player's progress | ✅ Pass | Save-after-every-move, atomic temp+rename, corrupt-file → defaults ([contracts/save-format.md](./contracts/save-format.md), data-model save policy) |
| VI. Simplicity, minimal dependencies | ✅ Pass | Dependency set is exactly the constitution's approved list (FBInk, nlohmann/json, doctest, host-only SDL2); all vendored |

**Post-Phase-1 re-check**: design introduces no multi-project sprawl, no speculative abstractions beyond the two platform interfaces required by Principle I and the portability requirement (FR-014/FR-015). Pass.

## Project Structure

### Documentation (this feature)

```text
specs/001-kobo-sudoku/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output
│   ├── core-model.md            # Core API contract (board/generator/session)
│   ├── platform-abstraction.md  # Renderer + TouchInput interfaces
│   ├── save-format.md           # JSON schemas for save/stats/settings
│   └── install-layout.md        # On-device package layout + KFMon config
└── tasks.md             # Phase 2 output (/speckit-tasks — NOT created by /speckit-plan)
```

### Source Code (repository root)

```text
CMakeLists.txt               # 3 flavors: kobo (cross), host-tests, host-sim
cmake/
└── kobo-toolchain.cmake     # koxtoolchain arm-kobo-linux-gnueabihf

src/
├── core/                    # Portable, no OS calls, fully unit-tested
│   ├── board.{h,cpp}        # Grid, cells, givens, pencil marks, rules
│   ├── generator.{h,cpp}    # Solution grid + hole digging + uniqueness
│   ├── solver.{h,cpp}       # Backtracking counter + logical-technique solver
│   ├── difficulty.{h,cpp}   # Technique-based grading (Easy/Medium/Hard)
│   └── session.{h,cpp}      # Game state machine, moves, hints, timer model
├── persist/
│   ├── store.{h,cpp}        # Atomic JSON load/save, corruption recovery
│   └── paths.{h,cpp}        # App data directory resolution
├── ui/                      # Renderer-agnostic screens & widgets
│   ├── theme.{h,cpp}        # Grayscale-first styles, DPI-relative metrics
│   ├── widgets.{h,cpp}      # Button, number pad, board view, dialogs
│   └── screens/             # menu, game, stats, settings, completion
├── platform/
│   ├── renderer.h           # Renderer interface (contract)
│   ├── input.h              # TouchInput interface (contract)
│   ├── kobo/                # FBInk renderer, evdev touch, refresh policy
│   └── sdl/                 # Desktop simulator backend
└── main.cpp                 # App loop, backend selection, clean exit

tests/                       # doctest, host-built
├── test_generator.cpp       # uniqueness property tests (SC-004)
├── test_rules.cpp           # conflicts, error marking, completion
├── test_pencilmarks.cpp     # layout slots, toggling, auto-cleanup (FR-006a)
├── test_difficulty.cpp
└── test_persist.cpp         # round-trip + corrupted-file recovery (FR-018)

third_party/                 # vendored: FBInk, nlohmann/json, doctest
dist/                        # device package template
├── .adds/sudoku/            # binary + assets land here
└── kfmon/                   # sudoku.png cover + sudoku.ini KFMon config
tools/
└── package.sh               # builds the copy-to-device zip
```

**Structure Decision**: Single project, single binary. The only architectural seam is `platform/` (Renderer/TouchInput interfaces) — required by the multi-device portability requirement and by host-side testing/simulation. Everything else is a plain layered layout: `core` → `ui` → `main`, with `persist` used by `core/session`.

## Complexity Tracking

No constitution violations to justify (no constitution ratified). No exceptional complexity introduced: two platform backends are the minimum for "runs on device" + "developable/testable on PC".
