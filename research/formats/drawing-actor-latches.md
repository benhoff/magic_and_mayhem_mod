# Whole-pool actor latch observations

`MNMDLA01` V1 is an optional bounded read-only diagnostic for No-CD `40209ca7`.
It records active, slot-validated actors independently of the current hit at
`656600`. It reads pool base/capacity at `6def58`, stride `e4b`, active `+4`,
raw latch fields `+68/+70`, type `+a8`, owner `+174` and health `+e4`.
Existing static setter/resetter evidence is in `actor-control-latches.md`.
The count at `6e984c` and mode at `6903e8` remain raw independent observations.

The 64-byte header contains magic then version 1, row size 256, build marker,
capacity 2048, rows, dropped rows, numeric owner thread, foreign calls, 50 ms
periodic cadence and five zero DWORDs. Normal teardown flushes the bounded
memory buffer. A missing/truncated file, drops or foreign owner cannot be admitted.

Each 64-DWORD row contains ordinal/tick/thread/screen (0–3), kind (4: periodic
0, Region entry 1, return 2), exact Region call and phase ordinals (5–6), validity
(7), numeric observed pool ID/capacity (8–9), active/raw68/raw70 counts (10–12),
raw count/mode (13–14), hit slot+1 (15), three 256-bit active/raw68/raw70 bitmaps
(16–39), exact `DrawingInputPointV1` response (40–55), hit state (56: null 0,
validated 1, invalid 2, unreadable 3), first latched slot+1/type/owner/activity/
health/raw68/raw70 (57–63). Bitmaps concern active actors only. Other actors'
identity fields and allocation generations are not serialized.

Validity bits: 1 validated pool, 2 readable count/mode, 4 available cursor point,
8 two equal pool collections with unchanged numeric base/capacity. Bits 1/8
must occur together. Invalid pools retain zero actor data. Active slots whose
stored index disagrees with their current slot refuse the entire collection.
Unsigned range overflow and capacity outside 1..256 refuse it too.

Two equal collections are a diagnostic consistency check, not atomicity, locking,
allocation ownership, ABA protection or a complete resource lifetime proof. No
original lock is taken. Pool IDs count observed numeric base changes only.
Region samples immediately follow the corresponding existing observation on the
same owner thread; callback IDs/phases must join exactly. Cursor points bind to
completed original polling records, not to a new poll, click semantics or a
physically synchronized frame. In metadata mode the frame prefix can remain zero.
Complete selection, tooltip and source/physical/native parity stay separate gates.
