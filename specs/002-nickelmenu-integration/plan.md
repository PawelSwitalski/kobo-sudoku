# Implementation Plan: NickelMenu "More" Menu Launcher

**Branch**: `002-nickelmenu-integration` | **Date**: 2026-07-13 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `/specs/002-nickelmenu-integration/spec.md`

## Summary

Add a "Sudoku" entry to the Kobo's built-in "More" screen (the bottom-nav menu that also lists Settings) using NickelMenu as the sole underlying menu-extension mechanism. The project ships only its own NickelMenu config file and reuses the existing `start.sh` launch wrapper unchanged — no new application code. Per the spec's clarification session, this becomes the primary, documented install/launch path; the existing KFMon cover-tap method from `001-kobo-sudoku` remains installed and functional as a legacy alternative. Packaging (`tools/package.sh`) is updated to place the config at a fixed path under `.adds/nm/` so repeat installs never create duplicate entries, and `README.md` is restructured to lead with the NickelMenu setup.

## Technical Context

**Language/Version**: N/A for new code — no application source changes; this feature is packaging, configuration, and documentation only (a shell packaging-script edit + a NickelMenu config text file, both plain POSIX sh / text per the existing `001-kobo-sudoku` conventions)

**Primary Dependencies**: [NickelMenu](https://pgaskin.github.io/NickelMenu/) — external, user-installed prerequisite (its own `KoboRoot.tgz` patch), not linked or vendored; reuses the existing `start.sh` wrapper and `sudoku` binary from `001-kobo-sudoku` unchanged

**Storage**: N/A — no new persisted data; same `save.json` / `stats.json` / `settings.json` under `.adds/sudoku/` as 001

**Testing**: No new host-testable logic (no core code changes, so Principle III adds no test tasks here); validation is manual/on-device via `quickstart.md`, plus a host-side packaging smoke check (unzip and inspect the fixed config path) that needs no device

**Target Platform**: Same as 001 — Kobo e-readers on stock firmware; this feature is specifically confirmed on the connected Kobo Libra Colour, firmware 4.5 (NickelMenu is firmware-4.x-only per 001's research R3, consistent with the confirmed device)

**Project Type**: Packaging/config addition to an existing embedded native application (no new project/module)

**Performance Goals**: Menu entry appears within one device restart/menu refresh (SC-001); tapping it launches the game with the same responsiveness already established in 001 (no new perf surface)

**Constraints**: Must not implement or bundle any private-firmware patch or alternative menu-injection tool beyond NickelMenu (FR-004); must not require any computer-side toolchain at play time (FR-001); must guarantee exactly one menu entry survives reinstall/upgrade (FR-011); must not alter or hide pre-existing "More" entries (FR-003)

**Scale/Scope**: One new config file, one packaging-script change, one contract doc, one quickstart doc, README restructuring; no new source files under `src/`

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Evaluated against Constitution v1.0.0 (ratified 2026-07-12):

| Principle | Status | Evidence in this plan |
|---|---|---|
| I. Portable core, thin platform layer | ✅ Pass (N/A) | No core or platform code touched; reuses existing `Renderer`/`TouchInput` backends and `start.sh` unchanged |
| II. E-ink-first, grayscale-first UX | ✅ Pass (N/A) | No UI changes; the launch surface is Nickel's own "More" screen, not this app's UI |
| III. Host-testable correctness | ✅ Pass (N/A) | No core logic added; nothing new for the doctest suite to cover |
| IV. Firmware-agnostic device integration | ⚠️ Amendment recommended | The design stays within stable, community-proven surfaces — NickelMenu is the same firmware-4.x-only, non-private-API mechanism 001's research already vetted (R3); no new firmware risk. But the constitution's current wording ("KFMon primary...NickelMenu configs as optional extras only") is narrower than spec FR-010, which requires documenting NickelMenu as the *primary* recommended path. See Complexity Tracking. |
| V. Never lose the player's progress | ✅ Pass (N/A) | Same binary, same save files, same atomic-write persistence as 001; launch path has no bearing on save integrity |
| VI. Simplicity, minimal dependencies | ✅ Pass | NickelMenu is not vendored or linked (same precedent as KFMon, already an unvendored external prerequisite in 001); no new build-time dependency introduced |

**Post-Phase-1 re-check**: Design still introduces no new code dependency, no new architectural seam, and no core/platform changes. The only open item is the Principle IV wording gap, tracked below and recommended for a `/speckit-constitution` amendment; it does not block this plan because the underlying technical constraint (firmware-agnostic, community-proven surfaces only) is fully satisfied — only the *documentation-priority* phrasing needs updating to match. Pass, with that follow-up noted.

## Project Structure

### Documentation (this feature)

```text
specs/002-nickelmenu-integration/
├── plan.md                          # This file
├── research.md                      # Phase 0 output
├── data-model.md                    # Phase 1 output
├── quickstart.md                    # Phase 1 output
├── contracts/
│   └── nickelmenu-entry.md          # Phase 1 output — More-menu entry contract
├── checklists/
│   └── requirements.md
└── tasks.md                         # Phase 2 output (/speckit-tasks — NOT created by /speckit-plan)
```

### Source Code (repository root)

```text
dist/
├── .adds/
│   ├── sudoku/                      # unchanged from 001 (binary, start.sh, assets); nm-sudoku.txt removed from here
│   └── nm/
│       └── kobo-sudoku              # NEW: fixed-name NickelMenu config, ships directly in the zip
└── kfmon/ ...                       # unchanged from 001 (legacy alternative path)

tools/
└── package.sh                       # UPDATED: stage .adds/nm/kobo-sudoku from dist/.adds/nm/kobo-sudoku

README.md                            # UPDATED: NickelMenu install becomes the lead "Install on your Kobo" path;
                                      # KFMon section kept, reframed as a legacy alternative
```

**Structure Decision**: No new source directories. This feature only adds one config file to the existing `dist/` package template, updates the existing `tools/package.sh` staging list, and restructures install documentation. It reuses `001-kobo-sudoku`'s binary and `start.sh` completely unchanged.

## Complexity Tracking

> Fill ONLY if Constitution Check has violations that must be justified

| Violation | Why Needed | Simpler Alternative Rejected Because |
|---|---|---|
| Constitution Principle IV wording ("NickelMenu configs as optional extras only") is narrower than this plan's documentation priority (FR-010: NickelMenu path documented as primary) | The user explicitly asked for an own solution based on NickelMenu, placed in the native "More" menu, specifically because it is more discoverable than the existing cover-tap trigger — that discoverability gain is the entire value of this feature, and it only materializes if the docs actually lead with it | Keeping KFMon documented as primary while shipping the NickelMenu entry as an undocumented or footnote option would contradict spec FR-010 and defeat the discoverability goal; the technical constraint the principle actually protects (no private-firmware APIs, community-proven surfaces only) is still fully honored, so changing the *wording* to allow either path to be documented as primary is the smaller, correct fix rather than reworking this feature's scope. Recommend running `/speckit-constitution` to update Principle IV's wording (and the Technology Constraints "Launcher" line) before or alongside implementation. |
