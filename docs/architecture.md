# Project architecture

Design direction recorded 2026-10-04. This document describes current boundaries
and an incremental target structure. Proposed directories and interfaces below
are not claims that those components exist or have been validated.

## Purpose

Keep one repository for reverse engineering, the incremental native port, and
future game enhancements. Separate recovered behavior from native implementation
choices and intentional gameplay changes through module responsibilities and
dependencies. QWidget menus are an acceptable native implementation choice;
preserving gameplay behavior does not require preserving the original UI code.

The current application is a hybrid: a native Qt host cooperates with the
original PE32 game through Wine, injected adapters, and shared channels.
Reconstructed algorithms and native services exist, but their presence does not
establish complete live replacement of a gameplay subsystem. Consult the
[coverage ledger](../research/runtime/coverage-ledger.md) for implementation,
integration, validation, confidence, and remaining boundaries.

## Current structure

| Location | Responsibility |
| --- | --- |
| `reconstruction/` | Build-specific recovered algorithms and contracts, with explicit evidence and scope |
| `runtime/` | Injected PE32 observation hooks and adapters |
| `apps/qt-shell/` | Qt host, viewport, launch lifecycle, input forwarding, and media integration |
| `assets/` | Native asset resolution, read-only access, and comparison |
| `audio/` | Native PCM storage and playback foundations |
| `renderer/` | Native OpenGL surface operations and capture replay |
| `game/simulation/` | Native bounded entity ownership, transactional tick phases and single-creature movement through a native navigation interface |
| `game/persistence/` | Native-v1 through v6 checkpoints, staged restoration and POSIX publication; separate from original save decoding |
| `apps/world-sandbox/` | Headless lifecycle/movement/checkpoint harness and frozen reconstructed navigation/sample-motion adapters |
| `apps/world-scene/` | Bounded Qt/OpenGL diagnostic terrain and creature presentation from owned movement checkpoints; explicit visual fixture, separate from installed MAP admission |
| `tools/` | Preparation, inspection, staging, launch, patch, and validation workflows |
| `tests/` | Automated checks, independent references, fixtures, and manual protocols |
| `research/` | Reverse-engineering evidence, formats, runtime findings, and coverage |
| `patches/` | Patch specifications and release metadata |
| `original/` | Immutable source artifacts |
| `working/` | Disposable installations, builds, captures, and experiment outputs |

Existing separation is useful and should be retained. The main pressure point
is `apps/qt-shell/main.cpp`: its main window also owns launch orchestration,
channel creation, frame polling, input, media transitions, and cleanup. Adding
menus should lead to extracting those responsibilities into a session controller.

Shared frame, input, and media layouts currently have definitions in injected
code, Qt clients, and Python tooling. They need a common protocol owner as the
interface grows. Audio reconstruction sources are separated on disk, but their
target is currently defined in `audio/CMakeLists.txt`; build ownership should
eventually follow source ownership.

## Target structure

Grow toward this layout as concrete work requires it. Do not create empty engine
frameworks or move existing files solely to match the tree.

```text
apps/
  qt-shell/
    main.cpp
    ui/                    # Widgets, menus, dialogs, viewport, diagnostics
    application/           # Session controller, navigation, user actions
game/
  simulation/              # Native world state and gameplay systems
  rules/
    classic/               # Explicit baseline rules
    enhanced/              # Intentional gameplay changes
assets/
audio/
renderer/                  # Reusable native services
protocols/                 # Shared frame, input, media, and command contracts
compat/
  legacy/                  # Host-side connection to the original game
runtime/                   # Code injected into the PE32 process
reconstruction/            # Recovered behavior and contracts
docs/
  architecture.md
  decisions/               # Short records when consequential decisions arise
research/
tools/
tests/
patches/
original/
working/
```

`compat/legacy/` would own host-side bridge clients, Wine/X11 integration, and
the legacy session implementation. `runtime/` remains the injected side of that
connection. The two processes communicate through protocols, rather than sharing
host pointers or C++ object layouts.

`game/` now contains a bounded native ownership/tick/checkpoint foundation; see
[its implemented scope](../game/README.md). AI, combat/spell and campaign systems
remain pending, as does original-save resource rebinding.
Classic and enhanced rules should share simulation machinery. Add policies or
data where behavior differs; avoid duplicating the engine or distributing
enhancement flags through unrelated services. A classic rules profile provides
a behavior comparison baseline without requiring identical presentation.

## Dependency and ownership boundaries

For new code and refactors, use these boundaries:

- Widgets emit user intentions and display application state. They do not
  interpret executable addresses, shared-memory offsets, or Wine window IDs,
  and do not stage or patch executables.
- The application/session controller owns lifecycle and coordinates UI,
  session adapters, and native services. It maps actions to the currently
  available implementation and reports success, failure, and capabilities.
- Legacy adapters translate application requests and observed engine state
  into versioned protocol exchanges. Build-specific addresses, ABIs, and hook
  mechanics belong on the injected side and in documented research.
- Native services expose operations and data without depending on application
  widgets or enhanced gameplay rules. Their backends may use Qt where useful.
- Native simulation and rules remain independent of QWidget, Wine/X11, and
  injected-hook details. Gameplay changes belong here as ownership becomes native.
- Reconstruction preserves explicitly scoped recovered contracts. It may use
  native storage or backend interfaces where needed, as audio already does;
  native services must not acquire a dependency on build-specific reconstruction.
- Shared protocols define wire formats, versions, bounds, synchronization,
  statuses, and ownership. Keep wire definitions usable across PE32 C,
  host-side C++, and tooling, without application-widget dependencies. Shared
  constants or generated bindings can reduce duplication; retain independent
  reference checks for encoded bytes and behavior.

Qt is appropriate for widgets, presentation, media, and selected platform
backends. Removing Qt from all existing services is not a prerequisite for
clean separation. Keep future simulation rules and recovered algorithms free
of presentation dependencies.

## QWidget menu migration

A menu emits semantic actions such as opening new-game setup, loading a save,
changing settings, or quitting. The session controller delegates these actions
to a legacy implementation today and a native implementation when available.
Introduce only the interface required by actual menu work; a speculative
general-purpose engine abstraction is unnecessary.

```text
QWidget menu
    -> application/session controller
        -> legacy adapter -> protocol -> injected adapter -> original game
        -> native implementation, when available
```

The controller should distinguish session lifecycle from the visible screen.
Showing a menu does not establish that the original simulation has paused or
that its original menu has been suppressed. Define ownership of:

- Startup, movie, menu, legacy-screen, gameplay, error, and shutdown transitions.
- Input focus, suspension, and release of held keys/buttons across transitions.
- Whether simulation continues while a native screen is visible.
- Requests, acknowledgements, failures, and cancellation where supported.
- Settings application and persistence, including any behavior still delegated
  to the original engine.

Frame callbacks must respect screen ownership so received game/movie frames do
not displace a native menu. Keep QWidget mutations on the Qt GUI thread. Execute
legacy game actions at a verified engine-thread point, with no direct calls from
the Qt process into game addresses.

The current bridge does not establish a semantic menu-action contract. Recover
and observe original menu readiness and dispatch before replacing them. A widget
can first be exercised with a fake controller. Validate protocol/ABI behavior
separately, then live transitions before suppressing original work. Retain a
legacy fallback while integration remains incomplete.

See the [main-menu migration assessment](../research/runtime/main-menu-qt-migration.md)
for action-specific unknowns, bounded integration steps, and validation, and
[layout findings](../research/formats/main-menu-layout.md) for static inputs.

## Recovered behavior and enhancements

| Kind of change | Owner | Validation focus |
| --- | --- | --- |
| Recovered pathfinding or audio contract | `reconstruction/` | Agreement within documented original-build scope |
| QWidget menu, native renderer, safer asset backend | Application or native service | Intended semantics, lifecycle, and integration |
| Commander orders, veterancy, mana tempo | Native gameplay systems and `game/rules/`, as introduced | Explicit new behavior and balance, separate from baseline equivalence |

An enhancement that still requires the original game needs a bounded legacy
adapter and reproducible patch specification where applicable. Record that
integration honestly; do not hide changed rules inside recovered models.
Gameplay balance changes remain separate from engine behavior validation.

## Build and incremental migration

1. Extract session lifecycle and orchestration from the Qt main window as menu
   work requires it, preserving existing launch and shutdown contracts.
2. Add menu widgets under `ui/` and application actions/state under
   `application/`. Exercise them independently of the game.
3. Establish shared protocol ownership and move host-side compatibility code
   behind the application boundary in bounded steps.
4. Give each library ownership of its CMake target. Add a root native CMake
   entry point to compose targets without breaking useful standalone builds.
   Keep injected PE32 build tooling distinct from the host toolchain.
5. Introduce native gameplay modules and classic/enhanced rules when concrete
   systems and behavioral differences require them.

For each step, run checks appropriate to changed behavior and update evidence
when warranted. Offline reconstruction, synthetic checks, live observation,
and live replacement remain separate milestones. Directory moves alone do not
advance coverage.

## Documentation ownership

- This document owns the overall architecture and migration direction.
- `AGENTS.md` holds concise contributor rules and directory responsibilities.
- Component READMEs describe implemented interfaces, local ownership, builds,
  and checks; link here for cross-component design.
- `research/formats/` and `research/runtime/` own recovered facts, hypotheses,
  build restrictions, addresses, and confidence.
- The coverage ledger owns implementation and validation status.
- Add short records under `docs/decisions/` when consequential alternatives
  need a durable rationale. Record context, decision, and consequences; do not
  create records for every routine implementation choice.
