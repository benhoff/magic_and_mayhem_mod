# Magic & Mayhem reverse engineering and native modernization

A reproducible reverse-engineering and modding toolkit for Magic & Mayhem
(1998), with an incremental native port of selected engine systems. The campaign
host combines Qt/OpenGL presentation with the original PE32 game through Wine
and isolated adapters. Native sandboxes and labs exercise bounded simulation
and recovered contracts separately.

Implementation, original comparison, integration, and live replacement are
separate milestones. Use the [coverage ledger](research/runtime/coverage-ledger.md)
for demonstrated scope and remaining boundaries. Commander orders, veterancy,
and mana economy enhancements remain longer-term gameplay objectives.

## Start here

| Task | Entry point |
| --- | --- |
| Understand responsibilities and dependencies | [Project architecture](docs/architecture.md) |
| Contribute code or research | [Contributor rules](AGENTS.md) and [coverage accounting workflow](research/runtime/coverage/README.md) |
| Launch the prepared original game | `./tools/run-game.sh`; see its `--help` for launch, check, and smoke modes |
| Use the native Qt host | [Qt shell guide](apps/qt-shell/README.md), starting with `./tools/run-qt-shell.sh` |
| Explore progress and remaining work | [Coverage atlas](apps/coverage-ui/README.md), launched with `python3 tools/serve-coverage-ui.py` |
| Find checks and experiment protocols | [Test guide](tests/README.md) and the relevant component README |
| Understand rendering takeover scope | [Live drawing replacement plan](docs/live-drawing-replacement.md) |

For an unprepared checkout, inspect `./tools/prepare-working.sh --help` and follow
the storage rules below before creating an installation. The root
`run-x86.sh` and `run-asahi.sh` scripts provide host-specific launch/setup entry
points; their `--help` describes requirements and options.

## Directory map

| Location | Responsibility and guide |
| --- | --- |
| `apps/` | Application entry points, presentation, controllers, and diagnostic harnesses; see the application guides below |
| `game/` | [Native simulation and checkpoint ownership](game/README.md), including bounded movement and combat services |
| `assets/` | [Read-only native asset access and format loaders](assets/README.md); [resource identities](assets/resources/README.md) |
| `audio/` | [Native PCM storage, mixing, playback, and audio bridges](audio/README.md) |
| `renderer/` | [Native OpenGL surface operations and replay](renderer/README.md); [shared scene rendering](renderer/scenes/README.md) |
| `protocols/` | [Versioned wire contracts, schemas, and generated bindings](protocols/README.md) |
| `compat/legacy/` | [Host-side original scene adapters and replay](compat/legacy/README.md) |
| `runtime/` | Code injected into the original PE32 process: observation hooks and bounded adapters; [render hooks](runtime/render/README.md) and [pathfinding shadow capture](runtime/shadow/README.md) |
| `reconstruction/` | Build-specific recovered algorithms, separate from native policies; local guides for [pathfinding](reconstruction/pathfinding/README.md), [motion](reconstruction/motion/README.md), [animation](reconstruction/animation/README.md), [creature AI](reconstruction/creature-ai/README.md), [audio](reconstruction/audio/README.md), and [rendering](reconstruction/rendering/README.md) |
| `tools/` | Reproducible preparation, inspection, staging, launch, patch, capture, and validation workflows |
| `tests/` | [Automated checks, independent references, fixtures, and manual protocols](tests/README.md); experiment runners also live under `tools/` |
| `research/formats/` | [On-disk structures and wire-format research](research/formats/README.md) |
| `research/runtime/` | [Runtime findings, original addresses, evidence, and confidence](research/runtime/README.md) |
| `research/runtime/coverage/` | [Behavior register, source census, exact reviews, and preserved accounting history](research/runtime/coverage/README.md) |
| `docs/` | Architecture, rendering replacement direction, and [storage policy](docs/working-retention.md) |
| `patches/` | [Patch specifications and release metadata](patches/README.md) |
| `original/` | Immutable source media and supplied binaries; read-only input |
| `working/` | Generated installations, builds, captures, worktrees, and retained experiment outputs; ignored by Git |

## Native applications and labs

| Application | Purpose and build guide |
| --- | --- |
| [Skirmish](apps/skirmish/README.md) | Playable native arena with authored rules and a recovered ordinary-health backend; no Wine or original assets required |
| [Melee lab](apps/melee-lab/README.md) | Two-creature ordinary melee and checkpoint continuation using an installed profile |
| [Creature lab](apps/creature-lab/README.md) | Bounded targeting, footprint reach, melee, stun, and encounter continuation |
| [World scene](apps/world-scene/README.md) | Diagnostic terrain/creature presentation and native movement controls |
| [Terrain preview](apps/terrain-preview/README.md) | Terrain, region, and lighting diagnostics |
| [Sprite scene](apps/sprite-scene/README.md) | Sprite queue and scene diagnostics |
| Drawing session | Legacy drawing observer and presentation diagnostics; [CMake entry point](apps/drawing-session/CMakeLists.txt) |
| World and [combat sandboxes](apps/combat-sandbox/README.md) | Headless ownership, movement, combat, and checkpoint harnesses, currently built through [game/](game/README.md) |

Build from each component's documented CMake source directory. There is currently
no root CMake entry point. Keep native host builds separate from injected PE32
build tooling, and use the storage guard for builds and experiments that produce
output.

## Original inputs and working storage

Read [AGENTS.md](AGENTS.md) before changing the repository. Never modify
`original/`; verify its manifest before and after experiments that consume it:

```sh
./tools/original-manifest.sh verify
```

The working-storage target is 1280 GiB, with at least 100 GiB of filesystem free
space. Run storage-producing experiments and large builds through
`python3 tools/working-retention.py run --reserve-gib <estimated-output> -- <command>`.
See [working storage retention](docs/working-retention.md) for inventory,
protected evidence, worktrees, and archive restoration. Cleanup candidates require
review; generated output can still be referenced evidence.

For changed code, tests, research, or coverage metadata, follow the
[coverage accounting workflow](research/runtime/coverage/README.md), including
the audit and change gate. Preserve historical evidence and its recorded hashes;
directory or documentation changes do not establish new behavior validation.

## Evidence storage

After cloning, run `python3 tools/evidence-store.py fetch --verify`. Git stores
small evidence references and a SHA-256-pinned archive manifest. The audit,
coverage UI and research readers resolve original report bytes directly from
SQLite, without expanding thousands of JSON files. Export an individual report
with `python3 tools/evidence-store.py export <research-path> --output <new-file>`.
Configuration, schemas, fixtures and editable registers remain JSON. Original
report bytes and source fingerprints remain unchanged. See
[the storage workflow](research/runtime/coverage/README.md#sqlite-evidence-and-automated-bookkeeping).
