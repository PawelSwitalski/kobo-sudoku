# Feature Specification: NickelMenu "More" Menu Launcher

**Feature Branch**: `002-nickelmenu-integration`

**Created**: 2026-07-13

**Status**: Draft

**Input**: User description: "I have already connected Kobo libra color, firmware version 4.5... you can check. It is displayed as a D: You have a two different way to instal own software on Kobo. I found you can base on `NickelMenu`. You can check this solution and implement own. I would like to place my \"Sudoku\" game inside the section 'More'. There are some settings and program alright. Remember I want you to implement your own solution. Only base on library that I mentioned. All changes commit on separate git branch"

## Clarifications

### Session 2026-07-13

- Q: With this feature, players will have two launch methods installed side by side (the existing KFMon cover-tap from 001, and this new NickelMenu "More" entry). Should documentation present one as primary/recommended? → A: Make the "More" entry the primary/recommended method; keep the KFMon cover-tap documented as a legacy/alternative for players who already have it set up.
- Q: If a player reinstalls/upgrades the package later (copies the files again over USB), should the spec explicitly guarantee exactly one "Sudoku" entry survives in "More" (no duplicates)? → A: Yes — add an explicit requirement.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Launch Sudoku from the Kobo "More" menu (Priority: P1)

The player opens their Kobo, taps the built-in "More" screen (the same screen that lists Settings and other on-device items), sees a "Sudoku" entry alongside those existing items, and taps it to launch the game directly — no separate trigger (such as tapping a disguised book cover) is needed.

**Why this priority**: This is the entire point of the feature — a discoverable, native-feeling launch point, and it becomes the primary recommended way to launch the game going forward, with the existing library-cover trigger kept only as a legacy alternative.

**Independent Test**: Can be fully tested by installing the package on the device, opening "More", confirming "Sudoku" is listed alongside Settings and any other existing entries, tapping it, and observing the game start.

**Acceptance Scenarios**:

1. **Given** the Sudoku package has been installed to the device and the device has been restarted or Nickel's menus reloaded, **When** the player opens the "More" screen, **Then** a "Sudoku" entry is visible among the existing entries (e.g., Settings), without hiding or reordering the existing ones.
2. **Given** the "More" screen is open, **When** the player taps the "Sudoku" entry, **Then** the game launches and displays its main menu (or resumes an in-progress session per existing save behavior).
3. **Given** the game was launched from the "More" entry, **When** the player exits the game by any normal means, **Then** the device returns to the "More" screen or normal Nickel reading interface, and the "Sudoku" entry remains available for the next launch.

---

### User Story 2 - Guided one-time setup (Priority: P2)

A player who has never installed third-party additions on their Kobo follows written instructions to prepare the device once (installing the NickelMenu tool that lets Nickel display extra menu entries) and then copies the Sudoku package over USB, without needing a computer-side build toolchain or developer knowledge.

**Why this priority**: The launch entry (User Story 1) has no value if the player cannot get it onto the device in the first place. Ranked below US1 because it is a one-time setup path, not the repeated-use path.

**Independent Test**: Can be tested by following the documented steps from a factory-fresh state (NickelMenu not yet installed) through to seeing the "Sudoku" entry appear in "More", using only USB file copy and the device itself.

**Acceptance Scenarios**:

1. **Given** a Kobo device that does not yet have the prerequisite menu-extension tool installed, **When** the player follows the documented prerequisite step, **Then** the device gains the ability to show custom "More" entries (verified by any documented example entry appearing, or directly by Sudoku's entry appearing after the next step).
2. **Given** the prerequisite step is complete, **When** the player copies the Sudoku package to the device over USB and ejects it, **Then** the "Sudoku" entry appears in "More" without any further computer-side action.
3. **Given** the player has both an existing KFMon-based install (tap-the-cover trigger, delivered in an earlier feature) and this "More" entry installed at the same time, **When** either trigger is used, **Then** both launch the same game and share the same saved progress.

---

### User Story 3 - Clean removal (Priority: P3)

A player who no longer wants the game removes it from their device and expects no leftover trace in the "More" screen or elsewhere in Nickel's interface.

**Why this priority**: Good citizenship on a shared system menu — a stale, non-functional entry left behind after uninstalling would be confusing and reflects poorly on the install method, but it does not block the core play experience.

**Independent Test**: Can be tested by installing, confirming the entry appears, then following the documented removal steps and confirming the "More" screen no longer lists "Sudoku" and existing entries (Settings, etc.) are unaffected.

**Acceptance Scenarios**:

1. **Given** the game and its "More" entry are installed, **When** the player follows the documented uninstall steps, **Then** the "Sudoku" entry no longer appears in "More" and no other existing entries are altered or removed.

---

### Edge Cases

- What happens if the player copies the Sudoku package but the prerequisite menu-extension tool is not installed? The "Sudoku" entry does not appear in "More"; no crash or error occurs elsewhere in Nickel, and documentation must let the player recognize and correct this (install the prerequisite, then retry).
- What happens if the player's firmware version does not support the prerequisite tool? Documentation states the confirmed-working firmware; behavior on other versions is best-effort, consistent with the existing portability stance for other Kobo models.
- What happens if the "More" screen already has many entries from other add-ons? The Sudoku entry must appear as one additional row without displacing or corrupting existing ones.
- What happens if the player installs this "More" entry without removing the earlier KFMon cover-tap trigger? Both remain available and independently functional, launching the same game and sharing the same save (see User Story 2, Scenario 3).
- What happens if the player reinstalls or upgrades the package (copies its files over again)? Exactly one "Sudoku" entry remains in "More" afterward — the reinstall MUST NOT leave a duplicate or stale second entry.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The system MUST make a "Sudoku" entry appear in the Kobo's built-in "More" screen once the package is installed and the prerequisite menu-extension tool is present, without requiring any computer-side toolchain at play time.
- **FR-002**: Tapping the "Sudoku" entry in "More" MUST launch the game directly, with no additional trigger step (such as tapping a disguised library cover) required.
- **FR-003**: The "Sudoku" entry MUST be added without hiding, reordering, or otherwise altering any pre-existing entries in "More" (e.g., Settings).
- **FR-004**: The menu-entry integration MUST be built as the project's own configuration and launch script, using NickelMenu strictly as the underlying mechanism that lets Nickel display and act on custom "More" entries; the project MUST NOT implement or bundle any alternative private-firmware patch or menu-injection method to achieve this.
- **FR-005**: Installation instructions MUST clearly state, as a one-time prerequisite, that the NickelMenu tool itself must already be present on the device, since the game's package supplies only the menu-entry configuration, not NickelMenu itself.
- **FR-006**: Exiting the game when launched from the "More" entry MUST return the player to the normal Nickel interface, consistent with existing exit behavior.
- **FR-007**: Uninstalling the game MUST remove the "Sudoku" entry from "More" along with the game's other files, leaving no residual or non-functional entry behind.
- **FR-008**: The "More" launch entry MUST coexist with the existing KFMon-based cover-tap launch method without conflict; both, where installed together, MUST launch the same game and operate on the same saved progress.
- **FR-009**: The system MUST document the confirmed device and firmware combination on which this launch method is verified to work, consistent with the project's existing best-effort stance on other Kobo models and firmware versions.
- **FR-010**: Installation and setup documentation MUST present the NickelMenu "More" entry as the primary, recommended installation and launch method; the existing KFMon cover-tap method (from `001-kobo-sudoku`) MUST remain documented only as a legacy/alternative option for players who already have it set up.
- **FR-011**: Reinstalling or upgrading the package (copying its files to the device again) MUST result in exactly one "Sudoku" entry in "More" — never a duplicate or a second, stale entry.

### Key Entities

- **Menu entry configuration**: The on-device configuration (owned and shipped by this project) that tells the prerequisite menu-extension tool to display a "Sudoku" row in "More" and what to run when it is tapped; not a new gameplay data entity.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A player who already has the prerequisite menu-extension tool installed can copy the package over USB and see "Sudoku" appear in "More" within one device restart or menu refresh, with no computer-side tooling beyond file copy.
- **SC-002**: A first-time player following only the provided written instructions can go from a factory-fresh device to launching their first puzzle from the "More" screen in under 15 minutes, including the one-time prerequisite step.
- **SC-003**: 100% of taps on the "Sudoku" entry in "More" launch the game (or resume the in-progress session) without error, across repeated launches.
- **SC-004**: After following the documented uninstall steps, 0% of test installs show a residual "Sudoku" entry in "More".
- **SC-005**: Existing "More" entries (e.g., Settings) remain visible, correctly labeled, and functional both before and after installing the Sudoku entry, in 100% of verification passes.
- **SC-006**: After reinstalling or upgrading the package at least once, 100% of test installs show exactly one "Sudoku" entry in "More" — never zero or more than one.

## Assumptions

- NickelMenu is treated strictly as a prerequisite the player installs separately, following its own well-established community installation process; this feature ships only the menu-entry configuration and launch script that plug into it, not an installer or patch for NickelMenu itself.
- "More" refers to the Kobo Nickel reading interface's built-in bottom-navigation "More" screen — the same screen that lists Settings and similar on-device items — which is the menu location the prerequisite tool's standard configuration targets.
- The existing KFMon-based install and cover-tap launch (delivered in the `001-kobo-sudoku` feature) is not removed by this feature; it remains installable and functional, but documentation repositions it as a legacy/alternative path once this feature ships, with the NickelMenu "More" entry becoming the primary recommended method. Both may be installed and used at the same time.
- The confirmed target device for this feature is a Kobo Libra Colour on firmware 4.5, connected to a computer as a USB mass-storage drive; other Kobo models/firmware versions are best-effort, matching the existing project stance.
- The phrase "there are some settings and program alright" in the feature request is read as the player observing that the "More" screen already contains Settings and other existing items today, and confirming it is fine for Sudoku to sit alongside them — not a request for a new in-game settings screen (the game's existing Settings screen from `001-kobo-sudoku` is unaffected and out of scope for this feature).
- No new gameplay functionality is introduced by this feature; it is solely an additional installation/launch integration.
