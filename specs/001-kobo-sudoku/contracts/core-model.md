# Contract: Core API (`src/core`)

The portable core exposes these interfaces to the UI layer and tests. No OS calls, no I/O, no rendering — pure logic. Signatures are normative in shape; exact C++ spelling may vary.

## Generator

```cpp
struct Puzzle { std::array<uint8_t,81> givens, solution; Difficulty difficulty; uint64_t seed; };

// Generates a puzzle with exactly one solution at the requested difficulty.
// MUST complete well under the 5 s budget (SC-001); deterministic for a given seed.
Puzzle generate(Difficulty d, uint64_t seed);
```

**Guarantees**: unique solution (SC-004); technique-graded difficulty (FR-002); every given matches the solution.

## Solver (internal to core, contract-tested)

```cpp
int countSolutions(const Grid& g, int limit /*=2*/);   // early exit at limit
Difficulty grade(const Grid& givens);                   // logical-technique grading
```

## Session

```cpp
class Session {
  // Cell-first input (FR-004). All mutators return a ChangeSet: the cell indices
  // whose rendering changed, so the UI can do minimal e-ink partial refreshes.
  ChangeSet commitDigit(int cell, int digit);   // rejects Given cells
  ChangeSet clearCell(int cell);                // Entry cells only
  ChangeSet toggleMark(int cell, int digit);    // pencil mode (FR-005/FR-006)
  ChangeSet applyHint();                        // fills one correct cell (FR-008)

  bool  isError(int cell) const;    // value != solution (FR-007)
  bool  isComplete() const;         // FR-009
  const Cell& cell(int i) const;
  uint32_t elapsedSeconds() const;  // caller feeds active time via tick()
  void  tick(uint32_t activeSeconds);
};
```

**Behavioral guarantees**:
- `commitDigit` clears the cell's own marks and removes that digit from peer-cell marks (FR-006a); the ChangeSet includes every affected peer.
- `applyHint` targets an empty or erroneous cell, never a correct one; increments `hintsUsed`; marks the cell `fromHint`; returns "nothing to do" on a complete board.
- All mutators on a `Given` cell are no-ops returning an empty ChangeSet.
- Serialization: `Session::toJson()` / `Session::fromJson()` round-trip losslessly ([save-format.md](./save-format.md)); `fromJson` on malformed input throws → caller falls back to "no save" (FR-018).
