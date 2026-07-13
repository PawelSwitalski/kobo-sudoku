# Tasks: Kobo Sudoku

**Input**: Design documents from `/specs/001-kobo-sudoku/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Tests**: INCLUDED — Constitution v1.0.0 Principle III (non-negotiable) mandates host-run tests for core logic (generator uniqueness, grading, rules, mark cleanup, persistence round-trip/corruption).

**Organization**: Tasks are grouped by user story. Priority order from spec.md: US1 & US6 (P1) → US2 & US4 (P2) → US3 & US5 (P3). US1 is made independently testable before device work via the desktop simulator (foundational phase).

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: US1 play a game · US2 pencil marks · US3 errors & hints · US4 resume · US5 timer & stats · US6 runs on Kobo

## Path Conventions

Single project at repository root per plan.md: `src/`, `tests/`, `third_party/`, `dist/`, `tools/`, `cmake/`.

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Repository and build-system skeleton for the three build flavors (host tests, host simulator, device)

- [x] T001 Initialize git repository with C++ `.gitignore` (build dirs, `*.o`, IDE files) at repo root
- [x] T002 Create source tree skeleton per plan.md: `src/{core,persist,ui/screens,platform/{kobo,sdl}}`, `tests/`, `third_party/`, `dist/{.adds/sudoku,kfmon}`, `tools/`, `cmake/`
- [x] T003 Root `CMakeLists.txt` with flavors `-DSUDOKU_BACKEND=none|sdl|fbink` and `-DBUILD_TESTS=ON` (core always builds; backend and tests conditional)
- [x] T004 [P] Vendor doctest (single header) into `third_party/doctest/` and nlohmann/json into `third_party/nlohmann/`; register both in CMake
- [x] T005 [P] Add `tests/main.cpp` (doctest runner) and CTest registration so `ctest --test-dir build/host` works with a trivial smoke test

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Platform abstraction + desktop simulator + UI base so every user story can be built and seen on the host

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T006 [P] Define `Renderer` interface (`DisplayInfo`, `fillRect`, `drawText`, `drawLine`, `flushPartial`, `flushFull`, `Gray`/`Color`/`TextStyle`/`Rect` types) in `src/platform/renderer.h` per contracts/platform-abstraction.md
- [x] T007 [P] Define `TouchInput` interface (`Tap`, `waitForTap(timeoutMs)`) in `src/platform/input.h` per contracts/platform-abstraction.md
- [x] T008 SDL2 simulator backend: window renderer with e-ink flash simulation on `flushFull` in `src/platform/sdl/sdl_renderer.{h,cpp}`, mouse-click `TouchInput` in `src/platform/sdl/mouse_touch.{h,cpp}`; `--width/--height/--dpi` flags
- [x] T009 [P] Theme & metrics: grayscale-first styles, DPI-relative sizes (cell, pad button; minimum touch target ≥ 9 mm — ~106 px @300 dpi, ~75 px @212 dpi), bundled TTF font loading in `src/ui/theme.{h,cpp}` + font asset in `dist/.adds/sudoku/assets/`
- [x] T010 Base widgets: `Button`, `Label`, modal `Dialog` (confirm/info) with hit-testing in `src/ui/widgets.{h,cpp}` (uses Renderer only — no OS calls, Constitution I)
- [x] T011 App shell: main loop (tap → hit-test → update → flush), screen-stack manager, clean-exit path returning 0, backend selection in `src/main.cpp` + `src/ui/screens/screen.h`

**Checkpoint**: Simulator opens, shows an empty screen with a working button — user story implementation can now begin

---

## Phase 3: User Story 1 — Play a complete game of sudoku (Priority: P1) 🎯 MVP

**Goal**: Start a new puzzle at a chosen difficulty in the simulator, fill digits cell-first, get completion acknowledgement

**Independent Test**: In the simulator: New Game → Easy → solve to completion (dev cheat flag `--reveal` may pre-fill all but a few cells for manual testing) → completion dialog appears

### Tests for User Story 1 (Constitution III — write first, must fail, then implement)

- [x] T012 [P] [US1] Board rules tests: given immutability, value set/clear, peer computation, completion detection in `tests/test_rules.cpp`
- [x] T013 [P] [US1] Generator property tests: 500 seeded puzzles per difficulty → exactly one solution (SC-004), givens ⊂ solution, determinism per seed in `tests/test_generator.cpp`
- [x] T014 [P] [US1] Difficulty grading tests: known Easy/Medium/Hard fixtures grade in-band; technique ladder ordering in `tests/test_difficulty.cpp`

### Implementation for User Story 1

- [x] T015 [P] [US1] Board model: 81 cells (`Given|Entry|Empty`, value), row/col/box peer helpers in `src/core/board.{h,cpp}` per data-model.md
- [x] T016 [P] [US1] Solver: randomized-backtracking grid filler + `countSolutions(grid, limit=2)` early-exit counter in `src/core/solver.{h,cpp}`
- [x] T017 [US1] Logical-technique solver (naked/hidden singles, locked candidates, naked pairs) + `grade()` mapping to Easy/Medium/Hard in `src/core/difficulty.{h,cpp}` (research R6)
- [x] T018 [US1] Generator: dig-holes-with-uniqueness-check pipeline, regenerate until requested band, `Puzzle{givens,solution,difficulty,seed}` in `src/core/generator.{h,cpp}` (must meet SC-001 < 5 s; target < 500 ms)
- [x] T019 [US1] Session: cell-first mutators `commitDigit`/`clearCell` returning `ChangeSet`, given-cell no-ops, `isComplete()`, `tick()` elapsed-time accumulator in `src/core/session.{h,cpp}` per contracts/core-model.md
- [x] T020 [US1] Board view widget: 9×9 grid with thick 3×3 boundaries, bold givens vs normal entries, selection border, per-cell dirty rects for partial refresh in `src/ui/widgets.{h,cpp}` (stretch, may skip: tapping a filled cell highlights same-digit cells — spec edge case "may")
- [x] T021 [US1] Game screen: board + always-visible 1–9 pad + clear button, cell-first flow, busy indicator during generation, completion dialog in `src/ui/screens/game_screen.{h,cpp}`
- [x] T022 [US1] Menu screen: New Game → difficulty picker (Easy/Medium/Hard), Exit; wire into app shell in `src/ui/screens/menu_screen.{h,cpp}`

**Checkpoint**: Full game playable start-to-finish in the desktop simulator

---

## Phase 4: User Story 6 — Runs on the player's Kobo (Priority: P1)

**Goal**: The same game runs on the Kobo Libra Colour: cross-compiled, KFMon-launched, e-ink-correct rendering, clean exit to Nickel

**Independent Test**: quickstart.md scenarios 2, 3, 10 on the device — install zip via USB, launch from library cover, play, exit back to Nickel

### Implementation for User Story 6

- [x] T023 [P] [US6] Vendor FBInk into `third_party/FBInk/` (pinned release) and integrate its build into the `fbink` CMake flavor — fetched at pinned v1.25.5 by `tools/build-fbink.sh` (gitignored, not committed); `fbink` flavor links `third_party/FBInk/Release/libfbink.a`
- [ ] T024 [P] [US6] Kobo cross-toolchain file for koxtoolchain (`arm-kobo-linux-gnueabihf`, static-friendly flags) in `cmake/kobo-toolchain.cmake` + WSL2 build steps verified per quickstart.md — toolchain file written; WSL2 cross-build verification pending (no Ubuntu WSL distro on this machine)
- [x] T025 [US6] FBInk renderer: draw primitives, waveform selection, partial-refresh counting with auto-promotion to full refresh after N partials (ghosting policy, FR-016), rotation handling, and wake-from-sleep repaint (detect resume, force `flushFull` to clear the sleep screen — FR-016) in `src/platform/kobo/fbink_renderer.{h,cpp}`
- [x] T026 [US6] Evdev touch: `/dev/input/event*` discovery, type-B multitouch decode, down/up → `Tap`, coordinate rotation matched to renderer space in `src/platform/kobo/evdev_touch.{h,cpp}`
- [x] T027 [US6] Device lifecycle: SIGTERM/SIGINT clean shutdown, framebuffer state restore on exit, stderr → `crash.log` in `src/main.cpp` + `dist/.adds/sudoku/start.sh` per contracts/install-layout.md
- [x] T028 [P] [US6] Launch integration: `dist/kfmon/config/sudoku.ini`, `kfmon-sudoku.png` cover image, optional `nm-sudoku.txt` NickelMenu snippet per contracts/install-layout.md
- [x] T029 [US6] Packaging script producing copy-to-device zip (device-root layout) in `tools/package.sh`
- [ ] T030 [US6] On-device validation on Libra Colour: quickstart scenarios 2 (generation < 5 s), 3 (touch feedback < 1 s, targets hit first try), 10 (clean exit to Nickel); tune refresh policy and record results in `specs/001-kobo-sudoku/quickstart.md` notes — requires physical device

**Checkpoint**: Both P1 stories done — game fully playable on the physical device (MVP shippable)

---

## Phase 5: User Story 2 — Pencil marks (Priority: P2)

**Goal**: Multiple small candidate digits per cell at fixed 1–3/4–6/7–9 slots, toggling, and auto-cleanup on commit

**Independent Test**: In simulator or device: pencil mode → add 2/5/9 to a cell (verify fixed slots) → toggle 5 off → commit a digit elsewhere in the row and verify that candidate disappears from peers

### Tests for User Story 2 (Constitution III)

- [x] T031 [P] [US2] Pencil-mark tests: bitmask slots, toggle on/off, marks-cleared-on-commit (FR-006), peer auto-cleanup incl. ChangeSet contents (FR-006a), marks only on empty cells in `tests/test_pencilmarks.cpp`

### Implementation for User Story 2

- [x] T032 [US2] Core marks: 9-bit `marks` on Cell, `toggleMark()`, commit-time own-cell clear + peer cleanup with full ChangeSet in `src/core/board.{h,cpp}` and `src/core/session.{h,cpp}`
- [x] T033 [US2] Mark rendering: small digits at fixed 3×3 in-cell slots (1–3 top / 4–6 middle / 7–9 bottom), legible at smallest supported cell size in `src/ui/widgets.{h,cpp}` (board view)
- [x] T034 [US2] Pencil-mode toggle on the game screen: mode button with clear visual state; pad taps route to `toggleMark` in pencil mode in `src/ui/screens/game_screen.{h,cpp}`

**Checkpoint**: Pencil-mark workflow fully usable on top of US1

---

## Phase 6: User Story 4 — Interrupt and resume play (Priority: P2)

**Goal**: Auto-save after every action; Continue restores the exact board (entries, marks, elapsed time) across exit, sleep, and power-off

**Independent Test**: quickstart scenarios 4 (power-cycle resume) and 9 (corrupted save → clean menu, no crash)

### Tests for User Story 4 (Constitution III)

- [x] T035 [P] [US4] Persistence tests: Session ⇄ JSON round-trip (sparse cells, hints flag, marks), schema validation rejects malformed/inconsistent files, atomic-write temp cleanup in `tests/test_persist.cpp`

### Implementation for User Story 4

- [x] T036 [P] [US4] Data directory resolution (device `.adds/sudoku/`, host override via env/flag) in `src/persist/paths.{h,cpp}`
- [x] T037 [US4] Atomic JSON store: write-temp → fsync → rename, load-with-validation → `std::optional` (invalid ⇒ absent, FR-018) in `src/persist/store.{h,cpp}`
- [x] T038 [US4] Session serialization `toJson()`/`fromJson()` exactly per contracts/save-format.md (`save.json` schema v1) in `src/core/session.{h,cpp}`
- [x] T039 [US4] Wiring: save after every mutator on the game screen; Continue entry on menu (only when a valid save exists); new-game-discards-save confirmation dialog (FR-019); save+exit on SIGTERM in `src/ui/screens/{menu_screen,game_screen}.cpp`, `src/main.cpp`
- [x] T040 [US4] Sleep-time exclusion: only accumulate active time into `elapsedSeconds` (wall-clock deltas capped/paused when no events — device sleep must not count, per spec edge case) in `src/core/session.{h,cpp}` + app loop

**Checkpoint**: Close/reopen/power-cycle resumes identically; corrupt save degrades gracefully

---

## Phase 7: User Story 3 — Error highlighting and hints (Priority: P3)

**Goal**: Wrong digits (vs. solution) are marked immediately; a hint fills one correct cell

**Independent Test**: quickstart scenario 6 — place a wrong-but-consistent digit (flagged), correct it (flag clears), request hint (one correct cell filled, marked)

### Tests for User Story 3 (Constitution III)

- [x] T041 [P] [US3] Error & hint tests: `isError` true iff entry ≠ solution, hint targets empty-or-wrong cell only, hint increments counter + sets `fromHint`, hint on complete board is a no-op in `tests/test_rules.cpp` (extend)

### Implementation for User Story 3

- [x] T042 [US3] Core: derived `isError(cell)`, `applyHint()` per contracts/core-model.md in `src/core/session.{h,cpp}`
- [x] T043 [US3] UI: grayscale-safe error rendering (cell shading + digit style — no color-only signal, Constitution II), hint button with "nothing to do" feedback, hint-cell marker, hints count on completion dialog (FR-009) in `src/ui/widgets.{h,cpp}` + `src/ui/screens/game_screen.{h,cpp}`

**Checkpoint**: Assists complete; mistakes surface immediately and hints unstick the player

---

## Phase 8: User Story 5 — Timer and play statistics (Priority: P3)

**Goal**: Optional timer display (minute-granularity), per-difficulty stats recorded on completion, stats & settings screens

**Independent Test**: Solve one puzzle with timer shown and one hidden; stats screen shows both completions with plausible best/average times

### Tests for User Story 5 (Constitution III)

- [x] T044 [P] [US5] Stats/settings tests: completion-only updates, best/average math, hidden-timer completions still recorded (FR-011/FR-012), defaults on missing/corrupt files in `tests/test_persist.cpp` (extend)

### Implementation for User Story 5

- [x] T045 [P] [US5] Stats & settings persistence per contracts/save-format.md (`stats.json`, `settings.json`, schema v1, unknown-key preservation) in `src/persist/store.{h,cpp}` + small models in `src/core/stats.{h,cpp}`
- [x] T046 [US5] In-game timer display: minute-granularity updates via input timeout ticks (Constitution II — no per-second redraws), honoring `showTimer` in `src/ui/screens/game_screen.{h,cpp}`
- [x] T047 [P] [US5] Statistics screen (per-difficulty completed/best/average) in `src/ui/screens/stats_screen.{h,cpp}`; menu entry
- [x] T048 [P] [US5] Settings screen (`showTimer` toggle) in `src/ui/screens/settings_screen.{h,cpp}`; menu entry
- [x] T049 [US5] Completion pipeline: record stats on `Completed`, delete `save.json`, show solve time on completion dialog when enabled in `src/ui/screens/game_screen.cpp`

**Checkpoint**: All six user stories functional

---

## Phase 9: Polish & Cross-Cutting Concerns

**Purpose**: Final validation against Success Criteria and release packaging

- [x] T050 [P] Grayscale playthrough audit (SC-006): simulator forced-grayscale mode + full-puzzle pass verifying every state is distinguishable; fix any color-only signals — scripted `--gray` playthrough on 6" geometry screenshot-verified; error state is a Light-gray cell fill (red is an additive accent on color displays only), so no color-only signal exists by construction
- [x] T051 [P] Multi-size layout check (FR-015): simulator runs at 6" (758×1024 @212dpi) and 8" (1440×1920 @300dpi) geometries; pencil marks legible, touch targets ≥ minimum — 6" and 7" (1264×1680 @300) screenshot-verified; all sizes derive from mm()/DPI so 8" @300dpi only gains whitespace (not separately screenshotted)
- [ ] T052 Device soak pass on Libra Colour: full quickstart validation table (scenarios 1–11, incl. sleep/wake redraw), ghosting-policy N tuning, generation timing per difficulty; record results in `specs/001-kobo-sudoku/quickstart.md` — requires physical device
- [x] T053 [P] User-facing install/play guide (KFMon one-time setup, zip install, uninstall) in `README.md` (target: SC-002 10-minute first puzzle)
- [ ] T054 Fresh-user install test per SC-002 (user follows README only, stopwatch to first puzzle) and fix friction found — requires a human tester
- [x] T055 Code cleanup: dead code, TODOs, consistent naming; confirm dependency set still matches Constitution's approved list (stb_truetype addition documented in README per Constitution VI)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)** → **Foundational (Phase 2)** → all user stories
- **US1 (Phase 3)**: only needs Foundational (playable in simulator)
- **US6 (Phase 4)**: needs US1's game screen to have something to validate on device (T030); T023–T029 can start right after Foundational, in parallel with US1
- **US2 (Phase 5)**: extends US1's board/session/game screen
- **US4 (Phase 6)**: serializes US1's Session; T035–T038 don't depend on US2; T038 must follow US2's T032 to persist marks (or be extended after)
- **US3 (Phase 7)**: extends US1's Session + game screen; independent of US2/US4/US5
- **US5 (Phase 8)**: needs US1 completion flow; T045 shares `src/persist/store` with US4 (do US4 first as planned)
- **Polish (Phase 9)**: after all desired stories

### Within Each User Story

- Test tasks first (write → watch fail → implement until green; Constitution III)
- Core before UI; UI widgets before screens; wiring last

### Parallel Opportunities

- Phase 2: T006, T007, T009 in parallel; T008/T010/T011 after T006–T007
- Phase 3 ∥ Phase 4: one workstream on core gameplay (T012–T022), another on device backend (T023–T029) — they meet at T030
- All `[P]` test tasks within a phase can run together
- Phase 8: T045, T047, T048 in parallel after T044

## Parallel Example: Kick-off after Foundational

```bash
# Workstream A (gameplay):  T012, T013, T014 in parallel, then T015–T022
# Workstream B (device):    T023, T024, T028 in parallel, then T025–T027, T029
# Converge:                 T030 (on-device validation of the US1 game)
```

## Implementation Strategy

### MVP First

1. Phases 1–2 (setup + foundational: simulator running)
2. Phase 3 (US1): complete game in the simulator — **validate before touching device code**
3. Phase 4 (US6): same game on the Kobo — **MVP: shippable to the user's device**
4. Stop and play a few real games; feed findings into refresh-policy tuning

### Incremental Delivery

Each subsequent phase (US2 pencil marks → US4 resume → US3 assists → US5 timer/stats) is independently testable and leaves the game shippable at every checkpoint. Commit after each task or logical group.

---

## Notes

- Tests are mandatory only where Constitution III applies (core logic); UI/screen tasks are validated via simulator + quickstart scenarios
- `[P]` = different files, no incomplete-task dependencies
- Keep `src/core` free of any OS/IO includes (Constitution I) — persistence goes through `src/persist`, rendering through `Renderer`
