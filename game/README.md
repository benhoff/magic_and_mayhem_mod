# Native world and persistence

`mnm-world` is the first native simulation ownership/checkpoint milestone. It
runs without Qt, Wine, original assets or injected adapters. It implements
bounded entity storage, lifetime handles, transactional tick phases, a bounded
single-creature movement session and native v1/v2 snapshots. It does not yet implement AI, combat/spells, campaign
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
explicit native test policies. Original motion action `0x005104b0`, fine-position
integration, animation/speed/timing, complete original order eligibility and
multi-creature occupancy/scheduling remain unvalidated. Stored scalar/direction
metadata is not a replacement for original velocity or animation.

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
V1 lifecycle checkpoints continue to encode identically; movement uses explicit v2.

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
