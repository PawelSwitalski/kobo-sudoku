# Contract: Device Package & Launch Integration

Installation = unzip one archive to the Kobo's USB mass-storage root (e.g. `D:\`), eject, done (FR-013, SC-002). One-time prerequisite: KFMon installed (documented in quickstart).

## Package layout (zip root = device root)

```text
.adds/
├── sudoku/
│   ├── sudoku                # armhf binary (koxtoolchain build)
│   ├── start.sh              # launch wrapper (see below)
│   ├── assets/               # fonts (bundled TTF), icons
│   ├── nm-sudoku.txt         # optional NickelMenu snippet (FW 4.x users copy to .adds/nm/)
│   └── (save.json / stats.json / settings.json created at runtime)
└── kfmon/
    └── config/
        └── sudoku.ini        # KFMon watch config

kfmon-sudoku.png              # "book cover" trigger image at device root
```

## KFMon config (`sudoku.ini`)

```ini
[watch]
filename = /mnt/onboard/kfmon-sudoku.png
action = /mnt/onboard/.adds/sudoku/start.sh
```

Tapping the "Kobo Sudoku" cover in the Kobo library launches the game. Works on firmware 4.x and 5.x (KFMon is inotify-based; NickelMenu is FW-4.x-only — see research R3).

## `start.sh` obligations

- `cd` into `/mnt/onboard/.adds/sudoku`, set `LD_LIBRARY_PATH` if any bundled `.so` (target: none — static/vendored).
- Run `./sudoku`; on exit, control returns to Nickel (KFMon handles process supervision).
- Log stderr to `.adds/sudoku/crash.log` (truncate per run) for field debugging.

## Optional NickelMenu entry (`nm-sudoku.txt`, FW 4.x only)

```text
menu_item : main : Sudoku : cmd_spawn : quiet : exec /mnt/onboard/.adds/sudoku/start.sh
```

## Compatibility contract

- Binary: ARMv7 hard-float, linked against koxtoolchain glibc floor → runs on every Nickel-era Kobo (Touch C and newer), including sunxi (Sage/Elipsa) and 2024 colour devices (Clara/Libra Colour) via FBInk's device abstraction.
- Layout: all geometry computed from `Renderer::info()` (width/height/dpi) — no hardcoded pixel positions (FR-015).
- Uninstall: delete `.adds/sudoku/`, `.adds/kfmon/config/sudoku.ini`, `kfmon-sudoku.png`.
