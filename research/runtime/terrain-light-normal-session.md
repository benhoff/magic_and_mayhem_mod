# Normal-session terrain lighting

The existing finite file-capture experiment is independent of this
normal launch path. `MNM_TERRAIN_LIGHTING=observe` or `native` adds a checked
lighting DLL and one persistent native worker to the normal menu preparation
and game runner. The normal player entry points now select `native` when no
lighting mode is supplied; `MNM_TERRAIN_LIGHTING=original` opts out. Shared
diagnostic preparation retains its `original` default. The original world scheduler,
clock, loader and renderer keep their existing owners. Native graphics selection
and lighting selection are separate. Default selection is intentional launch
policy and does not establish long-duration/arbitrary-map acceptance.

The fixed little-endian `MNMLS001` file has a128-byte header and three8MiB lanes
for source, reply and optional observed result. There are no shared pointers or
C++ object layouts. A fresh64-bit nonce, monotonic sequence, input checksum,
explicit lengths and complete cycle state identify each transaction. One engine
thread owns requests; the host owns replies. Acquire/release phases transfer
ownership. Refusal, timeout, sequence exhaustion or invalid reply closes the
channel permanently for that session. A late reply cannot revive it. The
original updater handles fallback. Unavailable or unsupported source snapshots
remain original and do not consume an admitted sequence.

Normal mode keeps fixed storage and no per-update trace. Optional diagnostics
retain at most32 complete source/reply/result records and64MiB, while the worker
continues past that recording budget. `first32` begins immediately; `reload32`
starts at caller00472201 only after the host selects the named original saved
row and arms capture. Original-active observation precedes current-source native
replacement comparisons. No selection flag or successful worker startup counts
as a replacement or original-equivalence result.

The normal supervisor verifies staged binary and compiler-source identities,
starts the worker before Wine, and stops/reaps it after the admitted original
process wait returns. Lighting requires that DirectPlay-profile process-wait
boundary so early Wine desktop return cannot end the worker while the game is
still active. The child authorizer passes only the checked channel, including
inside the existing private graphics boundary. Outcomes and failures are stored
in SQLite; a selected backend with zero admitted updates makes no live claim.
All binary staging checks the original executable hash and expected updater
entry bytes. Source installations and original artifacts remain read-only.

Recorded-input controls, installed live equivalence, launcher integration and
production rollout remain distinct evidence dimensions. Current execution and
remaining limitations belong in the central register and coverage ledger.

A diagnostic mailbox is available only after CLOSED. It requests one original
fallback input/result witness, without reviving the worker or applying a reply.
This permits independent original-code comparison after deliberate worker loss.
The host records worker PID/start-tick identity in the session SQLite lifecycle;
this is operational bookkeeping rather than a shared pointer contract.

## Combined strict native Qt acquisition — 2026-10-26

The bounded combined driver `tools/test-terrain-light-qt-session.py` uses the
existing strict private-display owner and public Qt campaign actions. It runs
original-active lighting before native replacement, retains32pointer-free
source/reply/result records, and independently compares them with unchanged
PE32 code and an ASan/UBSan reconstruction. Normal completion additionally
requires native-only outcome checks, original exit0 and worker reaping.
The Save/Load journey attempts same-session named Save/Load followed by two
original resolution changes and Main Load to reconstruct the World renderer;
this does not establish a different campaign-map transition.

The first actual Save attempt failed after62original-active lighting updates
with zero lighting refusals/mismatches. Submitting native Save caused strict
glyph refusal reason15 and original exit124 before a nonempty save was written.
Reason15 in `glyph_live_enter`, original entry581ec0, groups global canvas/clip/
coverage admission and glyph-frame extent checks; the exact failing subcondition
is unresolved. The negative acquisition retains the32-byte refusal, original
process/Qt outcomes and staged source/binary identities in SQLite. It is not a
lighting equivalence failure or a successful Save/Load/reconstruction milestone.

A separate original-active campaign without Save completed275updates, both
Mini/Cancel and Quit No/resume/Yes, original exit0 and worker reaping. All32
retained source/results matched unchanged PE32 and sanitized reconstruction;
max observed wait22ms and max compute14389us. Longer native replacement and
peer-loss results are recorded separately below. The default remains original.

[The longer native attempt](terrain-light-qt-results-20261026/native-timeout-analysis01.json)
applied1,877updates without a byte mismatch. Request1,878exceeded the100ms
guest budget:106ms observed wait,175466us late worker wall-clock measurement.
One121.827-second physical stress phase with599actions completed; the second
phase and normal Quit were interrupted by the harness after the unexpected
timeout. All32retained native results matched unchanged PE32 and sanitized
replay. A late processed reply did not revive application: processed1,878,
applied1,877,disabled1. These timing fields do not distinguish computation
from host scheduling delay. No performance or longer-session acceptance passes.

[The deliberate worker-loss control](terrain-light-qt-results-20261026/worker-loss01.json)
passes128native updates and32independent comparisons before the checked worker
is killed. It then runs117original fallback updates and exits the original game
normally, reaping the worker. The selected backend and strict launcher report
exit1as expected. This is recovery acceptance and a bounded native prefix;
it does not turn the spontaneous timeout into a successful native session.

[The live Qt executable](terrain-light-qt-results-20261026/live-qt-provenance01.json)
and386linked compiler sources are retained and checked against the successful
observation. The renderer was llvmpipe with four Mesa threads. Original media
checks bracket each original-consuming acquisition/replay. Raw channels,32source
cohorts, failures, staged binaries, saved prefix registries and exact driver
versions are retained in SQLite. The supported executable was restored only in
the isolated fixture after the shared working installation failed source-hash
admission. Installation manifest/prefix and private tmpfs/XKB setup failures are
preserved separately. No engine behavior or lighting default was changed.

At this historical checkpoint, rollout was blocked on the strict Save glyph
refusal and spontaneous lighting deadline overrun. Their fixed-code acquisition
and remaining acceptance boundaries are recorded in the fix batch below. The
old timing fields do not establish the sole cause of the deadline overrun.

TLQT26.abnormal-worker-lifetime additionally records an owned lighting worker
left alive after the failed strict Save acquisition: original exit124, host parent1,
started-only lifecycle and no worker outcome. Closeout verified exact PID/start
ticks/executable and terminated only that orphan. Abrupt Qt/launcher cleanup
remains unaccepted despite normal-Quit and deliberate worker-loss success.

## Fix batch — 2026-10-26

The persistent worker now calls the one-shot replay's checked byte decoder
directly. The two reusable memfds and per-update file streams are removed.
A single derived-kernel cache is keyed by width, height, layers, ambient, ramp
and inherited single-cell value. A key change replaces the cached model. Every
request still restores all five mutable buffers and both cycle states before
stepping; no original process pointers or mutable producer state are cached.
The100ms guest deadline, sequence/nonce/checksum checks and permanent CLOSED
fallback latch are unchanged. Eliminating repeated kernel construction reduces
worker cost; historical timing alone cannot separate computation from host
scheduling and does not prove that this was the sole cause of the old overrun.

The supervisor installs scoped SIGTERM/SIGINT handlers so cancellation unwinds
through stop/reap and SQLite outcome recording. Cleanup ignores repeat signals
until it has reaped the child and restores prior handlers afterward. The native
worker additionally sets Linux PR_SET_PDEATHSIG(SIGKILL) and checks its expected
parent immediately afterward, covering both parent death before setup and
SIGKILL after setup. Abrupt supervisor SIGKILL can prevent its final SQLite
outcome; absence of that row is not a successful session. The worker cannot
survive that parent. Both cancellation paths have actual-process regression
controls, including adoption/reaping by a controlled subreaper.


The repaired combined acquisition passes original-active Save/Load264updates and
native Save/Load240updates. Both include two original resolution changes, Main
Load with a new World renderer, normal original exit0, strict native-only
outcomes,32unchanged-original/sanitized lighting comparisons and worker reaping.
The sustained native run completes both120-second physical phases, MiniCancel
and QuitNo/resume/Yes:2,434applied updates, zero fallback/refusal/mismatch,
max17mswait and5252usworker compute. The expected worker-loss run applies
128native updates, then continues
190original fallback updates with the latch
closed and normal original exit0; the selected backend correctly reports1.
All32retained native-prefix comparisons pass. The running Qt ELF and387compiler
sources are retained and checked against the successful observation.

Twelve actual worker controls include supervisorSIGTERM cleanup with a failed
SQLite outcome and supervisorSIGKILL with kernel worker termination/reaping.
The reproduced pre-fix Save failure also leaves no surviving worker under the
new parent-death guard. Its killed supervisor has no final outcome; that negative
acquisition remains a failure. Strict unsupported-glyph exit124controls still
pass. All new raw evidence, failures and reports use SQLite. These results close
the three observed blockers within their bounded controls; different maps,
arbitrary saves, hour-long execution, hardware/FPS and default rollout remain
separate acceptance. The normal lighting default remains original.


## Player default policy — 2026-10-26

The public player launchers now select native lighting when no mode is supplied.
Explicit original/observe/native modes retain precedence, and capture remains
none unless requested. Lighting options are validated before prefix preparation
and reach both preparation and the owned runner. Shared diagnostic preparation
still defaults to original. The 100 ms deadline, original fallback latch and
nonzero selected-session failure reporting remain unchanged.

`TL.player-lighting-default` records this intentional launch policy separately
from recovered lighting behavior. `TLDF26.player-controls` passes eighteen
current-source launcher controls, including actual Bash routing and original
worker bypass, with Wine/prefix/owner operations mocked. No new live-game
acceptance or engine status promotion follows from changing the default.
Combined-master map/save journeys, hour-long stability and hardware performance
remain pending; earlier execution evidence keeps its original hashes.
