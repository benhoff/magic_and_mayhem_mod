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
