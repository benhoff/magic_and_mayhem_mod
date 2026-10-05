# Native world and persistence

`mnm-world` is the first native simulation ownership/checkpoint milestone. It
runs without Qt, Wine, original assets or injected adapters. It implements
bounded entity storage, lifetime handles, transactional tick phases, a bounded
movement session with opt-in multiple same-profile creatures and native
v1/v2/v3/v4/v5/v6 snapshots. It does not yet implement AI, combat/spells, campaign
triggers or a playable world. The original engine remains responsible for live
gameplay. Native checkpoints are distinct from original version-20 saves.

```sh
cmake -S game -B working/build/native-world -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/native-world -j4
ctest --test-dir working/build/native-world --output-on-failure
working/build/native-world/mnm-world-sandbox create /tmp/example.mnw
working/build/native-world/mnm-world-sandbox inspect /tmp/example.mnw
working/build/native-world/mnm-world-sandbox resume /tmp/example.mnw /tmp/resumed.mnw 200
```

The `create` command supplies synthetic creature/effect data and pending cleanup
for admitted idle ticks. The movement commands below exercise an actual route
and position progression in a headless harness. Existing output files are refused.

## Single-creature movement

```sh
python3 tools/create-movement-fixture.py /tmp/movement-map.bin
working/build/native-world/mnm-world-sandbox move /tmp/movement-map.bin \
  /tmp/moving.mnw 1 1 1 5 1 1 2
working/build/native-world/mnm-world-sandbox trace /tmp/moving.mnw 6
working/build/native-world/mnm-world-sandbox resume /tmp/moving.mnw /tmp/arrived.mnw 6
working/build/native-world/mnm-world-sandbox inspect-json /tmp/arrived.mnw
```

The arguments after the output are start XYZ, destination XYZ and admitted tick
count. Tick 1 plans; tick 2 advances to `(2,1,1)`. Saving at that point retains
the original route and cursor. Resumption reaches `(5,1,1)` at tick 5, with the
same positions and checkpoint bytes as uninterrupted execution.

`MovementSession` consumes a native `Navigation` interface. Only
`apps/world-sandbox/frozen_navigation` depends on build-specific pathfinding
reconstruction. It owns a bounded `MNMWLD01` frozen map/profile and wires the
complete existing legality/cost/neighbor/search chain, including the recovered
16-waypoint output. It admits at most 4,096 map cells and one moving creature
with the captured type/profile. This fixture is generated synthetic data;
it does not reconstruct a live or installed MAP world's entity occupancy.

Native move orders are queued at the tick boundary. Planning occurs in decisions;
maintenance advances at most one waypoint per admitted ordinary tick, rechecking
legality first. A completed 16-waypoint prefix requests a fresh plan from the
new position when the destination lies beyond it. Alternate mode skips movement;
search suppression postpones planning. Unreachable searches become `blocked`,
and budget exhaustion becomes `searchLimited` without following a partial
closest-node fallback. The retained per-request budget defaults to 300; this
slice does not debit the original shared scheduler budget or persist search queues.

Typed actions are idle, planning, moving, arrived, blocked, searchLimited and
cancelled. Motion stores origin/destination, route metadata, next waypoint,
budget and an optional generation-checked goal guard. A guarded order captures
the target's coordinates when admitted and cancels if that entity is released;
it does not continuously chase a moving target. Cleanup cancels movement;
release removes the actor and invalidates incoming goals. Stale orders are
counted as rejected. Replacing an order prevents any previous waypoint from
executing after its admission.

The route helpers follow the recovered contracts. One-waypoint-per-tick
advancement, exact-target admission, goal guards and the native action enum are
explicit native test policies. NS03 separately validates selected motion-action arithmetic and route consumption.
NS04 validates bounded planar segment setup and speed continuation. NS05
integrates selected forward ANI event production. Complete
original animation/profile coverage, complete original order eligibility and
multi-creature occupancy/scheduling remain unvalidated. Stored scalar/direction
metadata is not a replacement for original velocity or animation.

## Bounded animation-sample movement

`move-fine` uses the same arguments as `move` and selects the recovered arithmetic
adapter for ordinary forward category 0/4 profiles. With the synthetic fixture,
tick 2 keeps grid position `(1,1,1)` and advances fine position to `(42,32,16)`;
ticks 3/4 reach X 52/62, and tick 5 consumes `(2,1,1)` and snaps X to 64. XY uses
32 fine units per cell and Z 16 per layer. Save during those intra-cell ticks
and `resume` retains the exact progress, accumulator, displacement and cursor.

```sh
working/build/native-world/mnm-world-sandbox move-fine /tmp/movement-map.bin \
  /tmp/fine-moving.mnw 1 1 1 5 1 1 2
working/build/native-world/mnm-world-sandbox trace /tmp/fine-moving.mnw 15
```

This opts into native v3. Existing `move` and v1/v2 checkpoints retain their
pacing. The independent motion model lives in `reconstruction/motion/`; native
simulation sees only a navigation driver interface. Scalar rates come from the
recovered route result, duration and twelve samples from the bound profile.
The twelve-sample cycle event, zeroed segment/interruption state and grid-based
Z origin are bounded native policies. `move-continuous` below adds recovered
planar category-zero setup and speed continuity; complete original setup, terrain
height offsets and callbacks remain open. `move-ani` adds selected ANI event production. Unsupported profiles fail transactionally. See
[fine-motion evidence](../research/runtime/native-creature-fine-motion.md).

## Planar segment continuity

`move-continuous` accepts the same arguments and opts into native v4. It uses
recovered category-zero planar setup rather than the predicted route rate.
Successive matching segments retain rate, fractional accumulator, excess
progress, sample cursor, animation clock and cycle residual snapshots. A turn
resets progress and selects the appropriate sample bank while retaining the
admitted speed/accumulator. New orders and cancellation clear continuation as
an explicit native policy.

```sh
working/build/native-world/mnm-world-sandbox move-continuous /tmp/movement-map.bin \
  /tmp/continuous.mnw 1 1 1 5 1 1 6
working/build/native-world/mnm-world-sandbox resume /tmp/continuous.mnw \
  /tmp/continuous-restored.mnw 12
```

The checkpoint owns completed-segment history and current-segment tick count.
Restore checks the map/profile and replays the bounded current segment to verify
all saved continuation fields before committing. The driver owns all 48 scalar
samples and refuses a cursor outside that storage. This mode retains a supplied twelve-frame event clock. `move-ani` below instead
uses the recovered ANI controller. Vertical/category-four setup, reverse and special
profiles, terrain offsets and live behavior remain open. See
[segment continuity evidence](../research/runtime/native-creature-segment-continuity.md).

## ANI-driven planar movement

`move-ani MAP ANI BASE OUTPUT SX SY SZ TX TY TZ TICKS` uses the existing recovered
forward controller instead of the supplied twelve-frame clock. `BASE` explicitly
selects eight contiguous directional sequences; it does not recover type/config
selection or compass-to-resource mappings. Only version-5 ANI assets and planar
category-zero movement are admitted in this mode.

```sh
working/build/native-world/mnm-world-sandbox move-ani /tmp/movement-map.bin \
  /path/to/movement.ani 8 /tmp/ani-moving.mnw 1 1 1 5 1 1 6
working/build/native-world/mnm-world-sandbox resume /tmp/ani-moving.mnw \
  /tmp/ani-restored.mnw 14
```

Each admitted movement substep advances the recovered ANI controller, including
record delays, repeat/jump controls and raw events. Event 2 restarts the selected
sequence and resets the sample cursor/residual snapshot and substep counter.
Matching segments retain controller phase; turns and new orders restart the
selected sequence. Other gameplay events and sequence stops are refused before
committing the tick. A cursor outside the 48 owned motion samples is also refused.
ANI call timing follows movement substep admission, with no invented frame rate.

Native v5 owns the complete ANI bytes, selected base and ongoing/completed
controller state: next/display record indices, active, delay, elapsed, repeats
and break flag. Resuming uses the saved ANI bytes even if the source ANI file is
edited or deleted. The external map still requires exact rebinding. Restore
replays both arithmetic and controller state before committing resources/world;
`restoreResources` lets a resolver inspect these owned resource bindings.

The decoder's pure byte implementation is separate from file loading, so the
headless adapter and native world remain free of Qt. This prototype copies
bounded state and reconstructs an owned player per tick; it does not establish
real-time performance. Original action selection, other animation events,
reverse/special/vertical motion, sprite presentation and live validation remain
open. See [ANI motion evidence](../research/runtime/native-ani-motion.md).

## Terrain-aware movement

`move-terrain` uses the same arguments as `move` and selects ordinary terrain
height inputs, sloped edges and category-zero/four setup, including pure vertical
movement. `move-terrain-ani` uses the same arguments as `move-ani` and combines
this with owned ANI continuation. Both select native v6; earlier modes retain
their existing layouts and pacing.

```sh
python3 tools/create-movement-fixture.py /tmp/terrain-motion.bin --terrain-profile slope
working/build/native-world/mnm-world-sandbox move-terrain /tmp/terrain-motion.bin \
  /tmp/climbing.mnw 1 1 1 5 1 2 6
working/build/native-world/mnm-world-sandbox trace /tmp/climbing.mnw 18
```

Fixture profiles `terrace`, `slope` and `vertical` are synthetic frozen inputs.
The latter admits category-four vertical examples `(1,1,2)` to `(1,1,3)` and
back. They do not restore an installed MAP's entities or occupancy. Terrain
offsets are bounded to -16..16; segment differences to -32..32. Special creature
height helpers and unsupported motion categories remain refused. V6 owns the
completed edge origin so height/profile validation and ongoing replay survive
layer changes and prefix replanning. ANI selection remains an explicit base.

Trace output resolves terrain-aware fine position even at segment boundaries.
`inspect-json` is structural and reports `fine: null` when a terrain-aware actor
has no ongoing fine state; use `trace FILE 0` to resolve the bound map.
See [NS06 evidence and limits](../research/runtime/native-terrain-motion.md).

## Ownership

`simulation/` owns state and tick orchestration. `persistence/` owns explicit
encoding, validation, staged restoration and POSIX file publication. Native
simulation has no Qt, reconstruction or runtime-hook dependencies.

Entity handles are `(slot, generation)` within one world/snapshot lineage.
They are not legacy indices or globally unique IDs. Slots never compact.
Activation takes the first reusable free slot; exhaustion fails without
changing the world. Release removes the entity, clears incoming references,
and increments its generation. Generation overflow retires the slot permanently.
Cleanup keeps the entity active and clears its outgoing target; this represents
the cleanup/release distinction, not complete original death semantics.
The three entity families currently share one bounded pool, a native policy
that does not reproduce original separate backing arrays/high-water bounds.

Every snapshot owns all entity data, target identities, scheduler counters,
pending commands and caller-supplied campaign/system bytes. V1 map paths are
identifiers only; movement v2 also stores map dimensions/content fingerprint
and requires a matching resource on session restoration. Opaque byte fields have
no invented gameplay semantics. Future systems must define their own schemas,
put all continuation state into the world, and avoid persisting host pointers.

## Tick boundary

The caller explicitly supplies admission, alternate mode and search suppression.
No timer, universal tick rate, pause inference or wall-clock dependence is added.
Admitted ticks drain queued commands, advance the sequence, and, in ordinary
mode, advance the unsigned tick counter and run maintenance, decisions and
secondary-creature phases. Effects, map, queued-map and audio phases follow.
The selected decision budget resets to 53 or zero, with 20/90 phase rollover.
These constants/order come from static evidence; the native orchestration is
not an original full-loop reconstruction or measured scheduling replacement.
Expansion debits, resume indices and per-creature stagger selection are absent.

A system callback receives immutable staged state and emits target/cleanup/
release/move/internal-motion commands. Commands become visible before the next phase. Invalid stale
identities are rejected and counted. Invalid operation encodings, resource-limit
failures or callback exceptions abort the tick and preserve all committed world
state, including pending commands. Recursive stepping and direct world mutation
inside a callback are rejected. Callbacks must be deterministic and avoid
external side effects: the transaction cannot undo I/O or external state.
Activation currently occurs explicitly between ticks; queued spawning,
health/damage and event dispatch belong to later system milestones.

Deterministic continuation means the same saved state and same input/system
implementation produce the same world bytes. It does not imply deterministic
networking, original AI equivalence or general restored gameplay resources. Existing handles and
entity pointers must not be reused across unrelated worlds or restoration;
resolve handles again after mutation.

## Persistence

The [native format](../research/formats/native-world-snapshot.md) has explicit
little-endian fields, size/version checks and a corruption checksum. Parsing
checks count/storage bounds before allocating nested data and rejects dangling
active references, unknown enums and trailing bytes. Pending stale requests are
preserved so continuation can report their rejection at the tick boundary.
Decode and validation complete before `World::restore` commits a state.
Movement restores use `MovementSession::restore`: the resolver loads an owned
map first, checks its exact binding, profile, route progression and every saved
edge, then commits both map and state. A missing/changed resource or invalid
route retains the existing world and its map. The map fingerprint covers exactly
the byte buffer decoded by the adapter, preventing a hash/parse reopen mismatch.
V1 lifecycle and v2 waypoint checkpoints continue to encode identically;
sample motion uses explicit v3; continuous segment motion uses v4; ANI-driven continuation uses v5; terrain-aware continuation uses v6.

The file writer validates/encodes first, writes a private sibling temporary,
syncs and closes it, atomically publishes with overwrite refusal by default,
and syncs the parent directory. Explicit overwrite uses rename. Failures before
publication throw and retain the old destination. A failure after publication
returns `durable=false`, with an explanation: callers must distinguish an
already-published snapshot from a pre-publication error. The directory must
already exist. Files default to private `mkstemp` permissions. Crash/power-loss
behavior and non-POSIX publication are not validated.

The current prototype copies bounded state for mutations and ticks. Allocation,
release scans and save preparation are not measured real-time policies.

See [foundation validation](../research/runtime/native-world-foundation.md),
[movement validation and boundaries](../research/runtime/native-creature-movement.md)
and the [coverage ledger](../research/runtime/coverage-ledger.md).

The [native movement scene](../apps/world-scene/README.md) presents this owned
terrain/ANI continuation with explicit diagnostic terrain assets and a shared
depth queue. Rendering reads simulation state without advancing an ANI clock.

## Stationary blockers (NS10)

The opt-in frozen adapter can combine one motion driver with stationary
same-profile creatures. `MovementSession::spawnBlocker` stores an ordinary native
entity without a motion driver; owned generational occupancy is rebuilt from
world slots. Planning and each next-edge check consume the current world view.
Cleanup/release removes blocking and a late obstruction stops/reset movement.

Use `mnm-world-sandbox move-occupied MAP OUTPUT SX SY SZ TX TY TZ BX BY BZ TICKS`,
then the existing `resume` or `trace` commands. The checkpoint resource identity
includes the occupancy policy; existing wire versions and default-mode behavior
remain supported. [Contract and evidence](../research/runtime/native-stationary-occupancy.md)
separate this native policy from recovered original occupancy predicates.

## Multiple moving creatures (NS11)

The opt-in multi-movement frozen adapter admits up to 32 same-profile motion
drivers. Conservative logical swept boxes preserve waiting routes and controllers;
ongoing fine edges retain their reservations across ticks and checkpoint restore.
Maintenance uses slot order and planners rotate priority while sharing 53 actual
search expansions per tick. Persistent conflicts have no deadlock resolution.

`mnm-world-sandbox move-pair` and `move-pair-fine` take a frozen MAP, output
checkpoint, two XYZ starts and targets, then ticks; `resume` recognizes the saved
policy fingerprint. This is a native policy with synthetic headless evidence;
original scheduler/occupancy, physical collision and multi-creature Qt presentation
remain separate. [Contract and evidence](../research/runtime/native-multi-creature-movement.md).

## Multiple-creature presentation (NS12)

The [native scene](../apps/world-scene/README.md) now presents multiple NS11
terrain-motion creatures using independent saved ANI displays and fine positions.
`move-pair-terrain-ani` creates two-actor checkpoints with an owned ANI binding.
Rendering joins their bodies into the mixed terrain depth queue and exports native
slot/generation identities. This remains diagnostic native presentation; original
multi-creature scene equivalence and live integration remain open.

## Scene move controls (NS13)

The native scene now selects individual available creature drivers and queues
explicit target-cell moves through `MovementSession`. Step applies the command;
saving first preserves it for restore. Full-generation selection stays outside
simulation/checkpoint state and refuses stale/cleaned actors.
[Scope and validation](../research/runtime/native-scene-orders.md) keep original
input mappings and commander/summoned-creature gameplay separate.

## Mouse move orders (NS14)

The native scene now selects creatures from displayed sprite coverage and queues
right-click terrain-cell moves through the same Orders/MovementSession path as
numeric controls. Commands still apply at Step and survive Save before stepping.
Presented identities and explicit terrain standing cells remain transient view
metadata; stale handles cannot target reused slots.
[Scope and evidence](../research/runtime/native-scene-picking.md) keep original
mouse/ray mappings and gameplay permissions separate.

## Native stop orders (NS15)

Operation 5 queues a payload-free stop at the tick boundary, using existing native
cancellation/reset semantics. `cancelQueuedMoves` atomically removes only one
full-generation subject’s pending moves without altering active motion. FIFO order,
queue budgets and rollback remain enforced. Pending stops require snapshot v7;
older checkpoint contracts stay strict. See [scope](../research/runtime/native-stop-orders.md).
