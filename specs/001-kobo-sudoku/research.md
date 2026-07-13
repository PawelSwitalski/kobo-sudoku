# Research: Kobo Sudoku

**Date**: 2026-07-12 | **Feature**: [spec.md](./spec.md)

All NEEDS CLARIFICATION items from the Technical Context were resolved. Findings below.

## R1. Rendering: FBInk instead of Kobo's own (Nickel/Qt) UI libraries

**Decision**: Render directly to the e-ink framebuffer using [FBInk](https://github.com/NiLuJe/FBInk) (C library, links cleanly from C++), running as a standalone full-screen app while Nickel is paused.

**Rationale**:
- The user's initial idea was to reuse "the graphic libraries Kobo offers for its own UI". Those are Nickel's private Qt builds (`libnickel`). They are undocumented, change between firmware releases, and broke wholesale in the 4.x → 5.x firmware transition (this is exactly why NickelMenu does not work on firmware 5.x). Linking against them would tie the game to specific firmware versions — the opposite of the "works on other Kobos" goal.
- FBInk is the de-facto standard for Kobo homebrew rendering: it abstracts the per-SoC framebuffer differences (NXP i.MX mxcfb, Allwinner/sunxi, and the 2024 colour devices), handles e-ink refresh ioctls, waveform modes, partial vs full refresh, rotation, and Kaleido colour panels. **Libra Colour support is confirmed** (tested by the FBInk project; FBInk also backs rmkit and KFMon on these devices).
- This is the same architecture KOReader, Plato, and rmkit apps use — one armhf binary runs across the whole Kobo lineup.

**Alternatives considered**:
- *Nickel/Qt (libnickel)*: rejected — firmware-fragile, undocumented, breaks portability goal.
- *SDL2 on device*: rejected — not present on the device, would need porting to the e-ink framebuffer anyway.
- *Raw mxcfb ioctls*: rejected — reimplements FBInk poorly, loses sunxi/Kaleido handling.
- *KOReader plugin (Lua)*: viable but rejected — requires users to install KOReader; user asked for a native C++ game.

## R2. Touch input: direct evdev reading

**Decision**: Read Linux multitouch events directly from `/dev/input/event*` (evdev protocol, type-B multitouch), with per-device coordinate rotation handled by a small mapping layer (FBInk exposes the canonical rotation to map against).

**Rationale**: There is no input library on the device; every Kobo homebrew app (KOReader, Plato, rmkit) reads evdev directly. Single-touch taps are all this game needs, which keeps the protocol handling small.

**Alternatives considered**: *libevdev* (extra dependency for little gain at this scope — plain `read()` of `input_event` structs suffices); *tslib* (abandoned upstream for these devices).

## R3. Launcher: KFMon (primary), NickelMenu config included as optional extra

**Decision**: Ship the game as a KFMon-launched app: the package contains a "book cover" PNG that appears in the Kobo library; tapping it makes [KFMon](https://github.com/NiLuJe/kfmon) launch the game. Also include an optional NickelMenu config snippet for users on firmware 4.x.

**Rationale**:
- **NickelMenu is not compatible with Kobo firmware 5.x**, which the Libra Colour ships with/updates to. KFMon is inotify-based and independent of Nickel internals, so it works across firmware 4.x and 5.x and across all models — this is how KOReader is launched on current devices.
- KFMon is a one-time install for the user (documented in quickstart); the game package itself is then pure file-copy over USB, satisfying FR-013 and SC-002.

**Alternatives considered**: *NickelMenu only* (rejected — dead on FW 5.x); *replacing Nickel at boot* (rejected — invasive, risky for a casual install); *NickelDBus* (same FW-fragility class as NickelMenu).

## R4. Toolchain & build: koxtoolchain cross-compile, CMake, host builds for tests

**Decision**: Cross-compile with [koxtoolchain](https://github.com/koreader/koxtoolchain) (`arm-kobo-linux-gnueabihf`, armhf, the toolchain KOReader uses for every Kobo model including the Libra Colour), driven by CMake with a Kobo toolchain file. Since the development machine is Windows, the device build runs under **WSL2 (or Docker)**; the game core and desktop simulator also build natively on the host for day-to-day development.

**Rationale**: One armhf binary covers the entire Kobo lineup (KOReader ships exactly one Kobo build). koxtoolchain pins a glibc old enough for all supported firmware. CMake gives us the three build flavors (device, host tests, host simulator) from one tree.

**Alternatives considered**: *MinGW/MSVC for device* (impossible — target is ARM Linux); *generic Linaro armhf toolchain* (glibc too new for older firmware); *Rust/Plato-style stack* (user chose C++).

## R5. Architecture: portable core + thin platform layer, desktop simulator

**Decision**: Split the code into a **portable core** (board model, generator, solver, difficulty grading, game session, persistence, screen/widget layout — no OS calls) behind two small interfaces, `Renderer` and `TouchInput`, with two backends:
- `platform/kobo`: FBInk renderer + evdev input (the shipping product),
- `platform/sdl`: SDL2 window renderer + mouse input (desktop simulator for development and UI iteration without the device).

**Rationale**: The core (~90% of the code) becomes unit-testable on the host PC (SC-004, SC-005 verifiable in CI-style runs), and UI work doesn't require flashing the device for every change. This also directly serves the multi-model portability requirement: layout code is written against abstract width×height×DPI, not one device.

**Alternatives considered**: *device-only development* (rejected — slow iteration, untestable core); *full game engine* (rejected — enormous overkill for a static board game on e-ink).

## R6. Puzzle generation & difficulty grading

**Decision**:
1. Generate a complete solution grid via randomized backtracking.
2. Remove givens one at a time (randomized order); after each removal, run a solution counter with early exit at 2 — keep the removal only if exactly one solution remains (guarantees FR-001/SC-004 by construction).
3. Grade difficulty with a **logical-technique solver**: Easy = solvable with naked/hidden singles only; Medium = additionally needs locked candidates / naked pairs; Hard = needs more advanced techniques beyond that set. Regenerate (or keep digging) until the grid matches the requested difficulty band.

**Rationale**: This is the standard, well-understood pipeline; on a ~1–2 GHz ARM core a graded puzzle takes tens to hundreds of milliseconds — comfortably inside the 5 s budget (SC-001) with a busy indicator as fallback. Technique-based grading makes difficulty *observable* (FR-002) rather than a raw clue count.

**Alternatives considered**: *bundled pre-generated puzzles* (rejected by user decision — on-device generation chosen); *clue-count-only grading* (rejected — poor difficulty correlation).

## R7. Persistence: JSON files under `.adds/sudoku/`

**Decision**: Store `save.json` (in-progress game), `stats.json`, and `settings.json` as JSON files in the app's own directory `.adds/sudoku/` on the device's internal storage, written atomically (write temp file → rename). JSON handled by the header-only nlohmann/json library.

**Rationale**: `.adds/` is the community-conventional home for homebrew on Kobo (hidden from the library scanner). Atomic rename satisfies the power-loss requirement (SC-005: at most the last move lost — the game saves after every move, which is cheap at this data size). JSON keeps saves debuggable and forward-extensible; nlohmann/json is header-only, no cross-compile friction.

**Alternatives considered**: *SQLite* (rejected — heavyweight for three tiny documents); *custom binary format* (rejected — harder to debug/extend, no size pressure justifying it).

## R8. E-ink refresh strategy

**Decision**: Partial refreshes for cell/pad updates during play; full-screen flashing refresh on screen transitions and automatically after N (~10–15, tunable) partial refreshes to clear ghosting (FR-016). Timer display, when enabled, updates at most once per minute to avoid constant flashing. Grayscale-first design: selection = thick border, given digits = bold, errors = cell shading + digit style, so a monochrome panel loses nothing (FR-014); on Kaleido panels colour is added as accent only.

**Rationale**: Matches e-ink UX norms (KOReader behaves this way); FBInk exposes the needed waveform/refresh control per platform.

## R9. Testing approach

**Decision**: doctest (header-only) unit tests for the core, built and run on the host: generator uniqueness (SC-004, property-tested over many seeds), difficulty grading, rules/conflict logic, pencil-mark auto-cleanup, save/load round-trip including corrupted-file recovery (FR-018). Manual validation scenarios on device per quickstart.md.

**Alternatives considered**: *Catch2/GoogleTest* (fine too; doctest chosen for smallest footprint and fastest compile).

## Sources

- [NickelMenu — firmware compatibility](https://pgaskin.net/NickelMenu/) and [NickelMenu GitHub](https://github.com/pgaskin/NickelMenu) — no firmware 5.x support
- [KFMon GitHub](https://github.com/NiLuJe/kfmon) — inotify-based launcher, device-range support
- [FBInk GitHub](https://github.com/NiLuJe/FBInk) and [FBInk README](https://github.com/NiLuJe/FBInk/blob/master/README.md) — full Kobo lineup incl. Kaleido colour devices
- [KOReader PR #11737 — Clara B/W + Colour, Libra Colour support](https://github.com/koreader/koreader/pull/11737) — confirms armhf homebrew stack on Libra Colour
- [Kobo 5.x firmware findings](https://github.com/notmarek/KoboTolinoFindings) — 5.x architecture changes
- [rmkit + FBInk on Kobo](https://rmkit.dev/rmkit-comes-to-kobo-with-fbink/) — FBInk as homebrew rendering backend
