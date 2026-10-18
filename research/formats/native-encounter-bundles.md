# Native encounter input and health bundles

These are native fixture/checkpoint formats, not original game formats or saves.
All lengths, crop coordinates, scenario values and backend words are unsigned
little-endian32-bit values. No host pointer or executable bytes are stored.

## Input envelope

| Envelope | Prefix after eight-byte magic | Actors |
| --- | --- | --- |
| `MNMENC01` | crop X, Y, width, height | 2 |
| `MNMENC02` | scenario, crop X, Y, width, height | scenario0:2, scenario1:4, scenario2:8 |

The writer retains01 for scenario0 and writes02 for scenarios1/2. The reader also
admits02/scenario0. Other magic/version/scenario values refuse. Following the
prefix are five blobs, each a32-bit byte length followed by exactly that many
bytes, in this order:

1. `CFG/Encrypted/creature.cfg`
2. `CFG/Encrypted/HTH.cfg`
3. `Creatures/crocodile.ani`
4. `Realms/Celtic/Plains/CPsec01.map`
5. `Realms/Celtic/Plains/Terrain.ttd`

Each blob is limited to4MiB. Truncation, excessive length or trailing data refuses
before decoding/publication. The current constructor admits the documented
selected type17 profile and sealed12x12 crop. Scenarios1/2 deterministically add
an obstacle column, choose nonoverlapping legal stands and initialize actor0's
move request. Their meaning is fixture policy, not a campaign loader contract.
The bundle is retained in the outer native snapshot's campaign bytes so resume
needs no installation or PE file.

## Health backend

The `Continuation.backend` payload begins with `MNMECB01`, followed by one
31-word health projection per actor and six counters: hits, damage, stuns,
knockbacks, deaths, cues. The actor count comes from the enclosing continuation
and selected profile; this payload has no separate count field. Its required
extent is `8 + actors * 31 * 4 + 24`:280,528 or1024 bytes for2,4 or8 actors.
Counter offset is derived from the actor count, preserving the two-actor layout.

Health projection order is health, bar, dirty, cooldown, target, gate9c, expiry,
timer, cacheA, cacheB, marker, flag104, rank, followed by incoming, ages and credit
arrays in their declared order in `reconstruction/combat/health.hpp`. Canonical
actor/execution/maintenance aliases are read from those roots at each operation;
the backend retains auxiliary words rather than becoming a second authority.
The original encounter comparison separately checks bar/damage projections.

Malformed backend magic, extent or profile/pool mismatch refuses. Counter overflow
also refuses the staged operation. Profile identity, native CMD1/version bounds
and outer snapshot integrity checks remain independent.

## NS71 traffic fixture scenarios

MNMENC02 scenario words3/4 select four/eight actors with allied owner pairs;5/6/7
select eight actors sharing an owner. Goals start at each actor's stand before
the common actor0 move request. A retained one-point blocked route starts phase2
for these traffic fixtures; scenario6 instead targets the sealed column. Scenario7
uses test-only original/native interruption adapters; the application does not
inject those events. The installed profile carries the pinned original initial
slope coefficient0x3f781062. These are explicit common fixture inputs, not a
recovered complete original constructor or CFG importer.
