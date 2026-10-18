# Native wizard continuation

NS87's opt-in wizard encounter uses the existing native command session and
snapshot envelope. This is a native wire contract, not an original executable or
save-game layout. See [wizard admission and evidence](../runtime/wizard-ai.md).

## Immutable profile: MNMWIZ01

All counts are little-endian32-bit unsigned integers. A blob is a count followed
by exactly that many bytes (at most8MiB each). The layout is:

1. Eight literal bytes `MNMWIZ01`.
2. Inner `MNMENC02` encounter blob, scenario10/eight creature slots.
3. Installed WZD blob (`Greek41.wzd`).
4. Installed wizard ANI blob (`Greek Male.ani`).
5. Packed/encrypted `spells.cfg` blob.
6. Packed/encrypted `effectani.cfg` blob.
7. Effect ANI file count, matching the decoded effect animation recipe catalog.
8. One ANI blob per catalog asset, in ordinal order (13 in the pinned fixture).

Trailing bytes refuse. The inner profile owns encrypted creature/HTH rules,
Crocodile ANI, MAP/TTD and crop. Asset path case is resolved uniquely while loading;
the portable profile stores bytes, not installation paths. The profile also
selects the native authored three-spell policy and learned fixture. Gaea's WZD
provides stats; the fixture learned list is independent of WZD starting spells.
All inputs are in the snapshot campaign/profile bytes, so mixed encounter resume
requires no installation, Wine, Qt or original PE.

## Mutable command state: CMD1v20

Version20 retains the version19 continuation layout and appends an optional
wizard state to `NativeProviderState`; older versions omit it. The exact ordered
field definitions are in `apps/command-session/codec.cpp` and the owning structs
in `native_wizard_state.hpp`. Integers use the existing explicit scalar codec;
Boolean/optional/vector tags are validated, vector lengths are bounded, and
scores preserve32-bit float bits. The total command-state limit remains1MiB.
Writing wizard state with an older version is refused, never truncated.

The wizard state owns:

- Frame/next task serial and cast/heal/impact/summon/retirement/decision/mana/
  regeneration/refusal/bounce/expiry counters.
- Planning/production/effect-capacity inputs and ordered controllers. Each
  controller retains actor identity, maximum mana, producer flag,104 learned
  bits, selector, cursor, candidate/best plan, active spell/target/stand/aim/
  score/start/expiry/serial/issued state and next think clock.
-104 spell definitions and recovered cast admission state: costs/modes, observed
  matrix,124 learned bits per owner, notices, fixed-point resources, directional
  muzzle offsets, facing and admission counters.
- Separate250-word strategic RNG state and cursor pair.
- Fixed owned effect slots and generation, selected ANI asset/sequence and
  interpreter state, Cure or projectile state, placement parameters/position/
  sentinels, trajectory and collision caches.
- Eight first-use summon-slot flags and chosen Cure/Sphere animation identities.

Existing creature inventories, health/maintenance, action/motion/route state,
terrain reservations and backend combat state remain in their existing owned
continuation sections. The wizard model contains IDs and values; operation-local
services/verifiers do not enter the wire representation.

Decode/publication checks configuration bounds, clocks, actor/mana identity,
selector validity, plan coordinates/score/serials, effect references/coordinates/
lifetimes and `retirements + active tasks == nextSerial - 1`. Runtime restore also
binds immutable wizard/spell/muzzle/actor definitions to the selected profile.
Exact ANI sequence restoration occurs before wizard work on the next staged step.
Unsupported spell families, polymorph and reused creature identities are excluded.
Any provider/verifier exception discards the entire staged frame.
