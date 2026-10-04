# Magic & Mayhem Reverse Engineering

## Goal

Build a documented, reproducible reverse-engineering and modding toolkit for
Magic & Mayhem (1998), alongside an incremental native modernization of selected
engine systems.

Current work covers build-specific pathfinding and audio reconstruction,
isolated runtime observation and adapters, native Qt/OpenGL presentation,
input and media integration, and native asset loading and audio foundations.
Validate recovered behavior and native implementations in bounded steps before
replacing original work during live execution. Offline reconstruction,
synthetic tests, observation hooks, and live replacement are separate milestones;
record their evidence and remaining boundaries in
`research/runtime/coverage-ledger.md`.

The longer-term gameplay objectives remain:

1. Commander orders for summoned creatures.
2. Per-creature veterancy.
3. A more active mana tempo/economy.

These gameplay features remain outstanding. Engine modernization is current
scope, while behavior validation must remain separate from gameplay balance
changes.

## Rules

- Never modify files in `original/`.
- Treat `original/` as read-only input, including the scripts stored there.
- Run `./tools/original-manifest.sh verify` before and after experiments that
  consume original artifacts.
- `working/` may be modified and may contain generated or installed game files.
- Every binary modification must be performed by a script.
- Before patching a binary, verify the expected original bytes and/or executable
  hash.
- Record discovered file structures under `research/formats/`.
- Record runtime structures, functions, addresses, signatures, and hooks under
  `research/runtime/`.
- Prefer data/config modification over executable patching where possible.
- Prefer isolated runtime hooks over broad executable rewrites.
- Keep experiments small, reversible, and independently testable.
- Separate confirmed findings from hypotheses.
- For reverse-engineering findings, record the evidence and confidence level.
- Do not assume pointer addresses or offsets remain stable across launches unless
  verified.
- Avoid introducing gameplay balance changes while still validating engine
  behavior.

## Repository layout

See [project architecture](docs/architecture.md) for current boundaries,
the proposed target structure, and the incremental migration plan. Proposed
directories do not imply implemented or validated subsystems.

- `original/`: immutable source media and supplied binaries.
- `working/`: disposable generated installations and experiment outputs.
- `tools/`: reproducible preparation, inspection, launch, and patch scripts.
- `reconstruction/`: build-specific models of recovered engine behavior,
  currently pathfinding and audio; keep these separate from native policies
  and application code.
- `runtime/`: injected PE32 observation hooks and adapters, including neighbor
  shadow capture and rendering, input, and media bridges.
- `apps/qt-shell/`: native Qt application hosting, viewport presentation,
  input forwarding, and media playback.
- `renderer/`: native OpenGL surface storage, drawing operations, and capture
  replay, separate from runtime hooks and the Qt shell.
- `assets/`: native asset path resolution, read-only file access, and byte
  comparison tools.
- `audio/`: native PCM loading, sample storage, and voice state foundations,
  separate from recovered engine contracts in `reconstruction/audio/`.
- `research/formats/`: documented on-disk structures and configuration formats.
- `research/runtime/`: documented runtime structures, functions, and hooks.
- `research/`: installation evidence and the immutable-input manifest, alongside
  the format and runtime research directories.
- `patches/`: patch specifications and release metadata, not patched binaries.
- `tests/`: automated checks and repeatable manual test protocols.
- `docs/`: architecture and consequential design decisions, separate from
  reverse-engineering evidence and coverage status.

## Architecture boundaries

- Keep QWidget presentation separate from session lifecycle and orchestration.
- Widgets express semantic actions; keep game addresses, wire offsets,
  Wine/X11 details, and executable staging behind application/adaptor boundaries.
- Native services must not depend on application widgets or build-specific
  reconstruction. Recovered contracts may use native storage/backend interfaces.
- Keep future native simulation/rules independent of Qt presentation and
  legacy hook mechanics; isolate intentional gameplay changes from recovered
  baseline behavior.
- Treat shared channels as versioned wire contracts, not shared host pointers
  or C++ object layouts. Record integration and validation in the coverage ledger.

## Game launch

Once the working installation and launcher have been created, use:

```bash
./tools/run-game.sh
```
