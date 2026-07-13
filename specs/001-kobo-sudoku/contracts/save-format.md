# Contract: Persistent File Formats

Location on device: `.adds/sudoku/` (internal storage root). All files JSON, UTF-8, `schemaVersion` field required. Writes are atomic: write `<name>.tmp`, `fsync`, `rename`. Any unreadable/invalid file is treated as absent (FR-018) — never a crash.

## `save.json` — in-progress game (FR-010)

```json
{
  "schemaVersion": 1,
  "savedAt": "2026-07-12T20:30:00Z",
  "puzzle": {
    "difficulty": "medium",
    "seed": 1234567890123,
    "givens":   "530070000600195000098000060800060003400803001700020006060000280000419005000080079",
    "solution": "534678912672195348198342567859761423426853791713924856961537284287419635345286179"
  },
  "elapsedSeconds": 754,
  "hintsUsed": 1,
  "cells": [
    { "i": 2, "v": 4 },
    { "i": 3, "v": 6, "hint": true },
    { "i": 5, "m": [2, 5, 9] }
  ]
}
```

- `givens`/`solution`: 81-char digit strings, row-major, `0` = empty given.
- `cells`: sparse — only player-touched cells. `v` = committed digit (1–9), `m` = pencil marks (sorted, 1–9), mutually exclusive. `hint` default false. Cell indices of givens never appear.
- Validation on load: string lengths, digit ranges, `v`/`m` exclusivity, no entry at a given index, solution consistency. Any failure ⇒ discard file.

## `stats.json` — per-difficulty statistics (FR-011)

```json
{
  "schemaVersion": 1,
  "difficulties": {
    "easy":   { "completed": 12, "bestSeconds": 340, "totalSeconds": 6120 },
    "medium": { "completed": 4,  "bestSeconds": 810, "totalSeconds": 4400 },
    "hard":   { "completed": 0,  "bestSeconds": 0,   "totalSeconds": 0 }
  }
}
```

Average = `totalSeconds / completed` (computed, not stored). Updated only on puzzle completion; includes completions with hidden timer (FR-012) and with hints.

## `settings.json` — preferences (FR-012)

```json
{
  "schemaVersion": 1,
  "showTimer": true
}
```

Missing file or missing key ⇒ documented defaults. Unknown keys preserved on rewrite.
