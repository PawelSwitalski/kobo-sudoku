# Quickstart: Install & Validate — NickelMenu "More" Menu Launcher

**Feature**: [spec.md](./spec.md) | Contracts: [nickelmenu-entry](./contracts/nickelmenu-entry.md) | Builds on: [001-kobo-sudoku quickstart](../001-kobo-sudoku/quickstart.md)

## Prerequisites

- A packaged `kobo-sudoku.zip` built per `001-kobo-sudoku`'s build steps, updated per this feature's `tools/package.sh` change (see [plan.md](./plan.md)).
- Kobo device connected over USB (confirmed: Kobo Libra Colour, firmware 4.5, mounts as a drive, e.g. `D:`).
- NickelMenu is **not** required for the host-side packaging check below — only for the on-device scenarios.

## Host-side packaging check (no device needed)

```bash
tools/package.sh build/kobo
unzip -l dist/kobo-sudoku.zip | grep ".adds/nm/kobo-sudoku"   # must list exactly this one path
unzip -p dist/kobo-sudoku.zip .adds/nm/kobo-sudoku            # must print the menu_item line from contracts/nickelmenu-entry.md
```

Expected: the archive contains exactly one file at `.adds/nm/kobo-sudoku` with the documented `menu_item` directive — proves FR-011's "one fixed file" precondition before ever touching the device.

## Install on device (primary path)

1. **One-time prerequisite — NickelMenu**: download `KoboRoot.tgz` from https://pgaskin.github.io/NickelMenu/, copy it into the hidden `.kobo/` folder on the device, eject. The Kobo reboots and installs it.
2. Connect the Kobo over USB.
3. Extract `kobo-sudoku.zip` onto the drive root (merges `.adds/`).
4. Eject safely.
5. Open **More** → confirm a **Sudoku** row appears alongside Settings and other existing items.
6. Tap **Sudoku** → the game launches.

## Validation scenarios (map to Success Criteria)

| # | Scenario | Expected | SC/FR |
|---|----------|----------|----|
| 1 | Host: packaging check above | Exactly one `.adds/nm/kobo-sudoku`, correct contents | FR-011 (precondition) |
| 2 | Device: fresh install per steps above | "Sudoku" appears in More within one restart/menu refresh; existing entries (Settings) unchanged | SC-001, SC-005, FR-001, FR-003 |
| 3 | Device: tap "Sudoku" in More | Game launches directly, no extra trigger | SC-003, FR-002 |
| 4 | Device: exit the game after launching from More | Returns to normal Nickel interface; "Sudoku" entry still present for next launch | FR-006 |
| 5 | Device: extract `kobo-sudoku.zip` onto the device a second time (reinstall/upgrade) | Still exactly one "Sudoku" entry in More afterward | SC-006, FR-011 |
| 6 | Device: install both this feature's package and 001's KFMon cover trigger together, use each in turn | Both launch the game; same save/progress resumes either way | US2 Scenario 3, FR-008 |
| 7 | Device: follow documented uninstall steps (delete the four listed paths) | "Sudoku" no longer appears in More; Settings and other entries unaffected | SC-004, FR-007 |
| 8 | Fresh user test: this doc + zip + USB cable, no prior NickelMenu, no other help | Playing first puzzle from More in under 15 minutes | SC-002 |
