# Data Model: NickelMenu "More" Menu Launcher

**Feature**: [spec.md](./spec.md)

This feature introduces no gameplay data and no changes to the existing `save.json` / `stats.json` / `settings.json` schemas defined in `001-kobo-sudoku`'s [save-format contract](../001-kobo-sudoku/contracts/save-format.md). It adds exactly one on-device configuration artifact.

## Menu entry configuration

Not a runtime data entity — a static, packaged text file the prerequisite NickelMenu tool reads at Nickel startup (spec's "Menu entry configuration" key entity).

| Field | Value | Notes |
|---|---|---|
| Device path | `/mnt/onboard/.adds/nm/kobo-sudoku` | Fixed filename (see [research.md](./research.md) R2) — reinstall overwrites in place, never duplicates (FR-011) |
| Menu location | `main` (NickelMenu directive) | Renders as a row in Nickel's "More" screen (FR-001) |
| Label | `Sudoku` | Exact text shown to the player |
| Action | `cmd_spawn : quiet : exec /mnt/onboard/.adds/sudoku/start.sh` | Reuses `001-kobo-sudoku`'s existing launch wrapper unchanged (FR-002, FR-008) |

No lifecycle or state transitions: the file is written once per install/reinstall (fully overwritten each time, never appended) and removed as one step of uninstall (FR-007). It carries no per-player state — all game state remains in the unchanged 001 save files.
