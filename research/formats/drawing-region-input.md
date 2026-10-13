# Bounded Region mouse callback observation

`MNMDRE01` version 2 is an optional observation contract, selected by
`MNM_DRAW_REGION_INPUT_FILE`. Production pins No-CD Region table `5c6a54` slot
zero to `4fdad0` and its exact 11-byte prefix before replacing the slot in the
private process. No original file is changed. The original callback runs once
with thiscall receiver, event mask and 40-byte record passed by value; the raw
EAX result, LastError and 512-byte FX state are preserved. The mask and raw
result retain their original values without assigning complete event semantics.

The 64-byte little-endian header contains magic (8 bytes), then 14 DWORDs:
version 1, 2 or 3, row size 256, build marker `40209ca7`, capacity 8192, emitted rows,
dropped calls, numeric owner thread, foreign-thread calls, pending calls,
recorded calls, phase count, and three zero words. Complete decoding requires
zero drops, foreign calls and pending calls; each recorded call has two rows.
Data is buffered and written on normal module/process teardown. Forced teardown
and a missing file cannot count as a completed observation.

| Row DWORDs | Meaning |
| --- | --- |
| 0–7 | Row ordinal, call ID, entry/return phase (1/2), phase ordinal, tick, thread, mask, validity bits |
| 8–17 | Unchanged 10-DWORD by-value mouse record |
| 18–21 | Raw return (zero on entry), LastError, control ID, control activity |
| 22–31 | Hit class, draw index, actor state, numeric pool ID, capacity, slot+1, type, owner, activity, health |
| 32–38 | Camera orientation, three whole and three fraction fields |
| 39–41 | Raw `6903e8` mode, cursor type, selected ring index |
| 42–44 (V2) | Raw validated actor fields +68/+70 and raw count at `6e984c` |
| 45–63 (V2), 42–63 (V1) | Reserved zero |

Validity bits are 1 for exact Region receiver, 2 for readable control, 4 for
readable candidate globals, 8 for camera, 16 for raw mode, 32 for cursor type
and 64 for selected ring index. V2 adds bit 128 for readable raw latch count.
Actor state is 0 for null, 1 for a candidate
validated against current pool capacity/stride/slot, and 2 for an unvalidated
nonnull candidate. No host pointers are serialized. Numeric pool IDs indicate
observed base changes, without allocation ownership or generation guarantees.

Rows are allocated in call pairs. Phase ordinals retain nested callback order;
the strict decoder checks unique complete phases and properly nested returns,
identical arguments/owner, valid candidate shapes and reserved fields. It keeps
unknown raw mask bits and scalar meanings visible. Observation of a delivered
callback or changed scalar does not prove persistent selection, tooltip,
complete queue/control lifetime, source ownership or physical/native parity.

V1 originally recorded actor +4/+a8/+174 under incorrect type/owner/activity
labels. Historical reports and raw bytes remain unchanged. Current decoding
normalizes V1 to type +a8, owner +174, activity +4. V2 writes those correct
offsets in the declared word order and adds raw +68/+70/count observations.
These raw fields do not independently establish complete selection semantics.

The PE32 fixture uses relocated owned pages and a synthetic original callback.
Production address pins remain fixed. Its first fixed-address allocation attempt
is retained as negative evidence; its allocation conflict is unassigned.

Current bounded click-latch analysis requires one uniquely matched original
mask1/mask2pair with stable validated pool/slot/type/owner, raw+68/count/mode
0→1and a later same observed-slot latch after1000ms. The1000msbound is
diagnostic policy. It doesnot establish allocation generations, general
selection/tooltip semantics or exact physical/source causality. Four strict
guards and current static original metadata analysis are retained separately.

## Optional V3 requested actor slot

`--region-track-slot 0..255` requires `--region-input` and selects V3 through
`MNM_DRAW_REGION_TRACK_SLOT`. Tracking disabled retains V2. The same 256-byte
row appends an independent read-only sample after the ordinary hit sample:

| Row DWORDs | Meaning |
| --- | --- |
| 45 | Requested (4), pool valid (1), actor valid (2) bits |
| 46–48 | Local pool ID, capacity, requested slot + 1 (zero if actor invalid) |
| 49–54 | Type +a8, owner +174, activity +4, health +e4, raw +68 and +70 |
| 55 | Requested zero-based slot |
| 56–63 | Reserved zero |

The bounded pool must have capacity 1..256 and contain the requested slot.
The address calculation uses stride e4b with overflow checks; the first 178
bytes must be readable and the actor's first word must equal the requested
slot. Invalid pools or actor slot words refuse data, retaining only valid
request/pool metadata. No original state is written. The hit and tracked
samples share the local pool ID counter; equal IDs do not establish allocation
generation. Strict decoding rejects incompatible flags, capacities, slot
identities, nonzero invalid fields, pair requests or reserved words.

Current owned V3 and disabled V2 executions each pass 32 paired callbacks and
35 forwards with exact result, LastError, full 512-byte FX state and read-only
source-memory guards. The original V3 capture records seven retained actor+68
latch samples without a current actor hit, 6214–24501ms after the left click.
The analysis requires an independently tracked later sample for V3 admission.
Five analysis guards pass. Deselection and complete selection semantics remain
open; these are callback-time observations without synchronized frame proof.
