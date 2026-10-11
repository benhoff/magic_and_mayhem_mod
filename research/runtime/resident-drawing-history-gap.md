# Resident drawing-history contention investigation

2026-10-11. This investigates native scheduling policy, not recovered driver
ordering or original pixel equivalence. Historical reports and hashes remain
unchanged. The visible command refusal `reason 2` means GAP, independently of
the producer's local `session_gap` subreason.

## Confirmed failure chain

The retained stage-profile capture failures `run-5cef8x_c` and `run-u4m1wj53`
record an ordered copy timeout against `bridge.c:307`, the Unlock wrapper in
their declared source version. The observed holder elapsed times are559ms and
604/669ms; the native admission budget is50ms. Timeout forwards the original
copy without a captured transaction, increments `game_lock_epoch`, and marks
target uncertainty before/after that original. `game_session_sync()` then emits
local `session_gap(1)` and command-channel GAP2. This is intentional conservative
refusal after incomplete history, not a GPU pixel mismatch or archive exhaustion.
Clock resolution and separate atomic diagnostic reads limit exact timing claims.

Fresh phase instrumentation narrows Unlock ownership to snapshot preparation
(20..24), successful publication/cleanup (30..33), resident UI delta scan/packing
(40..43), and tracker release (44..48). Only the owning thread may change its
phase; external World binding must not overwrite another callback's marker.
Phase33 precedes final snapshot cleanup and tracker release. Phase45 covers the
bounded diagnostic call; phase47 covers inline command-channel pumping after
tracker release. All markers are local diagnostic integers, with no wire change.
The50ms ordering deadline, original results/LastError, byte/ownership budgets and
conservative refusal remain intact in this diagnostic milestone.

## Fresh negative observations

* [Combat/retreat negative](resident-history-combat-retreat-negative-20261011.json):
  2,880 balanced completed/imported/original-returned Worlds and no recorded
  copy timeout/GAP; failed the post-combat retreat waypoint. This is not a
  completed campaign or a GAP fix.
* [Unlock cleanup GAP](resident-history-unlock-cleanup-gap-negative-20261011.json):
  longer camera/menu capture fails at576 balanced Worlds. Unlock owner phase33
  is recorded with325ms holder age and306ms waiter age, then local epoch GAP1.
  Cleanup, diagnostics and inline pumping are not yet distinguished by this
  result; recording phase33 does not prove a heap or mapped-I/O cause.
* [World bind tracker refusal](resident-history-bind-tracker-refusal-negative-20261011.json):
  expanded markers reach651 balanced Worlds then the resident producer cancels.
  `RenderResidentWorldV1` requests the tracker while another thread holds
  `game_blit_after`; the8ms tracker budget expires at observed16ms. The pre-repair bind
  export skips the ordered-copy admission domain used by callbacks.
  This is a separate confirmed admission failure, not a copy-timeout GAP.

All three runs use private NVIDIA/Zink TITAN rendering, retain failed journeys,
stop their owned Wine sessions and verify immutable originals afterwards. They
do not claim uninstrumented cadence, whole-frame comparison or completed Quit.

## Next bounded changes

Serialize public resident binding with the existing callback/copy admission
domain before acquiring the tracker. Prove that a short admitted callback hold
longer than8ms lets binding succeed without losing history, while same-thread,
timeout, shutdown and invalid-input guards retain explicit refusal.

Investigate inline transport publication inside ordered callbacks. The existing
publication worker already owns retry/drain capability; a source-backed
regression should distinguish drawing callbacks from worker pumping and preserve
FIFO bytes, full frames and shutdown ACKs. Phase33 alone does not identify the
slow suboperation. Mapped page stalls, heap stalls, diagnostics, arbitrary owner
preemption and other tracker users remain explicit hypotheses/boundaries.

## Resident bind admission repair

The exported World bind now joins the existing callback gate and ordered-copy
admission before tracker access. Original drawing callbacks retain their50ms
deadline and conservative timeout invalidation. Binding has no original pixel
mutation to forward: its copy-admission timeout refuses without fabricating a
missed drawing operation; same-thread copy reentry also refuses. Callback gate
accounting protects session storage during stop/recovery. These branches are
reviewed policy, with the broader refusal matrix still pending execution.

The actual PE32 [baseline negative](resident-bind-short-hold-baseline-negative-20261011.json)
uses an independent thread holding real copy/tracker ownership, with release
after20ms. The old bind refuses at16.2411ms and publishes no World. The same
fixture [after repair](resident-bind-short-hold-fixed-20261011.json) binds at27.1321ms
and publishes exactly one generation/nonce World. Both retain one original Lock
and Unlock with unchanged results/LastError, identical FIFO/archive bytes, a
private snapshot before borrowed pixels are poisoned, and acknowledged END.
This proves the narrow native admission policy; no game executable, driver
ordering, GPU execution or live cadence is exercised by this control.

Committed-history receipt review for `d018309..650df0b` matched all14 changed
files to exact parent/current hashes (journal self-excluded), with zero unresolved
receipts. The report is retained at
`working/tests/resident-bind-history-range-20261011.json`; it asserts neither a
retrospective gate pass nor new behavior validation.

## Confirmed inline publication ownership stall

The [post-bind-repair TITAN negative](resident-history-inline-pump-gap-negative-20261011.json)
completes/imports/returns1,309 balanced Worlds then records Unlock owner at
`bridge.c:311`, phase47 (`0x2f`), with1030ms holder and waiter ages. The original
cursor operation falls through once, followed by local epoch GAP1. Phase47 spans
`command_channel_pump()` and LastError restoration after tracker release; the
outer ordered-copy lease is still held. The mapped-ring pump is now the
confirmed stalled region. The trace does not distinguish page fault/I/O latency
from preemption inside that region. This failed run stopped its private Wine
session and verified source stability and immutable originals. Frame-stream
updates and completed Worlds are different counters; neither establishes a
completed stress/Quit journey.

The bounded repair delegates tracker-release pumping to an already-created
publication worker, preserving the synchronous no-worker fallback and explicit
shutdown/recovery pumping. The owned32MiB queue and existing worker retain FIFO,
1MiB drain budget, no-ACK deadlines and conservative overload refusal; no
drawing callback waits for transport publication. It does not guarantee that
other drawing work or arbitrary scheduler pauses fit50ms.

The [controlled inline baseline](resident-publication-inline-baseline-negative-20261011.json)
records two callback pump calls and211.3365ms Unlock under100ms per-call injected
delay. The [worker-isolated control](resident-publication-isolation-fixed-20261011.json)
records zero callback pump calls and9.2083ms observed Unlock; the same short-hold
World bind succeeds. Both verify original results/counts/LastError, private bytes
before source poisoning, byte-identical FIFO/archive and acknowledged END. The
delay and routing probe compile only into the selftest DLL; production adds no
probe/export/wait. These timings are observations, not OS scheduling bounds.

The repaired [copy/Lock/GPU regression](resident-publication-copy-regression-20261011.json)
passes209 independent full-frame comparisons in four complete sessions, with
two deliberately timed-out sessions still producing conservative GAP and
preserving original counts/results/LastError. The repaired
[reader-pressure regression](resident-publication-backpressure-regression-20261011.json)
passes all eight pause, advancing-ACK, idle, stalled-reader, cancellation,
invalid-ACK and overload cases. These are synthetic native checks, independent
of the TITAN public-gameplay completion milestone.

## Repaired long instrumented journey: history clear, benchmark failed

The [retained performance/capture negative](resident-history-repaired-capture-performance-negative-20261011.json)
finishes three120-second gameplay phases, two native menu returns and normal
Quit on TITAN, with7,091 completed/imported/original-returned Worlds. No
copy/tracker timeout or session GAP is logged; no native fallback, readback or
pixel reply is recorded, and resident retirement is clean. Source stability,
private Wine cleanup and immutable-original verification pass.

The overall benchmark remains failed. Cycle0has one3.243-second sample
(16.03native updates/s,3.39paints/s); cycles1/2have no failed samples. The
asynchronous capture queue reaches265,977,768of268,435,456bytes and truncates
after183accepted/written Worlds. Engine World rate is19.1138FPS, which does
not excuse the stall or establish60FPS. Capture overload and scheduling are
remaining performance hypotheses; this result is neither a complete corpus
nor a passing ordinary-gameplay benchmark. A separate run without profiling
or capture will test that boundary. The drawing-history repair is supported
by the native controls and this bounded live continuity evidence.

## Header compatibility and final source control

The strict offline C queue test exposed an unused-function warning for the new
header helper. Declaring it `static inline` preserves header-only compilation.
The [optimized/ASan/UBSan queue build](resident-publication-offline-queue-build-20261011.json)
passes with the corrected header; its report is an unbound build diagnostic,
not a scope-bound original comparison. The fresh
[PE32 final-header control](resident-publication-final-header-control-20261011.json)
passes short-owner binding, zero callback pumps, owned FIFO/archive and shutdown
with current source hashes. Its rebuilt DLL is byte-identical to the previous
isolated control. Old evidence retains its earlier header hash and freshness
limitation. Ordinary unprofiled TITAN stress/menu/Quit validation passes below.

Committed-history review of `650df0b..27ed7f8` matches all24 changed file versions
to exact receipts (journal self-excluded) with no unresolved gaps; retained at
`working/tests/resident-publication-repair-history-range-20261011.json`. This is
source-accounting review, not retrospective validation or a new gate assertion.

## Ordinary public TITAN validation

The [unprofiled, uncaptured public journey](resident-history-repaired-ordinary-titan-20261011.json)
passes the19FPS harness with7,284completed/imported/original-returned Worlds,
three120-second camera/gameplay phases, two native menu returns and normal
Quit. Engine World rate is19.6335FPS. Native update averages are56.82..58.11/s;
paint averages are32.98..33.55/s. The lowest individual paint sample is16.16/s
and the longest sample1.132s, both within the harness's existing transient
limits. This is an average-rate plus bounded-stall benchmark, not a guarantee
that every individual second exceeds19paints.

No copy/tracker timeout or GAP is logged. There is no native fallback, pixel
reply or World/native/RGBA readback, no pending World, and normal Quit retires
the resident producer cleanly. Source hashes, private Wine cleanup and
immutable originals are verified. The previous instrumented failure stays
failed and its truncated capture is not promoted into a complete corpus.
This separate ordinary run supports the repaired public scheduling path;
capture/performance attribution, final-header casting/combat coverage,
complete original/driver comparison and60FPS remain separate work.
