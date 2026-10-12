# Original scheduler comparison boundary

Behavior `AU.scheduler-original-cases` is seeded before execution. The selected
No-CD build is SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The recovered scheduler model is described in [voice scheduling](audio-voice-scheduler.md).

The planned comparison executes six original bodies without instruction changes:
selection `0x571ab0`, assignment `0x571be0`, eviction `0x571c30`, retirement
`0x571d20`, old-volume reorder `0x571d80`, and whole-ring clear `0x571ec0`.
Clock and DirectSound vtable methods are scripted boundaries; valid host fixture
objects supply original 32-bit links. Candidate helper addresses remain mapping
debt where the central binary inventory has no corresponding discovered function.

The authored domain covers two-to-five-record rings, head/middle/tail changes,
free/future/expired/looping records, signed volume comparisons, manager gates,
optional output slots, null buffers, status/Stop/position failures and unsigned
clock wrap. Expected complete object, link and output bytes come from the native
model; surrounding guard bytes and untouched wrapper/manager fields must match.
This is independent execution of original code, with scripted API effects. It
cannot establish actual driver behavior, live cadence, malformed pointer/ring
fault equivalence or full manager, source-cache and event-route replacement.

[Fresh comparison evidence](audio-scheduler-original-pass-20261012.json) records
392,778 cases and 439,560 clock/COM events. Original and native selected records,
complete record/link/output bytes, wrapper and manager guards match in every
case; 480,760,272 object/guard bytes are checked. Per-operation case counts are
213,120 selection, 44,544 eviction, 44,544 retirement, 38,250 reorder, 44,544 clear,
and 7,776 assignment. Native address/undefined/leak sanitizer execution also
passes the same matrix. All six original instruction ranges remain unchanged,
source and executable bytes stay stable, and original manifests verify before
and after. Raw execution is retained in
`working/tests/audio-scheduler-original-20261012-second/report.json`.

The first fixture attempt is retained at
`working/tests/audio-scheduler-original-20261012-first/report.json`: native
execution passed, but the i386 compiler rejected a locally scoped callback
retained in a static pointer. The callback fixture now has static lifetime and
explicitly clears its dispatch pointer after each original call. That failed
attempt is not original comparison evidence.

No live hook or default behavior is changed by this comparison milestone.
The broader AU.voice_scheduler registration retains its existing partial
statuses; this new behavior records only the authored original-execution
boundary, and leaves the helper inventory mapping debt explicit.

Selection/eviction/retirement/clear use two-to-four-record rings; assignment
and reorder extend through five. These are authored matrices, not exhaustive
state-space or original allocation-fault coverage.
