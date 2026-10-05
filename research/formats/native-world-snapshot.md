# Native world snapshots v1 and v2

This is an owned native checkpoint format, independent of original Magic &
Mayhem saves. Extension `.mnw` is a sandbox convention. No original save
compatibility or conversion is claimed. All integers use explicit little-endian
encoding; signed coordinates use 32-bit two's-complement bit patterns.

| Header offset | Size | Meaning |
| --- | --- | --- |
| 0 | 8 | `MNMNWLD` followed by NUL |
| 8 | 4 | Version: 1 lifecycle, 2 movement |
| 12 | 4 | Payload byte count, exactly file length minus 24 |
| 16 | 8 | FNV-1a-64 of the payload |

FNV-1a starts at 14695981039346656037, XORs each payload byte, and multiplies
by 1099511628211 modulo 2^64. It detects accidental corruption; it is not an
authenticity mechanism. Header identity/version/length are validated separately.

V1 payload order:

1. Five DWORDs: sequence, tick, phase20, phase90, expansion budget.
2. Three blobs: map identifier, campaign data, system data.
3. DWORD slot count, followed by that many slot records.
4. DWORD pending-command count, followed by that many command records.

A blob is a DWORD byte count followed by exact bytes, including embedded NULs.
There is no string terminator or implicit encoding conversion.
A boolean is one byte and must be zero or one.
A handle is two DWORDs: slot, generation.
An optional handle is a boolean followed by a handle when present.

Each slot is a generation DWORD and an entity-presence boolean. A present
entity then stores family/type/owner DWORDs, signed x/y/z DWORDs, a cleaned
boolean, an optional target handle, and a state blob. Family values are
0 creature, 1 missile/effect, 2 map-linked. The semantic type catalog is caller
supplied. Generation zero is permitted only for an empty retired slot.

Each pending command stores an operation DWORD, subject handle, and optional
target. Operations are 0 set/clear target, 1 cleanup, 2 release. Cleanup/release
must have no target. An inactive/stale subject or command target is deliberately
accepted in the queue and later rejected by tick admission; active entity target
references must resolve immediately against the complete restored slot table.

Limits default to 65,536 slots, 65,536 commands and 64 MiB of encoded payload
and conservative decoded-storage accounting. Current storage accounting charges 256 bytes
per slot and per pending command, exact blob lengths, and 28 bytes per route
point. The initial v1 implementation charged 128/32 for slot/command storage;
the larger typed motion records required a stricter resource policy without
changing v1 wire bytes. It is an explicit
budget policy, not a host `sizeof` ABI. Construction and decoding check bounds
before allocation. Encoders also enforce the DWORD payload-size maximum.
Phase counters must be below 20/90; expansion budget must be at most 53.
Unknown fields/enums, invalid boolean bytes, truncation and trailing data fail.
Future field or semantic changes require an explicit new format version.

Confidence is high for native v1 encoding and tested validation policies.
The independent Python oracle checks exact fixture bytes, every one-byte
mutation/incomplete prefix, separately encoded input, recomputed-checksum
invalid states, and split-process continuation. See
[native world evidence](../runtime/native-world-foundation.md).

## V2 movement additions

The header/checksum and existing fields retain their meaning. Encoders select
v2 when a navigation binding is present and otherwise retain exact v1 encoding.
Both versions decode; no original game save is converted by this codec.

After the three blobs, v2 adds a binding-presence byte (must be 1), signed XYZ
dimension DWORDs and a 64-bit FNV-1a fingerprint of the entire external frozen
map/profile file. Dimensions are positive XY <=1024 and Z <=32. A nonempty map
identifier is required. Native simulation does not interpret capture pointer
tokens or embed map host storage in its save.

After each present entity's state blob, v2 adds a motion-presence boolean.
Present motion then stores:

1. Action DWORD: 0 idle, 1 planning, 2 moving, 3 arrived, 4 blocked,
   5 searchLimited, 6 cancelled.
2. Origin XYZ and destination XYZ (six signed DWORDs).
3. Optional goal handle.
4. Budget, next-waypoint index and route-count DWORDs.
5. Route points: signed XYZ, direction, vertical delta and category, then unsigned
   scalar (seven DWORDs, 28 bytes each).

Only creature-family records may contain motion. Budget is 2..4096; routes
contain at most 16 points. Origin/destination/current position and every point
must be canonical within the binding. Consecutive points must be adjacent,
including toroidal XY seams. Direction is 0..7, vertical delta -1..1, category
0..5. Moving requires a remaining waypoint; arrived requires the destination
and a completely consumed route. Other actions have no route/cursor. Current
position must match origin at cursor zero or the most recently consumed point.
Cleaned creatures cannot have an active action; goal references must resolve.

After each command's optional target, v2 adds a destination-presence byte and
signed XYZ when present. Operation 3 is move and requires a destination;
its optional target is a cancellation guard and supplies coordinates at admission.
Other persisted operations have no destination. Operation 4 (internal motion
update) is a transient system command and is rejected in pending save queues.
Core structural decoding is followed by actual map/profile/edge validation in
`MovementSession::restore`; structural validity alone does not establish that
a saved direction/category/edge agrees with the rebound reconstructed world.

The independent Python movement oracle writes complete expected v2 checkpoints
and checks pending orders, route metadata/cursor, restart traces, map refusal,
all incomplete prefixes/byte mutations and malformed states with recomputed
checksums. See [movement evidence](../runtime/native-creature-movement.md).
