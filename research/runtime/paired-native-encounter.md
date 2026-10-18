# Persistent original/native encounters (NS69)

This fixture advances two independent simulations from the same bounded installed
profile, actor initialization and R250 seed. The original PE32 actor pool, terrain,
ANI, route/search containers, scheduler and random stream persist across ticks.
After initialization no oracle endpoint reads native outcomes. Native32 runs the
normal transactional command session; separate native64 and ASan/UBSan/LSan64
processes restore the portable initial snapshot and compare complete native final
snapshots and per-tick projection digests.

The original tick calls each active actor's `509e50` maintenance/action callback,
then `52a780` routing/decisions and `52a9f0` environment. Body ANI binds at the first
active maintenance, matching native initialization order. Reached original AI,
geometry, direct route, search, ANI, movement, melee, health, stun and physical
death bodies execute together. This replaces the previous trace fixture's
per-poll supplied outcomes with a retained original creature-core encounter.
It does not execute the complete original `World` tick.

## Reproduction and artifacts

Run through the main checkout's storage guard, from this worktree:

```sh
python3 /home/hoff/swdev/magic_and_mayhem_mod/tools/working-retention.py run \
  --reserve-gib 2 -- python3 tools/test-paired-encounter.py \
  --install /home/hoff/swdev/magic_and_mayhem_mod/working/game-nocd \
  --output working/tests/NEW_PAIRED_RUN --claims working/tests/NEW_CLAIMS.json
```

The runner pins `Chaos.exe` SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
checks the original manifest before/after, fingerprints installed inputs and
compiler/configure dependencies, retains logs and refuses existing output paths.
Original32 requires Linux/i386 execution and private FS exception-chain storage
for the original MSVC search container constructors. Linux C++ TLS stays on GS;
no Windows exception handler runs. Original allocators use bounded host storage;
original constructors and destructors execute. Declared endpoint jumps modify
only the private mapped research image. Executable sections are compared with
input bytes plus the explicit jump list, made RX and rechecked after execution.
No original file is patched. Native production and native64 replay need no PE,
Wine, Qt or Windows services.

Each matching tick compares the explicit actor field whitelist in
`tests/paired-encounter-state.hpp`, forward/reverse ANI cursor/display/timing/break state,
valid route prefix and metadata, ordinary health/damage auxiliary words, every
terrain kind/occupant/flag/link, scheduler continuation and all250 RNG words and
both cursors. It excludes raw pointers, allocation layout, presentation state,
unused route tails and fields outside the documented projection. Equality of a
projection does not imply equality of every actor byte or whole-engine state.

A checkpoint consists of a native snapshot containing immutable input bytes,
seed and elapsed ticks, plus an oracle projection. Native state restores directly
in a fresh process. The original reconstructs its own prefix from the immutable
profile and seed, checks its projection against the saved oracle, then continues.
This is a replay-backed oracle checkpoint, not an original save loader or a
serialization of original heap/pointers. It is bounded to8192 total ticks.
On the first mismatch the runner saves before/after native snapshots, before/after
oracle projections and a field/tick diagnostic. An intentional native-only health
fault checks detection and exact replay from the preceding checkpoint. A corrupt
oracle checkpoint must be refused before output publication.

## Findings and native corrections

These findings concern the pinned build. Confidence is high within the tested
ordinary profile and explicit helper domains; untested generalization is pending.

- Idle `50acb0` calls gesture probe `516800`, not object selector `51d7b0`.
  The probe consumes its probability draw **before** rejecting species17 through
  its switch table. The former native binding skipped that draw and immediately
  desynchronized the encounter. The recovered probe preserves gates, hostile
  visibility/sight selection, signed score arithmetic, strict ties, orientation
  and peer history. The helper test supplies sight and score matrices. Installed
  matrix2f0 decoding for gesture-capable species remains pending.
- `513370` first constructs a direct terrain route using `4df6e0`/`4df260`
  trajectory initialization and `4df500` stepping. Occupancy is checked later at
  segment admission. The native request provider now tries this path before its
  existing search fallback. Whole generic actor request/shared region/search
  wrapper equivalence is not claimed.
- Segment completion `517580` records a nearby reachable visible hostile as a
  cached moving opponent. `516ee0` validates or clears that cache before motion.
  These helpers now replace empty native responses. The separate `516d60`
  cached-attack/obstacle branch remains an explicit unimplemented service.
- `509230` reserves anchor bit1 and covered cells bit2; it does not set both on
  every cell. `5093e0` restores terrain classification16 on the checked base row.
  Origin reservation clears link bit0x80. Native conflict refusal, clipped top
  bounds and self-token release remain native policies, not full generic
  occupancy overwrite/cleanup equivalence.
- Ordinary health state aliases dirty/route marker/visibility ages with canonical
  actor state. Repeated stun must clear an old external ANI-break request when a
  new animation starts. Melee target-response reads route renewal `+6ac` or
  recent-response `+6b0`, rather than moving-opponent cache `+6a4`.
- Lethal health runs peer reference repair before death behavior dispatch. Native
  cleanup now follows that ordering. One-hop knockback publishes its destination
  as well as its route. Maintenance, scheduler and health use the same local
  owner0 for this fixture.

The 65536-case helper corpus independently compares `516800`, `517580`, `516ee0`
and `513370` with deterministic supplied sight/orientation/reach/movement/hazard
services. This is separate from the composed encounter, where original geometry,
ANI and these helpers execute together.

## Declared boundaries and remaining work

The scenario uses two ordinary physical rank0/type17, width2/height2 actors,
installed selected encrypted CFG/HTH, Crocodile ANI and a sealed12x12x23 Celtic
Plains crop. Hostile owners are all-visible. World objects, inventory, spells,
regions, owner work, feedback/audio/display and effect ownership are empty.
`tests/paired-encounter-original.hpp:endpoints` is the complete endpoint list;
its allocator and zero-resource/effect services check their admitted domains.
Original pool removal is replaced by the documented retained fixed-identity
boundary, including original occupancy release and peer reset. It is not a proof
of original dynamic `506d30` lifecycle ownership.

Original lookup/config globals, actor callbacks and search containers are fixture
construction, not recovered full creature/World construction. General steering,
shared/region route allocation, cached-attack/obstacle handling, all-species
profiles, water/flying encounters, dynamic actors, original save rebinding,
complete World/campaign integration and live replacement remain outstanding.
The result introduces no balance changes. NS68 evidence remains immutable and
historical; changed shared sources do not retroactively validate old trace claims.

Native provider profile identity now includes gesture score matrices. Rebinding
old native continuations can refuse the changed identity; no snapshot migration
or original-save compatibility is asserted by this milestone.

## Recorded execution

The prospectively bound
[`NS69 report`](paired-native-encounter-bound-ns69-20261015.json) passes with214
compiler/configure/test source fingerprints unchanged. Across three1024-tick
encounters it records90 hits, six stuns/recoveries, one knockback, eight route
completions and two deaths/removals. All65536 helper cases agree across
original32/native64/sanitizers. Complete native snapshots and per-tick projection
digests agree across architectures; six fresh-process checkpoints reach the same
final native snapshot and original projection. A native-only health fault
reproduces exactly at tick137, actor0 `+e4` (327 versus326), including byte-identical
after artifacts. A corrupt oracle checkpoint is refused. Original target text,
immutable media manifests and installed inputs pass their guards.

Five existing native command-session CTests additionally pass; their source-bound
logs and `core-regression-report.json` are retained under
`working/tests/paired-encounter-20261015/`. They do not promote unrelated historical
behavior contracts. Raw comparison artifacts are retained under
`working/tests/paired-encounter-bound-ns69-20261015-v3/`, with the reusable CMake
builds under `...-v1/`. Retained negative/development results include the initial
forward-only reverse-ANI refusal, the `-O3` warning in unchanged
`game/simulation/movement.cpp`, and the explicitly stopped superseded v2 run.
The final native64, original32 and sanitizer comparison uses the recorded `-O1`
configuration; no passing `-O3` build is asserted.
