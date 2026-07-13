# Tasks: NickelMenu "More" Menu Launcher

**Input**: Design documents from `/specs/002-nickelmenu-integration/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Tests**: NOT INCLUDED — this feature adds no application/core code (Constitution v1.1.0 Principle
III only mandates host-run tests for core logic, and none is touched here; see plan.md's
Technical Context). Verification is the manual/on-device quickstart.md scenarios plus one
host-side packaging smoke check, both referenced below.

**Organization**: Tasks are grouped by user story. Priority order from spec.md: US1 (P1, MVP) →
US2 (P2) → US3 (P3).

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: US1 launch from More · US2 guided one-time setup · US3 clean removal

## Path Conventions

Single project at repository root (unchanged from `001-kobo-sudoku`): `dist/`, `tools/`,
`README.md`. No `src/` changes — this feature is packaging, configuration, and documentation only.

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Land the new NickelMenu config artifact and retire its superseded predecessor

- [ ] T001 [P] Add fixed-path NickelMenu config `dist/.adds/nm/kobo-sudoku` containing
      `menu_item : main : Sudoku : cmd_spawn : quiet : exec /mnt/onboard/.adds/sudoku/start.sh`,
      per contracts/nickelmenu-entry.md — this exact, stable filename is what makes reinstall
      idempotent (FR-011, research.md R2)
- [ ] T002 [P] Remove the superseded `dist/.adds/sudoku/nm-sudoku.txt` (its content is replaced by
      T001; keeping both would leave two divergent, easy-to-confuse NickelMenu snippets in the repo)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Wire the new config into the package build — nothing in any user story is testable
without this

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T003 Update `tools/package.sh`: create `.adds/nm/` in the staging directory and copy
      `dist/.adds/nm/kobo-sudoku` into it (replacing the old
      `cp "$ROOT/dist/.adds/sudoku/nm-sudoku.txt" "$STAGE/.adds/sudoku/"` line), so every build
      ships the fixed-path config with no manual copy step required on the device (FR-001, FR-011)

**Checkpoint**: `tools/package.sh build/kobo` produces `dist/kobo-sudoku.zip` containing exactly
one `.adds/nm/kobo-sudoku` with the expected content — verify with the host-side packaging check
in quickstart.md (`unzip -l` / `unzip -p`) before any device work.

---

## Phase 3: User Story 1 - Launch Sudoku from the Kobo "More" menu (Priority: P1) 🎯 MVP

**Goal**: A "Sudoku" entry appears in the Kobo's "More" screen once installed, launches the game
directly when tapped, and returns cleanly to Nickel on exit

**Independent Test**: Install the package (with NickelMenu already present) on the device, open
"More", confirm "Sudoku" is listed alongside Settings and any other existing entries, tap it, and
observe the game start; exit and confirm return to the normal Nickel interface (quickstart.md
scenarios 2–4)

### Implementation for User Story 1

- [ ] T004 [US1] Rewrite `README.md`'s "Install on your Kobo" section to lead with NickelMenu as
      the primary path: one-time prerequisite (download NickelMenu's `KoboRoot.tgz` from
      https://pgaskin.github.io/NickelMenu/, copy to the device's hidden `.kobo/` folder, eject)
      followed by install steps (extract `kobo-sudoku.zip` onto the drive root, eject, open
      **More**, tap **Sudoku**); note the confirmed device/firmware (Kobo Libra Colour, firmware
      4.5) and that NickelMenu is firmware-4.x only (FR-001, FR-002, FR-005, FR-009, FR-010)
- [ ] T005 [US1] In the same `README.md` section, keep the existing KFMon cover-tap steps
      immediately below, under an explicit "Legacy alternative: KFMon cover-tap" subheading,
      mechanics unchanged from `001-kobo-sudoku` (FR-008, FR-010)
- [ ] T006 [US1] Manually validate quickstart.md scenarios 2–4 on the confirmed device: the
      "Sudoku" entry appears in More without hiding/reordering Settings or other existing entries,
      tapping it launches the game directly, and exiting returns to the normal Nickel interface
      with the entry still present for next time (FR-001, FR-002, FR-003, FR-006, SC-001, SC-003,
      SC-005)

**Checkpoint**: "Sudoku" is launchable from "More" as the documented primary install/launch path —
this is the MVP for the feature.

---

## Phase 4: User Story 2 - Guided one-time setup (Priority: P2)

**Goal**: A player starting from a factory-fresh device (no NickelMenu installed) can follow only
`README.md` to reach their first puzzle via "More" in under 15 minutes, and installing this method
alongside an existing KFMon install (from `001-kobo-sudoku`) is safe

**Independent Test**: Follow `README.md` from a NickelMenu-less state through to seeing and using
the "Sudoku" entry in "More", using only USB file copy and the device itself (quickstart.md
scenario 8); separately confirm a device with both install methods present launches correctly from
either trigger (quickstart.md scenario 6)

### Implementation for User Story 2

- [ ] T007 [US2] [P] In `README.md`, note that the NickelMenu prerequisite follows the same
      one-time-per-firmware-update pattern as the existing KFMon prerequisite (it survives until a
      firmware update, then the step is simply redone) — so it reads consistently for players who
      already know the KFMon setup from `001-kobo-sudoku` (FR-005)
- [ ] T008 [US2] [P] Add a short "Using both install methods" note to `README.md` confirming it is
      safe to have both the NickelMenu "More" entry and the KFMon cover-tap installed at the same
      time — both launch the same game and share the same save (FR-008)
- [ ] T009 [US2] Manually validate quickstart.md scenario 8 (factory-fresh-equivalent walkthrough
      using only `README.md`, first puzzle reached from More in under 15 minutes, SC-002) and
      scenario 6 (both install methods present and installed together still launch correctly and
      share saved progress, FR-008)

**Checkpoint**: A first-time player can go from nothing installed to playing via "More" using only
`README.md`; coexistence with an existing KFMon install is confirmed safe.

---

## Phase 5: User Story 3 - Clean removal (Priority: P3)

**Goal**: Uninstalling leaves no residual "Sudoku" entry in "More", and reinstalling/upgrading
never produces a duplicate entry

**Independent Test**: Install, confirm the entry appears, follow the documented removal steps, and
confirm "More" no longer lists "Sudoku" while Settings and other entries are unaffected
(quickstart.md scenario 7); separately, reinstall the package and confirm exactly one entry remains
(quickstart.md scenario 5)

### Implementation for User Story 3

- [ ] T010 [US3] Update `README.md`'s "Uninstall" section to add `.adds/nm/kobo-sudoku` to the list
      of paths to delete, alongside the existing `.adds/sudoku/`, `.adds/kfmon/config/sudoku.ini`,
      and `kfmon-sudoku.png` (FR-007)
- [ ] T011 [US3] Manually validate quickstart.md scenario 7 (uninstall removes the entry, other
      entries unaffected, SC-004) and scenario 5 (reinstalling the package a second time leaves
      exactly one "Sudoku" entry, never a duplicate, SC-006, FR-011)

**Checkpoint**: The install → uninstall → reinstall cycle leaves "More" exactly as documented at
every step.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Final pre-release verification across all three stories

- [ ] T012 [P] Run the host-side packaging smoke check from quickstart.md (`unzip -l` / `unzip -p`
      on a freshly built `dist/kobo-sudoku.zip`) as a final pre-release check (FR-011 precondition)
- [ ] T013 Run all quickstart.md validation scenarios end-to-end on the confirmed device (Kobo
      Libra Colour, firmware 4.5) in one pass, confirming no regression to the existing
      `001-kobo-sudoku` KFMon install/launch path

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup (T003 copies the file T001 creates) — BLOCKS all
  user stories
- **User Stories (Phase 3+)**: All depend on Foundational phase completion
  - US1 → US2 → US3 in priority order is the intended sequence (each layers documentation onto the
    same shipped artifact), though US2 and US3 do not modify any file US1 also modifies, so they
    could proceed in parallel once Foundational is done
- **Polish (Phase 6)**: Depends on all three user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational — no dependency on other stories; delivers
  the MVP
- **User Story 2 (P2)**: Can start after Foundational — documents the prerequisite journey and
  coexistence; independently testable without US3
- **User Story 3 (P3)**: Can start after Foundational — documents uninstall/reinstall; independently
  testable without US2

### Within Each User Story

- Doc edits before manual validation (validation task confirms the preceding edits)
- Story complete before moving to the next priority (recommended sequence, not a hard requirement)

### Parallel Opportunities

- T001 and T002 (Setup) can run in parallel — different files
- T007 and T008 (US2) can run in parallel — distinct `README.md` sections, no conflicting edits
  (coordinate to avoid a merge conflict if literally run at the same time by different people)
- Once Foundational (T003) is done, US2's and US3's doc tasks (T007–T008, T010) could proceed in
  parallel with US1's (T004–T005) by different people, since they touch different `README.md`
  subsections — though sequential order (US1 → US2 → US3) is recommended for a single implementer
  to keep the README diff coherent

---

## Parallel Example: Setup

```bash
Task: "Add dist/.adds/nm/kobo-sudoku with the NickelMenu menu_item directive"
Task: "Remove the superseded dist/.adds/sudoku/nm-sudoku.txt"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL — blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: run quickstart.md scenarios 2–4 on-device
5. This alone satisfies the feature's core value: a discoverable "Sudoku" entry in "More"

### Incremental Delivery

1. Complete Setup + Foundational → packaged config ready
2. Add User Story 1 → validate independently → MVP: entry works and is the documented primary path
3. Add User Story 2 → validate independently → first-time factory-fresh journey confirmed
4. Add User Story 3 → validate independently → clean install/uninstall/reinstall cycle confirmed
5. Phase 6 Polish → full quickstart pass, confirm no regression to `001-kobo-sudoku`

---

## Notes

- [P] tasks = different files/sections, no dependencies
- [Story] label maps task to specific user story for traceability
- This feature is packaging/config/docs only — no `src/` changes, so there are no model/service/
  contract-test tasks in the usual sense; contracts/nickelmenu-entry.md is a packaging contract,
  verified by the host-side check (T012) and on-device scenarios, not by a compiled test
- Commit after each task or logical group
- Stop at any checkpoint to validate a story independently
