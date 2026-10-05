# Native world snapshots v1, v2, v3 and v4

This is an owned native checkpoint format, independent of original Magic &
Mayhem saves. Extension `.mnw` is a sandbox convention. No original save
compatibility or conversion is claimed. All integers use explicit little-endian
encoding; signed coordinates use 32-bit two's-complement bit patterns.

| Header offset | Size | Meaning |
| --- | --- | --- |
| 0 | 8 | `MNMNWLD` followed by NUL |
| 8 | 4 | Version: 1 lifecycle, 2 waypoint movement, 3 sample motion, 4 segment continuity |
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
and conservative decoded-storage accounting. Current storage accounting charges 384 bytes
per slot and per pending command, exact blob lengths, and 28 bytes per route
point, plus 72 bytes for present fine-motion continuation and 84 for completed-segment history. The initial v1 implementation charged 128/32 for slot/command storage;
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
All four versions decode; no original game save is converted by this codec.

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

## V3 animation-sample continuation

V3 retains the v2 layout and adds two booleans after each present motion record's
route array: sample-driver selection, then fine-continuation presence. When the
latter is true, fourteen DWORDs follow (56 bytes):

1. Signed rate, duration, height origin, height delta.
2. Signed accumulator, progress, cumulative X displacement, cumulative Y displacement.
3. Signed fine XYZ.
4. Signed presentation residual X/Y, then unsigned sample index.

The encoder selects v3 when any owned creature selects the sample driver,
including idle/pending/cancelled states without an active continuation. V1/v2
records decode with sample selection false and no continuation. Encoding those
states retains their old bytes. There is no implicit upgrade of old motion saves.

Fine continuation requires a moving creature with sample selection true. Rate
and duration are 1..1,000,000; accumulator is 0..2,000,000; progress is 0..191;
sample index is 0..11. Cumulative XY magnitudes cannot exceed progress. Height
delta is -16..16, and height origin equals current grid Z times 16. Fine XY must
equal grid XY times 32 plus signed cumulative displacement divided by six;
fine Z must equal origin plus height delta times progress divided by 192.
Residual magnitudes are at most 64. Decoding validates these numeric/spatial
relationships before committing state.

The bound navigation driver additionally checks the saved rate/duration against
the route/map profile, directional displacement, progress versus the sample
cursor, cycle residuals and accumulator below duration. Every saved/planned edge
must admit a supported profile. Binding remains the exact external resource
fingerprint; samples need not be duplicated inside the checkpoint.

The independent Python v3 oracle checks complete initial, intra-cell and arrived
bytes, restores a Python-authored checkpoint, and compares every fresh-process
continuation trace. See [fine-motion evidence](../runtime/native-creature-fine-motion.md)
for the tested contracts and the unimplemented original-motion boundaries.

## V4 planar segment continuity

V4 retains the v3 layout but extends each present fine record to eighteen DWORDs
(72 bytes). After the fourteen v3 fields it adds unsigned animation-frame index,
unsigned initial sample index, signed initial residual X and signed initial
residual Y. These distinguish the supplied cycle clock from the absolute cursor
within the 48 owned scalar samples. Legacy-driver records in a v4 world use zero
for these four fields.

After each motion record's sample/fine fields, v4 adds:

1. One-byte continuous-driver selection (requires sample selection).
2. Unsigned current-segment tick count DWORD.
3. One-byte completed-segment history presence.
4. When present, signed direction/vertical/category DWORDs, followed by a full
   eighteen-DWORD fine record: 84 bytes total.

The encoder selects v4 when any creature selects continuity, including pending,
arrived or cancelled states. V1/v2/v3 bytes retain their existing layout. Readers
initialize absent continuity fields to zero; no automatic behavioral upgrade
occurs. Storage accounting is now 384 bytes per slot/command, 72 per present
fine record even for legacy wire versions, 84 per history and 28 per route point.
This tighter native budget policy does not alter older wire formats.

Continuous motion supports zero rate, duration 1..1,000,000, animation index
0..11, current/initial sample indices 0..47, and residual magnitudes at most
1,000,000. Completed history requires progress 192..383 and permits terminal
sample index 48; the driver refuses any subsequent read outside owned storage.
History is planar category zero and vertical zero, with flat grid-based Z.
Current progress is 0..191. Current-segment ticks are at most 100,000, nonzero
exactly when a current fine record exists. History is required after a route
point has been consumed; it can also exist at a replanned prefix's cursor zero.
Cleanup, blocked movement and replacement orders clear history/tick state.

The navigation driver checks history duration/displacement/fine coordinates
against the bound profile and previous grid edge. For an ongoing segment it
reconstructs setup from history, replays exactly the saved tick count, and
compares every fine field before committing state/map. History is not an
authenticated transcript of all past ticks; a well-formed history need not be
provably reachable from a previous checkpoint. Structural decoding alone does
not validate driver/profile semantics. Recomputed checksums cannot bypass
current-segment replay or exact map binding.

Independent Python v4 encoding tests boundary and intra-cell bytes, a
Python-authored checkpoint, fractional speed, turns, prefix replanning and
fresh-process continuation. See [NS04 evidence](../runtime/native-creature-segment-continuity.md).
