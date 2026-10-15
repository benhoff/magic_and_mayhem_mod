# Exact producer/update queue binding

Admit a read-only portable cadence queue point only on the engine owner with the
exact last observed queue event, local queue/caller identity and unchanged live
update fields. Record it at source queue entry and return alongside producer
sequence boundaries, then match the exact event in the independent cadence trace.
No host pointer crosses the point ABI or side journal. Fixtures precede original
capture;120contiguousownedwindows and all presentation/action ownership remain
pending. Counts alone never establish binding.

The getter checks the current engine owner, writable 64-byte output, nonzero
frequency, last event type 8, exact local queue/return caller and unchanged live
counter/accepted-sequence/mode. It preserves LastError and returns a 16-DWORD
version-1 point: size, version, cadence event ordinal, world counter, accepted
sequence, mode byte, owner thread, caller ID (1=`004fd4b0`, 2=`004fd559`), QPC low/
high, frequency low/high, event type 8, active World boolean, two reserved zeros.
Neither local queue nor screen pointer enters the point.

`--producer-update-binding` requires cadence plus producer capture. The side
journal `producer-update-points-0000.bin` uses `MNMDUP01`, a 64-byte header and
96-byte rows. Header words 2–6 are version 1, row size 96, build tag `0x40209ca7`,
row limit 256 and requested queue limit; remaining words are zero. Row words
0–7 are ordinal, phase (entry=1/return=2), source queue ID, owner, producer
sequence, bound canvas ID, normalized caller ID and getter result. Words 8–23
are the portable point. Return points must match the entry exactly; original
queue/caller arguments come from the saved source frame. Phase 4 closes with
reason 1 in word 2, pending flag in word 3, completed queue count in word 4,
final producer sequence in word 5 and failures in word 6; remaining words zero.
A complete decoder rejects pending/failed/inconsistent closure.

Offline analysis checks the exact event ordinal, owner, QPC/frequency, counter,
sequence and selected caller against the independent cadence trace. It also
checks each source queue entry/return and canvas ID lie within the recorded
producer sequence bounds. A complete update-window match remains separate from
retention of all associated presentations, UI composition and resource lifetimes.

Six PE32 cases pass: getter two caller IDs/twelve guards/LastError, producer
entry/return closure and changed point, failed getter, nested queue and wrong
queue refusals. Six malformed parser controls also refuse. Original run 01 binds
all sixteen source queues to exact cadence records and advancing update IDs
1–16, with unchanged return points. It retains 69 complete advancing windows,
332 API/331 post-EGL witnesses and 116 held source pairs through 218 commands.
The source prefix remains sixteen returns, and the full 120-window gate remains
open. The next capture must retain the required composition and presentations
through each whole update boundary, beyond merely extending a queue counter.

Fresh source-only native prediction matches all 102 selected physical witnesses
with zero differences (230 excluded), and external held-source analysis matches
204 selected planes / 97,920,000 pixels with zero differences. The restricted
cohort does not close the longer interval or all presentation/lifetime routes.
