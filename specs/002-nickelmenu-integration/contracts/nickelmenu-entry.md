# Contract: NickelMenu "More" Menu Entry

Extends `001-kobo-sudoku`'s [install-layout.md](../../001-kobo-sudoku/contracts/install-layout.md) package with a NickelMenu-based launch entry in the Kobo's "More" screen, promoted to the primary documented install method by this feature (FR-010).

## Prerequisite (unchanged mechanism, new documentation weight)

NickelMenu must already be installed on the device — a one-time step identical in shape to 001's KFMon prerequisite: download NickelMenu's `KoboRoot.tgz` (https://pgaskin.github.io/NickelMenu/), copy it into the hidden `.kobo/` folder on the device, eject; the device reboots and installs it. This project does not install, patch, or vendor NickelMenu itself (FR-004, FR-005).

## Package layout addition (zip root = device root)

```text
.adds/
├── sudoku/               # unchanged from 001: binary, start.sh, assets
│                          # (nm-sudoku.txt removed from here — superseded by .adds/nm/kobo-sudoku below)
├── nm/
│   └── kobo-sudoku        # NEW — fixed filename, NickelMenu config (this feature)
└── kfmon/
    └── config/
        └── sudoku.ini     # unchanged from 001 — legacy alternative launch path

kfmon-sudoku.png            # unchanged from 001 — legacy cover-tap trigger image
```

## `kobo-sudoku` config contents

```text
menu_item : main : Sudoku : cmd_spawn : quiet : exec /mnt/onboard/.adds/sudoku/start.sh
```

- `main` → Nickel's "More" screen (FR-001).
- Label `Sudoku` (FR-002).
- Action reuses `001-kobo-sudoku`'s `start.sh` verbatim — same process, same save files, same crash-log behavior (FR-008).

## Compatibility contract

- Confirmed working: Kobo Libra Colour, firmware 4.5 (this feature's target device). NickelMenu is firmware-4.x only (per `001-kobo-sudoku` research R3); on firmware 5.x devices the "More" entry will not appear, but the KFMon path is unaffected and still shipped, so the game remains launchable (FR-009; best-effort elsewhere per spec assumptions).
- Reinstall/upgrade: extracting the package again overwrites `.adds/nm/kobo-sudoku` in place — never a second file, never a duplicate "Sudoku" row (FR-011; see [research.md](../research.md) R4).
- Uninstall: delete `.adds/sudoku/`, `.adds/kfmon/config/sudoku.ini`, `.adds/nm/kobo-sudoku`, and `kfmon-sudoku.png` — removes both launch paths and leaves nothing behind in "More" (FR-007).
- Independence from the KFMon path: both configs are read independently by their respective tools (NickelMenu vs. KFMon) and both invoke the same `start.sh`; installing or removing one does not affect the other (FR-008; edge case in spec.md).
