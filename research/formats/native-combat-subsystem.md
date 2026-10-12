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
