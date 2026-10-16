# Native World Cure component, version1

This authored format occupies the existing native World snapshot's opaque
`State.systems` field. It does not change the native World snapshot format or
decode original `.sav` files. The outer native snapshot supplies corruption
detection and file publication; the embedded Cure lab continuation has its own
version/checksum. All new fields are explicit little-endian DWORDs, never C++
object layouts or host pointers. Booleans are0/1; optional numeric indices use
ffffffff, optional handles have a Boolean followed by slot/generation.

Field order is owned by `apps/cure-world/session.cpp::content`:

1. Magic `CWLD`, version1, then a length-prefixed `MCUR` Cure lab continuation.
2. Counted fixed actor World handles and optional effect World handles.
3. Recipe74's four metadata words and selected normalized ANI records. Each
   record is opcode, signed argument and nine preserved metadata DWORDs.
4. Spatial dimensions/high-water mark, counted cells (terrain/head/otherA/otherB/
   flags), and complete records (active/type,63 parameters, position/fine/initial
   coordinates,12 sentinels, next/cell/initialized, reservation/property/reset).
5. Optional binding flag per record and its relative ANI cursor: pc, optional
   displayed index, active/delay/elapsed/repeats/break. The binding is rebuilt from
   the embedded selected records and metadata, never an external asset path.

World effect entity `state` contains two DWORDs: Cure slot and its independent
pool generation. Entity family/type/target and these words must match the component
mapping. World itself owns entity generation increments, queued-command rejection,
reference clearing, scheduler counters and positional records.

Restore bounds the component to32MiB, embedded Cure bytes to8MiB, actor/effect
maps to4096, cells to128×128×32 and selected records to65536. The product of selected
ANI record bytes and effect capacity is limited to16MiB before materializing player
copies. Actor, effect and presentation counts must exactly match the embedded
core capacities before allocation or player construction. Cell words, dimensions, pool/ring links, cursor fields and metadata labels
are checked. Core Cure validation additionally checks actor/configuration/status
audio domains. Generation handles, entity family/type/owner/position/target and
effect tag/activity/spatial state must agree before publication. Motion components
are rejected; this consumer owns direct grid relocation. Missing old actor
handles are accepted only for inactive enrolled actors, without rebinding a reused
slot. Queued World operations are preserved exactly.

Snapshots contain selected data and current playback state, not ANI file handles,
SPR pixels, wall-clock deadlines or physical audio state. Enrolling newly spawned
actors, multiple independent `systems` consumers and native campaign envelope
embedding are future integration boundaries. This format proves owned continuation
when paired with its source-bound tests; it is not original save compatibility.
