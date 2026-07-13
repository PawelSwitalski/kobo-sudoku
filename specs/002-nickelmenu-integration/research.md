# Research: NickelMenu "More" Menu Launcher

**Date**: 2026-07-13 | **Feature**: [spec.md](./spec.md)

No NEEDS CLARIFICATION items remain in the Technical Context (this feature adds no new language/runtime/storage surface). Findings below cover the design decisions needed to satisfy the spec's functional requirements.

## R1. NickelMenu as the sole "More"-menu mechanism (reused, not reinvented)

**Decision**: Use NickelMenu's plain-text config directive syntax (`menu_item : main : <label> : cmd_spawn : quiet : exec <path>`) to add the entry. The project supplies only this config file plus the existing `start.sh`; NickelMenu itself remains an external, separately-installed prerequisite (its own `KoboRoot.tgz`), never bundled, vendored, or patched by this project.

**Rationale**:
- NickelMenu's `main` menu target is the documented way to inject rows into Nickel's hamburger/"More" screen — the exact screen the user named, and the same screen that already lists Settings.
- `001-kobo-sudoku`'s research (R3) already vetted NickelMenu as firmware-4.x-compatible and non-private-API; it is already scaffolded in this repo (`dist/.adds/sudoku/nm-sudoku.txt`) though never wired into packaging or documentation. This work finishes that job.
- Satisfies FR-004 (own config built on top of NickelMenu, no alternative menu-injection method) and the constitution's "community-proven surfaces only" test.

**Alternatives considered**:
- *Patching Nickel/libnickel directly to add a menu item*: rejected — reintroduces exactly the firmware-fragility Constitution Principle IV forbids, and duplicates what NickelMenu already solves.
- *NickelDBus*: rejected — same firmware-fragility class as noted in 001 R3, and doesn't specifically address native "More" placement.
- *A KOReader/Plato-style plugin menu*: rejected — requires installing an entirely different reader shell; out of scope and not "NickelMenu" as the user instructed.

## R2. Config placement: fixed filename under `.adds/nm/`, shipped directly in the package

**Decision**: Ship the NickelMenu config pre-placed at `.adds/nm/kobo-sudoku` inside `kobo-sudoku.zip` (superseding the current `dist/.adds/sudoku/nm-sudoku.txt`, which today requires the user to manually copy it into `.adds/nm/` — an undocumented, easy-to-miss step, and not referenced anywhere in `README.md`). Use one fixed, stable filename for this config, never a version-suffixed or timestamped name.

**Rationale**:
- NickelMenu reads every file inside `.adds/nm/` as config (no naming convention required beyond living in that directory), so shipping it there directly removes the manual "copy this file" step entirely — satisfying FR-001 (no computer-side toolchain) and shrinking SC-002's install-time budget.
- A fixed filename is what makes FR-011 (no duplicate entries on reinstall/upgrade) trivial: unzipping the same package again always overwrites the same file in place, rather than accumulating multiple config files with different names, which would otherwise register duplicate `menu_item` lines and show up as two "Sudoku" rows.

**Alternatives considered**:
- *Keep the config inside `.adds/sudoku/` and document a manual copy step*: rejected — this is the status quo the feature exists to fix, and it fails FR-001/SC-002's "no extra computer-side step" bar.
- *An on-device installer script the user runs*: rejected — no general shell access on stock Nickel without more homebrew; unnecessary when direct zip placement already works.

## R3. Menu label and action

**Decision**: Keep the directive's substance unchanged from what is already scaffolded: `menu_item : main : Sudoku : cmd_spawn : quiet : exec /mnt/onboard/.adds/sudoku/start.sh` — label "Sudoku", action reuses the same `start.sh` wrapper 001 already ships (which already handles `cd`, stderr→`crash.log`, and clean exit).

**Rationale**: Reusing `start.sh` verbatim means this feature adds zero new runtime code paths to test or maintain — the KFMon trigger and the NickelMenu entry are two doorbells to the exact same process. This directly satisfies FR-008 (both methods launch the same game, same save).

**Alternatives considered**: *A separate wrapper script for the NickelMenu path*: rejected — pure duplication with no behavioral difference needed, against Principle VI's simplicity preference.

## R4. Idempotent install / no duplicate entries (FR-011)

**Decision**: Rely on (a) the fixed filename from R2, and (b) documenting install as "extract, merging/overwriting `.adds/`" — the same instruction pattern 001 already uses for its own `.adds/sudoku/` and `.adds/kfmon/` merge. No dedup logic is needed in code because there is no runtime code involved here; it is a filesystem-overwrite guarantee established at packaging time.

**Rationale**: The failure mode FR-011 guards against (duplicate menu entries) can only happen if two differently-named config files both exist under `.adds/nm/` and both reference Sudoku. Fixing the filename at packaging time, combined with "extract on top of the existing install" (standard zip-merge behavior via the Kobo's own file import or Windows Explorer), eliminates the failure mode by construction. Verified by the quickstart's reinstall scenario.

**Alternatives considered**: *A first-run cleanup routine that scans `.adds/nm/` for old Sudoku entries and removes stragglers*: rejected — this project has no code running with device shell access outside the game binary itself; over-engineered for a problem a fixed filename already prevents.

## R5. Documentation priority: NickelMenu-first README, KFMon reframed as legacy

**Decision**: Restructure `README.md`'s "Install on your Kobo" section to lead with the NickelMenu prerequisite and install steps as the primary path; keep the existing KFMon steps immediately after, under an explicit "Legacy alternative: KFMon cover-tap" subheading, unchanged in mechanics.

**Rationale**: Directly implements FR-010 from the spec's clarification session (documentation must present NickelMenu as primary, KFMon as legacy/alternative). Both mechanisms remain installable side-by-side per FR-008/US2 Scenario 3 — this is a documentation reordering/relabeling, not a removal of the KFMon path.

**Alternatives considered**:
- *Two fully separate top-level install documents*: rejected — unnecessary duplication for two short procedures that already share the same "extract the zip" step.
- *Removing KFMon documentation entirely*: rejected — explicitly out of scope per the spec's assumptions, and would break existing `001-kobo-sudoku` users who already set up KFMon.
