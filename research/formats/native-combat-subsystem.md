# Native combat subsystem v1

This is an intentional native checkpoint extension, not an original game format.
It occupies `State.systems` inside the existing native world checkpoint. Its
enclosing snapshot length/storage limits and payload checksum remain unchanged.

All integers are unsigned little-endian DWORDs. The header is:

| Offset | Value |
| --- | --- |
| 0 | Eight ASCII bytes `MNMCOM01` |
| 8 | Acquisition range, 1..64 logical cells |
| 12 | Fighter count, 0..32 |
| 16 | Fighter records, each 48 bytes |

Each record contains these twelve DWORDs in order:

1. Actor slot.
2. Actor generation.
3. Maximum health (1..1,000,000).
4. Fixed hit damage (1..1,000,000).
5. Windup ordinary ticks (1..100,000).
6. Recovery ordinary ticks (1..100,000).
7. Current health (0..maximum).
8. Action: idle 0, approaching 1, winding up 2, recovering 3, dead 4.
9. Remaining phase ticks; positive within the corresponding configured bound
   for winding/recovery, otherwise zero.
10. Target present, exactly 0 or 1.
11. Target slot, zero when absent.
12. Target generation, zero when absent.

Records are strictly ordered by actor slot. Every allocated world entity must
have exactly one combat record, resolve by its complete generation, and be a
creature with a movement driver. Living actors are uncleaned; health zero
requires both dead action and cleanup. Idle/dead records have no target;
approaching/timed records require a registered living hostile target on the same
layer within acquisition range. The decoder rejects unknown magic, truncation,
excess count, noncanonical target presence and trailing bytes. The combat session
adds semantic/resource/world validation before publication or restoration.

Continuation uses countdowns rather than host time or pointers. No reservation
cache is stored: existing occupancy and fine-edge claims derive from native
movement state. Attack animation, damage RNG, original creature configuration,
death animation and original-save import/export are outside this format.

## Explicit control extension (NS22)

`MNMCOM02` retains the v1 header and 48-byte fighter prefix, then appends
five DWORDs to each fighter: control (automatic/hold/move/attack = 0..3),
destination-present flag and XYZ. Records are 68 bytes. After the fighters,
a DWORD count precedes up to 64 36-byte pending orders: kind
(move/stop/attack = 0..2), subject identity, target-present flag/identity and XYZ.
Absent fields are zero. Full-generation stale/dead orders are refused during
admission; positions and encoding are checked before publication. Move suppresses
attacks until arrival, then automatic acquisition resumes; Stop holds; Attack
retains its requested target and becomes Hold when that target is lost.

The encoder emits the exact v1 layout for automatic-only states with no queued
controls. Both versions decode. The outer world format is unchanged. Projectile
impact reuses atomic clamped damage/death cleanup; this is an explicit native
policy, not original damage equivalence.

## Immutable health backend binding (NS23)

`MNMCOM03` uses the v2 control records and pending order layout. It adds a
nonzero little-endian QWORD health-rules identity after range/count, before the
fighters: header24, fighter68, pending count4 plus36 bytes per order. Zero is
reserved for legacy fixture rules, represented by v1/v2. Unknown identities are
refused by the app resolver; a session restores only its matching backend.
The stateless No-CD ordinary numerical adapter identity is
`0x4e53323348454131` (`NS23HEA1`). It is a versioned contract identifier, not a
host pointer, executable address or source hash. Provenance remains in execution
reports. New arena sessions select that backend; old v1/v2 arena saves keep the
legacy authored calculator. No old save is silently migrated. The outer native
world and skirmish campaign blobs are unchanged. Health, motion and queued work
remain owned and serializable; no callback or C++ object layout crosses the wire.
