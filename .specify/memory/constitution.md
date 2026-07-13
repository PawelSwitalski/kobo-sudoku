<!--
Sync Impact Report
==================
Version change: (template, unversioned) → 1.0.0 (initial ratification)
Modified principles: n/a (initial adoption — all principles new)
Added sections:
  - Core Principles (I–VI)
  - Technology Constraints
  - Development Workflow & Quality Gates
  - Governance
Removed sections: none (template placeholders replaced)
Templates requiring updates:
  - ✅ .specify/templates/plan-template.md — generic "Constitution Check" gate, compatible as-is
  - ✅ .specify/templates/spec-template.md — no constitution-specific sections required, compatible as-is
  - ✅ .specify/templates/tasks-template.md — tests are opt-in there; Principle III opts this project in
    for core-logic tests (task generation MUST include them)
  - ✅ specs/001-kobo-sudoku/plan.md — Constitution Check section updated to evaluate v1.0.0
Follow-up TODOs: none
-->

# Kobo Sudoku Constitution

## Core Principles

### I. Portable Core, Thin Platform Layer

All game logic — board rules, puzzle generation, solving/grading, session state, serialization,
and screen/widget layout — MUST live in a portable core with no OS, filesystem, rendering, or
input calls. Device- and host-specific code is confined to `src/platform/` behind the `Renderer`
and `TouchInput` interfaces, and to a thin persistence adapter. The core MUST compile and run on
the development host without any Kobo hardware or toolchain.

Rationale: this is what makes the game testable on a PC, developable without flashing the
device, and portable across the Kobo lineup — the three riskiest goals of the project.

### II. E-ink-First, Grayscale-First UX

The UI MUST be designed for e-ink: partial refreshes for in-play updates, full refreshes on
screen transitions and periodically to clear ghosting, and no UI element that requires frequent
redraws (an always-ticking seconds timer is forbidden; minute-level updates are the ceiling).
Every piece of information MUST be distinguishable in pure grayscale — color is accent only,
never the sole carrier of meaning. Touch targets MUST scale from display DPI, never hardcoded
pixels.

Rationale: the product ships on monochrome and color e-ink devices of several sizes; a UI that
assumes LCD behavior or color semantics fails the spec's portability and usability requirements.

### III. Host-Testable Correctness (NON-NEGOTIABLE)

Correctness-critical core logic MUST have host-run unit tests that pass before a change merges:
puzzle generation (uniqueness of solution, property-tested across many seeds), difficulty
grading bands, board rules (errors, completion, pencil-mark auto-cleanup), and persistence
(round-trip fidelity and corrupted-input recovery). A generated puzzle with zero or multiple
solutions, or a save that cannot survive a round-trip, is a release-blocking defect.

Rationale: on-device debugging is slow and crude (a crash.log at best); correctness must be
established where tests can actually run.

### IV. Firmware-Agnostic Device Integration

The game MUST NOT link against or depend on Kobo's private firmware libraries (libnickel/Qt
internals) or any firmware-version-specific behavior. Device integration is limited to stable,
community-proven surfaces: the e-ink framebuffer via FBInk, evdev input, and launch via KFMon
(with NickelMenu configs as optional extras only). One armhf binary MUST serve all supported
devices; installation MUST remain plain file copy over USB.

Rationale: private-API dependencies broke the entire NickelMenu ecosystem on firmware 5.x;
avoiding them is the difference between "works on my device today" and "works on other Kobos
across firmware updates".

### V. Never Lose the Player's Progress

Session state MUST be persisted after every mutating action (move, mark change, hint) using
atomic write-then-rename. Unreadable or invalid persistent files MUST degrade to defaults
without crashing — a corrupt save costs at most one puzzle, never a crash loop. The app MUST
persist and exit cleanly on termination signals so device sleep/power events cannot corrupt
state.

Rationale: the spec's reliability criteria (SC-005, FR-010, FR-018) define user trust for a
casual e-reader game; violating them makes the game feel broken regardless of features.

### VI. Simplicity and Minimal Dependencies

Prefer the simplest design that satisfies the spec. New third-party dependencies MUST be
vendored, MUST build with the koxtoolchain cross-compiler, and MUST be justified in writing
(in plan.md or a PR description) against the alternative of writing the small amount of code
directly. Header-only libraries are preferred. No frameworks, no speculative abstractions
beyond the two platform interfaces of Principle I.

Rationale: the target is a single small binary on a constrained device; every dependency is
cross-compilation risk and binary-size cost.

## Technology Constraints

- Language: C++17. Device builds use koxtoolchain (`arm-kobo-linux-gnueabihf`) under WSL2/Docker.
- Rendering: FBInk (vendored). Input: raw evdev. Launcher: KFMon primary.
- Persistence: JSON via nlohmann/json (vendored, header-only) in `.adds/sudoku/`.
- Tests: doctest (vendored, header-only), built and run on the host.
- Approved dependency set is exactly the above plus SDL2 (host-only simulator); additions go
  through Principle VI justification.

## Development Workflow & Quality Gates

- Every feature follows the speckit flow: spec → clarify → plan → tasks → implement; the plan's
  Constitution Check gate MUST evaluate Principles I–VI explicitly.
- Task generation MUST include the Principle III test tasks for any work touching core logic;
  tests for a story are written with (or before) the implementation, never deferred past it.
- A change is "done" only when host tests pass and, for user-visible behavior, the relevant
  quickstart.md validation scenario has been run (on device or simulator, as the scenario
  specifies).
- Layout and rendering changes MUST be sanity-checked in grayscale (Principle II) before being
  considered complete.

## Governance

This constitution supersedes ad-hoc practice for this repository. Amendments are made by
editing `.specify/memory/constitution.md` with an updated Sync Impact Report, a semantic
version bump, and propagation to dependent templates and the active feature's plan.

Versioning policy: MAJOR for removing or redefining a principle in a backward-incompatible
way; MINOR for adding a principle or materially expanding guidance; PATCH for clarifications
and wording. Compliance is reviewed at every `/speckit-plan` Constitution Check and re-checked
after design; violations must be justified in the plan's Complexity Tracking table or the
design changed.

**Version**: 1.0.0 | **Ratified**: 2026-07-12 | **Last Amended**: 2026-07-12
