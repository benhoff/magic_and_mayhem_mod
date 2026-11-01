# Native first-World watchdog investigation

This batch starts at `922062334`. It establishes a reproducible host-liveness
failure mechanism and repairs an execution-proof defect. It **does not repair or
retire `RN31.G.first-world-watchdog`**. The callback or host scheduling/I/O delay
responsible for that historical failure remains unknown. Later finite passes do
not close it, and no watchdog or ownership guard has been relaxed.

## What the retained failure establishes

Quick acquisition03, session `run-xzolz15c`, remains immutable in SQLite. The
[ring interpretation](rendering-watchdog-results-20261101/retained-ring-01.json)
and [observation review](rendering-watchdog-results-20261101/retained-observations-02.json)
bind its receipt and exact bytes. Confidence is high for the following recorded
facts; addresses and clocks identify this execution only.

The publication worker's private queue was empty. The V3 mapped ring retained
67,896 unread bytes: 68 complete ordered commands after sequence 7,671. The
five-second watchdog recheck found no ACK progress. This was transport inactivity,
not a cursor rollover or an already-drained idle ring. Native World 5 was prepared,
imports/returns remained at four, and the last visible World was generation 3.

The source-only chunk had already been acknowledged. Its 16,384 records are not
the unread tail. That tail contains 54 masked updates, eleven copies, two
presentations and one World import, with no CPU_READ. All consumed commands had
reached a paint observation. A CPU history refusal for generation 5 was also
retained, but its ordering relative to the watchdog is not timestamped.

Paint559 precedes swap559 by about 3.645 seconds; paint563 precedes its swap by
about 3 seconds. Intermediate paints repeat command count 7,671. These establish
host observation gaps, without identifying compression, file writes, GL/compositor
work, resizing, input/discovery callbacks or OS descheduling as their cause.

The [media snapshot review](rendering-watchdog-results-20261101/retained-media-deadline-01.json)
also establishes producer cancellation of Intro0 request2 before World. The
exact admitted media source has one write site for that field, reached on a
host/acceptance/playback deadline. Qt movie skip alone does not write it. The old
snapshot cannot distinguish the branch or timestamp; a user skip is not inferred.

## Controlled discrimination

The [fault review](rendering-watchdog-results-20261101/fault-discrimination-01.json)
keeps these experiments distinct:

| Experiment | Observation | What it establishes |
| --- | --- | --- |
| Intro fault09 | One 6.2-second GUI pause, unchanged media heartbeat, producer cancellation; then 62 independently matching World swaps, 41 reference Worlds, 61,508 original draws and normal Quit. | Media deadline expiry alone was insufficient to cause the later World stall in this run. Its exact earlier source phase remains recorded. |
| World fault10 | Pause after the first World paint; unchanged ACK, 68,196 unread bytes and a 5,004 ms command watchdog; pending World3/imports2 and a terminal native refusal. | Losing GUI command-consumption time can produce the same unread-transport failure mechanism. It does not identify the old callback/OS cause. |

Fault10 used skipped intros to isolate consumption. It ended with original
producer exit95 and no retained CPU history refusal; the historical execution's
exit/refusal ordering is not reproduced in full. Similar terminal ring bytes do
not establish an identical causal sequence. These injected runs grant no nominal
acceptance or general fault/restart requirement.

Faults require both the normal acceptance harness and an explicit diagnostic
flag. The collector assigns a distinct diagnostic acquisition kind; nominal
admission rejects diagnostic acquisition/comparison kinds and fault configuration.
Fault09's comparison retains its older nominal comparison label, but its diagnostic
acquisition already prevents nominal admission. Future diagnostic comparisons
also receive a distinct label. Historical report bytes are never rewritten.

## Established tooling repair

Trace08 revealed that the collector read compiler dependencies **before** the
public launcher rebuilt the player. A newly linked header was absent from the
receipt. The earlier native admission binds DLLs, not the whole Qt executable;
that absence cannot be waived by the native DLL admission or matching pixels.
The [limitation receipt](rendering-watchdog-results-20261101/trace08-build-binding-limitation-01.json)
preserves the successful comparison and its incomplete Qt source boundary.

The collector now builds first, records the actual linked compiler closure,
retains the exact player executable, and checks the dependency set and executable
again after execution. The comparator rejects omitted linked dependencies and
verifies the retained executable. A real compiler regression adds an include
between dependency discovery and rebuild, demonstrates rejection of the old
receipt, and accepts the rebuilt closure.

The [control report](rendering-watchdog-results-20261101/controls-01.json) records
24 Python methods and six Qt presentation/input executions at DPI1/1.5. A separate
[five-case reader control](rendering-watchdog-results-20261101/inspector-controls-01.json)
checks an unread World across ring wrap and rejects truncation, sequence gaps and
changed bytes. Complete negative captures can be analyzed without pretending the
producer emitted a successful END.

Slow host scopes and complete GUI callback timing are opt-in at 50 ms, capped at
256 rows per class. Receiver labels are copied before callbacks may destroy their
objects. Instrumentation does not renew leases. Capped output is an observation
boundary, not proof that all later callbacks stayed below the threshold.

## Final current execution checkpoint

The final default enabled-intro Quick journey completed with **55 World swaps**,
30 independent reference Worlds and 52,234 original draws. Its complete compared
viewport/history cohort contains 107,869,460 pixels, with zero mismatches and no
remaining surfaces; the command ring drained fully and original Quit retired the
session. All three new probe headers occur in its as-built source map.

The final current intro-fault interface also completed with 55 matching World
swaps, 33 reference Worlds and 63,836 original draws after the explicit media
pause/cancellation. Its comparison has the diagnostic kind, and the real nominal
admission function rejects that pair. The [diagnostic join](rendering-watchdog-results-20261101/joined-01.json)
binds both executions, the expected World failure and controls across 757 current
source paths. It preserves the older intro-fault09 result at its historical phase.

An additional [cache diagnostic](rendering-watchdog-results-20261101/cold-driver-review-13.json)
used enabled intros, no injected pause, a fresh XDG cache directory and
`MESA_SHADER_CACHE_DISABLE=true`. The actual Qt process environment and executable
were verified. It completed World/normal Quit with a fully drained ring and no
watchdog. The largest callback overlapping the captured World interval was
275.122 ms; the timing classes did not exhaust their caps.
This did not expose the old stall. Internal cache branches were not independently
instrumented, and no independent cold-cache pixel comparison or acceptance claim
is made. The historical cause remains open.

## Shortest remaining route

1. Use the current instrumented, as-built collector on the enabled-intro default
   journey. Preserve any unforced failure and inspect its complete callback and
   nested host timings, command ACK/publication, World state, media state and
   CPU refusal bytes together. Do not accumulate blind passing repetitions.
2. Identify the actual long callback or external stall. Where the trace isolates
   a host stage, measure that stage's suboperations or use a bounded blocked-stage
   control before changing its scheduling/ownership. If it cannot identify a
   stage, retain that limitation rather than naming a speculative root cause.
3. Repair the established dependency, preserve strict deadlines, and demonstrate
   the affected controlled case plus the full-intro route and independent World
   comparison. A generic six-second GUI freeze is expected to fail safely; making
   that arbitrary freeze pass is not itself the product requirement.
4. Review the historical failure's exact applicability before retiring it. Then
   resume the remaining EP04 action/HUD/camera proofs and EP05 progression,
   extracting operation evidence before scheduling residual EP06 work. Software,
   fault/restart, the complete join and both final decisions remain separate.

The [current diagnostic join](rendering-watchdog-results-20261101/joined-01.json)
binds fresh execution and controls to this narrow diagnostic contract. It grants
no additional acceptance leaf or original-recovery verdict. The earlier six
proved leaves retain their original source phase; this batch does not renew the
entire high/low storage proof domain after shared source changes.

## Execution and storage boundary

The source worktree and isolated installations/prefixes are on the second NVMe;
only builds and compiler staging use RAM. A `/tmp` execution was refused by the
unchanged output-isolation policy. Setup/build refusals are retained separately
from engine failures. The task uses an explicit 100 GiB cold-drive policy and the
outer unchanged 90 GiB main-drive guard. Default launcher policy remains unchanged.
Generated reports, raw captures, failed attempts and unique source versions are
retained through SQLite. Original manifests are verified before and after every
original-consuming acquisition/comparison. No original file or gameplay balance
is modified, and no evidence is published.

## Focused capture repair — follow-up batch

This follow-up starts at `65095aee6` in the isolated
`working/worktrees/startup-watchdog-repair-20261101` checkout. The original
`RN31.G.first-world-watchdog` remains unresolved. A new current startup pause
was observed and a blocking capture stage was isolated and repaired; that does
not establish the callback or OS cause of the old five-second failure.

The traced baseline completed both movies and 55 matching World swaps against
33 independent reference Worlds / 74,088 original draws. The trace-disabled
baseline also completed both movies and normal Quit, with 56 matching World
swaps against 32 reference Worlds / 57,793 original draws. Its command capture
retains a 2.076-second ACK observation gap. Both sibling debugger attaches were
denied by the host ptrace policy; no successful stack sample is inferred from
those attempts.

A separate parent-GDB diagnostic, with movies skipped to isolate startup work,
measured another pending-ACK pause and sampled the main thread in
`deflate → qCompress → AcceptanceCapture::append → capturePaint → paintGL`.
This identifies an active blocking stage on the live command caller with high
confidence. It does not measure the duration of every individual compression
call or identify every contributor to either complete observation gap. Its
launch-path wrapper and exact unchanged ELF are retained separately; the generic
collector's wrapper hash is explicitly insufficient as a Qt player identity.
That diagnostic has no nominal admission credit. The exact player was restored
before the repaired build.

`AcceptanceCapture` now deep-copies and queues snapshots with their caller host
stamp and row identity. One worker owns compression and all stream-file writes.
FIFO order, the wire envelope and all raw/stored limits remain strict. A 256 MiB /
256-job limit includes the in-flight job; exhaustion returns without blocking the
caller and closes an incomplete proof. A terminal row is reserved for refusal.
Only terminal `finish`/destruction joins the writer. GPU readback, bounded copying,
arbitrary GUI freezes and general shutdown scheduling remain separate.

The regression interposes the actual compression entrypoint and holds it while
the caller must return and reuse its input storage. The exact synchronous
preimage fails that progress requirement; the worker passes. Optimized and
ASan/UBSan executions also verify deep ownership, FIFO counters, bounded overflow,
compression failure and first-refusal preservation. The actual production reader
accepts the complete stream and rejects all three failure streams in each build.
No producer lease, watchdog deadline or native ownership guard is relaxed.

The [joined follow-up receipt](rendering-watchdog-results-20261101/startup-repair-joined-02.json)
binds the failed setup attempts, both baselines, denied attaches, parent-debugger
capture, exact preimage, regression and repaired execution. `NP.acceptance-capture-worker`
is the narrow native policy for this change. Historical bytes, source hashes,
acceptance leaves and independent recovery dimensions are preserved.

The repaired default full-intro run completed with **102 independently matching
World swaps**, 78 reference Worlds, 137,118 original draws and 153,694,060 compared
viewport/history pixels, with zero mismatches. Intro0 and Intro1 both completed
(1,777 and 1,004 presented video frames); there was no producer media cancellation.
The command ring drained and original Quit released all graphics owners. The
1.5-second pending-ACK sampler did not trigger. None of the 37 retained slow host
operations was a caller-side capture append; the timing cap was not reached.
Worker compression reached 93.340 ms and stream writes 668.604 ms in this run.
These are bounded observations, not a general latency or frame-rate guarantee.

Reproduce the capture regression in a fresh owned output directory:

```sh
python3 tools/working-retention.py run --reserve-gib 1 -- \
  python3 tools/test-acceptance-capture.py \
    --output working/tests/acceptance-capture-new \
    --report research/runtime/rendering-watchdog-results-20261101/capture-new.json
```

For the retained old-stage control, restore the two `startup-repair-before-*.hpp.bin`
preimages through the SQLite reader into a new directory under `working/`, then
pass its `acceptance_capture.hpp` with `--baseline-header`. The tool retains the
expected old failure without granting current implementation credit. The regular
`run-rendering-acceptance-pilot.py --scenario normal-quick` and independent
`check-rendering-acceptance-pilot.py` commands remain the live verification path;
use fresh report identities and the admitted isolated installation/prefix inputs.

The first join retained valid execution assertions but used the wrong scope-claim
container type. The failed audit and original join bytes remain retained; the
new `RW02.worker02` / `startup-repair-joined-02.json` registration fixes that
accounting schema only and does not refresh execution hashes. Filesystem-fault
and row/stored-limit exhaustion are not directly injected by the fresh controls.

## Parent-debugger sampling follow-up — 2026-11-01

The opt-in `--sample-stalls` normal-quick collector launches the actual unchanged
Qt ELF as a GDB child. No executable wrapper replaces the recorded build output.
It records the player and debugger hashes, arguments, command-file hash, stable
V3 observations and any one-shot sample in bounded execution interchange, then
imports those bytes with the acquisition into SQLite. The collector always gives
this route a diagnostic acquisition kind; its comparison remains diagnostic too.
These runs cannot admit nominal acceptance leaves.

The sampler resolves a single generation through the public owner's admission
journal. It validates the V3 header, unsigned ring distance, session and file
identity, and requests one all-thread stack after 1.5 seconds of outstanding
bytes without ACK progress. Idle, actual ACK progress and ambiguous reads restart
the interval. READY, terminal and cancelled states do not trigger a sample;
multiple generations, identity changes or malformed evidence disable observation.
Only the separate debugger process receives the interrupt. The sampler opens
the ring read-only and never acknowledges bytes or changes producer deadlines.
Debugger interruption and polling can perturb scheduling, so this is observation
within a declared diagnostic environment, not a transparent nominal execution.

[Sampler controls](rendering-watchdog-results-20261101/sampler-controls-01.json)
pass ten methods covering idle/progress/publication/wrap, terminal states,
malformed/session/file identities, partial admission, nominal-launch refusal and
real parent-GDB children. The real controls prove one stack, continuation, normal
and nonzero exits, fatal-signal handling, and observation-error refusal. Separate
[admission/build/ring controls](rendering-watchdog-results-20261101/sampler-admission-controls-01.json)
also pass. These are isolated controls, not game or original-equivalence proof.

World fault02 sampled 33,068 outstanding bytes at ACK 45,364,042 after 1.546 seconds.
The GUI stack contains `WatchdogHostFault::schedule` and `nanosleep`. The actual
producer recheck found the same ACK and refused at 5,000 ms, leaving the ring in
FAILED/INTERRUPTED. The host recorded its planned 6.2-second pause. This confirms
pre-timeout sampling of that controlled mechanism while retaining the guard.
It does not reproduce the historical callback/OS cause or terminal CPU-refusal
ordering. The actual Qt ELF remains
`56b077654613f15916ff773ba2ecf4243b8d54cc45a3212903b3ff5fb4e9815c`,
byte-identical to the repaired worker run.

The first relative launch was rejected before execution by the main checkout's
older parser ([preparation refusal](rendering-watchdog-results-20261101/sampler-launch-refusal-01.json)).
World fault01 then stopped during staging because the isolated extracted music
catalog was absent, with zero ring observations. Its negative acquisition remains
immutable; a [hash-verified catalog copy](rendering-watchdog-results-20261101/sampler-media-setup-01.json)
supplied the missing input before fault02. Neither preparation failure identifies
an engine stall. Original media passed the before/after manifest checks.

The final [joined proof](rendering-watchdog-results-20261101/sampler-joined-04.json)
binds 760 current owned-source fingerprints and only the two narrow native
policies below. RN31.G.first-world-watchdog remains unresolved.

### Current full-intro result and a focused driver repair

Full-intro02 completed normal Quit, but its unchanged strict comparator rejected
the original-input proof: only the second Right down/release pair was retained.
The first Qt publication was subsequently released without a poll receipt. The
old driver scheduled press at 250ms and release at 700ms; elapsed time alone did
not establish observed consumption. Exact old poll timing and transient observer
contention are not recorded, so neither is asserted as the historical cause.

The acceptance-only driver now holds one Qt-owned press until the existing
MNMIAK01 observer records its original owned poll, releases focus, and awaits
the corresponding release before continuing display/Results actions. Each phase
has a 3s admission deadline, without retry or producer-lease renewal. The reader
refuses malformed/stale/partial/full journals and old World rows. Bounded-size
receipt reads do not preempt arbitrary OS I/O. Ordinary manual gameplay does not
install this test driver. The original input hook and strict comparator are
unchanged.

[Optimized/sanitized controls](rendering-watchdog-results-20261101/sampler-input-controls-02.json)
keep the semantic key held through a 750ms delayed peer, past the old nominal 450ms
window, observe release, and refuse a missing peer after the deadline. Two hundred
Qt event-loop ticks continue during each held control. Standalone build01's Qt/GCC
include-order failure and exact source preimages remain retained; it is not an
engine failure.

Fresh World fault03 uses the rebuilt ELF and samples 31,558 unread bytes at
ACK 45,419,132 after 1.541s. Its actual producer recheck sees the same ACK at 5,000ms
and refuses. The original guard sources match repaird0bf7a551 exactly.

Full-intro03 uses actual player ELF
`e779e87fe92059e02ff52a8134b90d0075b339d178cee3b7ca71acbdaea78a57`.
Intro0/Intro1 complete with 1,777/1,004 decoded and presented video frames, no
producer cancellation, four actual original input transitions, two World visits,
a drained ring, normal original Quit and zero owner retirement. Its sampler
makes 3,822 stable observations, with a 464ms peak pending interval and no stack
request. The unchanged [independent comparator](rendering-normal-results-20261031/sampler-full-intro-comparison-03.json)
accepts 92 World swaps against 64 continuously reconstructed original/history
Worlds and 106,363 original draws: 147,273,180 compared pixels, zero mismatches.
The original executor reads neither native player pixels nor original destination
captures as its inputs. Movie/audio content, unleased movie planes and physical
scanout remain separate from the World/viewport history comparison.

Both player acquisitions and comparisons remain explicitly diagnostic. No nominal
requirement, complete camera/input recovery or supported-player verdict is granted.
Shared wrapper/driver edits leave older complete-source acceptance domains pending;
the new evidence binds only the sampler and receipt-driven acceptance policy.

### Distinct negative evidence retained

Traced full-intro01 reached Main action 1 but no World. The original Main callback 2
entered and returned; its terminal channel has ACK 1 after client failure, which
does not order engine/client retirement. Final Intro1 status is COMPLETE with no
producer cancellation. V3 pending peaked at 52ms, so the command sampler correctly
made no request. The Main transition/lease cause remains unresolved as
RW03.G.intro-main-transition. A later finite pass does not close it.

The optional read-only menu observer for intro02 could not checkpoint its
uncommitted in-memory SQLite transaction. A disposable control reproduces the
backup wait and succeeds after commit. The owned failed helper was stopped;
physical output and exact producer are retained in the
[checkpoint failure](rendering-watchdog-results-20261101/sampler-menu-observer-02-failure.json).
Its uncheckpointed timeline is unavailable and supports no causal claim. Core
acquisition and input receipts are independent of that helper; intro03 uses no
auxiliary observer. Failed join02/03 and their exact producers also remain
immutable; join04 resolves the negative flow artifact and exact menu-channel
identity without changing any engine result.

The [committed-history review](rendering-watchdog-results-20261101/sampler-history-review-01.json)
checks 4fa166010..a4d3a6ee3, including all 1,790 repair and 1,792 merge file transitions,
and verifies 754 distinct SQLite payloads. Retrospective provenance receipts cover
previously unlisted binary artifacts and committed journal versions; they assert
neither a past gate pass nor fresh engine validation. Current uncommitted work is
accounted separately.

## Watchdog follow-up integrated with cursor diagnostics — 2026-11-01

[RW03.integration01](rendering-watchdog-results-20261101/sampler-integration-joined-02.json)
binds the actual combined source, including the independently added cursor
provenance tooling. The explicit drawing-only cursor exercise retains its timed
input/cursor schedule and gains no receipt-wait input admission. The normal
acceptance route keeps the new observed-press/release wait; the current strict
comparator accepts all four original transitions. Neither existing producer guard
nor the original input hook changes.

The fresh fault samples stable pending bytes after 1.542s, before the unchanged
5013ms refusal. The combined full-intro run completes both movies, two World visits
and normal original Quit with a drained ring and zero owners. Its 89 swaps match
the independent original/history reconstruction across 144967420 pixels with zero
mismatches. This is current scoped diagnostic evidence; movie/audio content,
physical scanout, nominal player requirements, the historical five-second
RN31.G.first-world-watchdog and separate RW03.G.intro-main-transition remain
outside the claim. Both unresolved failures retain their original evidence.

The later concurrent oracle correction `e5fc9a0eb` is excluded from this bounded
08f813e1d/2c22e2156 integration snapshot. Its changed paths overlap none of the727
retained native engine/capture sources. The present comparison preserves its
original oracle fingerprint; a separate integration must rebuild the corrected
original history over the same immutable capture before claiming current-master
comparison. See [the review](rendering-watchdog-results-20261101/sampler-concurrent-oracle-review-01.json).
