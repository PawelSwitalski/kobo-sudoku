# Data Model: Kobo Sudoku

**Feature**: [spec.md](./spec.md) | **Plan**: [plan.md](./plan.md)

All entities live in the portable core (`src/core`, `src/persist`). Persistent entities serialize to JSON per [contracts/save-format.md](./contracts/save-format.md).

## Puzzle

The generated problem. Immutable after generation.

| Field | Type | Notes |
|-------|------|-------|
| `givens` | 81 × digit 0–9 | 0 = empty; the initial board |
| `solution` | 81 × digit 1–9 | The unique solution (used by error check FR-007, hints FR-008, completion FR-009) |
| `difficulty` | enum `Easy \| Medium \| Hard` | Technique-graded (FR-002) |
| `seed` | uint64 | RNG seed for reproducibility/debugging |

**Invariants**: `solution` is the only solution of `givens` (verified at generation, SC-004); every non-zero given equals the solution at that index.

## Cell (runtime view, part of GameSession board state)

| Field | Type | Notes |
|-------|------|-------|
| `kind` | enum `Given \| Entry \| Empty` | `Given` cells are immutable (FR-004) |
| `value` | digit 0–9 | 0 iff `Empty`; committed full-size digit otherwise |
| `marks` | bitmask 9 bits | Pencil-mark candidates; rendered at fixed slots — bit *d−1* always draws at position row `⌈d/3⌉`, column `((d−1) mod 3)+1` of the in-cell 3×3 layout (FR-005) |
| `fromHint` | bool | Set when filled by hint (FR-008); counts toward `hintsUsed` |

**Rules**:
- Committing a value clears `marks` in this cell (FR-006) and clears bit *value−1* from `marks` of every peer cell (same row/column/box) (FR-006a).
- `isError` is **derived**, not stored: `kind == Entry && value != solution[i]` (FR-007).
- `marks != 0` only when `value == 0`.

## GameSession (persistent: `save.json`)

The single in-progress game (one at a time, per spec assumption).

| Field | Type | Notes |
|-------|------|-------|
| `puzzle` | Puzzle | Embedded |
| `cells` | 81 × Cell | Current board state |
| `elapsedSeconds` | uint32 | Accumulated active solving time; excludes device sleep (edge case) |
| `hintsUsed` | uint8 | Shown at completion (FR-009) |
| `savedAt` | ISO-8601 string | Informational |

**State transitions**:

```
(no save) ──New Game──▶ InProgress ──all cells correct──▶ Completed
InProgress ──exit/sleep/power──▶ persisted; resume restores identical state (FR-010)
InProgress ──New Game──▶ confirmation required (FR-019) ──▶ old save discarded
Completed ──▶ stats updated, save.json deleted, back to menu
Corrupt/missing save.json ──▶ treated as (no save), menu without Continue (FR-018)
```

**Save policy**: written (atomic temp+rename) after every committed move, mark change, and hint — bounds power-loss data loss to the last move (SC-005).

## StatisticsRecord (persistent: `stats.json`)

One record per difficulty level.

| Field | Type | Notes |
|-------|------|-------|
| `difficulty` | enum | Key |
| `completedCount` | uint32 | FR-011 |
| `bestSeconds` | uint32 | Lowest completion time; completions with hints still count |
| `totalSeconds` | uint64 | For average = `totalSeconds / completedCount` |

**Update rule**: on transition to `Completed` only; recorded even when the timer display is hidden (FR-011/FR-012).

## Settings (persistent: `settings.json`)

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `showTimer` | bool | true | FR-012 |
| `schemaVersion` | uint | 1 | Forward-compatibility for all three files |

Unknown fields are preserved on rewrite where practical; unreadable file ⇒ defaults (FR-018 behavior applies to all persistent files).

## Derived/transient (never persisted)

- **Selection**: currently selected cell index (or none) + input mode (`Digit` / `Pencil`) — cell-first model (FR-004).
- **Error set**: computed per render from `cells` vs `puzzle.solution`.
- **Completion**: `∀i: value(i) == solution[i]` — checked after every commit (FR-009).
