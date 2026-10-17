# Original primitive and source-acquisition lifetimes

The latest incoming commit `d69d082c` covers native pixel routes and empty
rectangle semantics; it does not implement the call-level tracing required by
the retained [first120 interval rejection](draw02-end-to-end.md). Inspection of
`drawing_update_accounting.py`, `whole_update_capture.h` and the exact commit
confirms that callbacks previously retained pending counts without identities.

The additive [lifetime protocol](../formats/drawing-primitive-lifetimes-v1.md)
records every guarded call's entry, request acquisition envelope, original return
and retirement, plus every active call at every source callback. An independent
decoder cross-checks canonical emitted requests and the entire source callback
denominator. Unknown source associations and original allocation ownership remain
explicit. The journal has no effect unless selected by the diagnostic runner.

`drawing_primitive_ownership.py` joins the first120 interval ledger to these
identities. It distinguishes original call return from later physical-observer
acknowledgement: a witness assigned by completion time can belong to an update
after the original call already returned. Such timing is not automatically a
primitive still executing or a dead canvas. This temporal classification preserves
all existing admission gates and every queue-free update.


## Executed original acquisition and reassessment (2026-10-17)

**Confirmed observations, scoped to this run.** The [actual acquisition](draw02-primitive-lifetimes-original120-03-20261017.json)
passes normal original child Quit/exit0, stable source checks and both2927-file
original manifests. It retains460000guarded calls in1840162lifetime rows,
120World queues and122accepted callback observations. The449MiB journal closes
with zero active calls; this is diagnostic overhead, not a production cadence
result. The original child exit is witnessed; launcher exit0 is not independently
witnessed by that report. Owned X server and private prefix stop/wait complete.

All39pending callback calls now have identities and acquisition/return envelopes.
They are outer cursor-copy wrappers on this run's thread432, separate from the
accepted-update thread392; those IDs are not stable across launches:

| Hook/caller | Pending calls | Observed canvas direction |
| --- | --- | --- |
| tag32 / `004a4b01` | 21 | source5 → destination1 |
| tag32 / `004a4c26` | 4 | source1 → destination5 |
| tag39 / `004a4d2f` | 14 | source6 → destination1 |

The guarded entries are `0058c360` (opaque rectangle copy) and `0058ca90`
(keyed rectangle copy), consistent with the existing cursor findings. Their
owned COPY requests contain source/destination generation IDs and rectangles,
**not source pixel payloads**. Knowing their parameter acquisition envelope does
not prove immutable source backing at the later Wine worker's consumption.

The [independent ownership join](draw02-primitive-lifetimes-ownership-01-20261017.json)
classifies every21cross-prefix witness:20original DirectDraw API calls returned
before the update associated with their observer completion began. Physical
acknowledgement and the enclosing wrapper return followed later. One keyed call,
API entry5677/update86, overlaps that update's original-call interval; its source
point phase1 precedes the callback's producer prefix. None is a missing/dead canvas
merely because the sampled prefix lies outside its completion-associated update.
Timing and an overlapping pending wrapper are not exclusive byte ownership.

The [first120 accounting](draw02-primitive-lifetimes-accounting-01-20261017.json)
retains119queues, queue-free113, all4336admitted call intervals/2100inputs and
329primary/source-worker-EGL witnesses with matching pixels. Across the full
capture, [432EGL copy regions](draw02-primitive-lifetimes-egl-01-20261017.json)
match under the established one-channel tolerance, with zero mismatches
and observed maximum channel difference1.42first120updates touch pending boundaries;
21source-point sets cross their associated prefixes. Historical34/19counts and
the original117/118transition observation remain unchanged in their old reports.

The [unchanged complete interval gate](draw02-primitive-lifetimes-interval-gate-01-20261017.json)
executes and returns2for a valid rejection. All four gates stay open: callback
quiescence, per-update source points, all presentation routes and all backing
aliases/writers. Identifying calls and distinguishing late ACKs does not waive
those gates. Step2, all47journey/21operation closures and native parity remain open.
The next ownership work is to bind these cursor COPY source/backing lifetimes
through their actual consumers and establish the encountered alias/writer and
alternate presentation boundaries.

[28Python methods](draw02-primitive-lifetimes-python-controls-05-20261017.json),
[six actual PE32 recorder cases](draw02-primitive-lifetimes-fixture-05-20261017.json)
and thirteen existing whole-update boundary cases pass. Initial CRT-link and
isolated-launch setup failures are retained in [provenance](draw02-primitive-lifetimes-provenance-20261017.json).
The second launch exposes a merged harness regression: capture constructed a
manifest but never wrote `manifest.json`. Exclusive publication before launch is
restored and exercised by the successful actual acquisition. No prior report is
rewritten to claim it ran the repaired harness.
