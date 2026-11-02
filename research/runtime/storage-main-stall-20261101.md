# Storage-to-Main command stall — 2026-11-01

This owned batch starts at `c1afab709`. EP06 storage01 reached Preferences Cancel,
Save and Mini Load, then cancelled returning toward Main. The retained ring has
1,010,864 unread bytes, including CPU_READ, and the post-click grab took 5,023 ms.
The grab is an unconfirmed lead. Observation overhead and command/CPU-read
service remain competing explanations; earlier watchdog/Main failures stay open.

The targeted diagnostic preserves enabled intros, storage actions, all-COPY
observation, deadlines and ownership. The existing one-shot parent sampler arms
only after this fresh run's stage 8 Mini Load action screenshot completes. That
witness does not prove original Load completion. A diagnostic acquisition cannot
gain nominal acceptance. Original manifest checks and private fixtures remain
mandatory. This was the initial hypothesis, before the observations below.

## Observed failures and bounded repair

`storage-main-sampled01.json` failed earlier, at Preferences Cancel. A source
acquisition took 4,976 ms while the acceptance worker's file write took 4,979 ms.
The storage arm had not fired. This rules out a return-to-Main screenshot as a
necessary trigger for that failure, but does not establish an I/O cause. Archive
verification was also running, so this execution cannot isolate storage load.

`storage-any-sampled02.json` used the same one-shot sampler from startup. At a
1.5368 s pending acknowledgement it captured the actual GUI thread in
`SHA256_Update -> QCryptographicHash::hash -> CommandCopyDiagnostics::after ->
CommandConsumer::execute -> LiveCommandRenderer::poll -> QTimer`. The exact
GDB-owned inferior was runnable at the host snapshot, with no observed memory
pressure. This confirms synchronous diagnostic COPY hashing in the command
service path; one sample does not explain every historical five-second stall.
This diagnostic completed Save, Mini Load, both rebuilds and Main Load, then
failed a separate final Mini Quit menu-adapter deadline. That failure remains
unresolved and is not reclassified as a rendering mismatch.

The repair keeps GL readbacks on their owner and transfers owned source,
before/actual images, geometry, sequence and ancestry to one FIFO worker. CPU
expectation, full comparison, hashing and witness writes run there. Reservations
include in-flight work and expected-image memory, bounded to512 MiB and256 jobs.
Overflow marks evidence truncated; sink errors survive terminal finalization.
END does not join before the command acknowledgement. Terminal `finish()` joins
before reporting. Expected pixels never enter rendering, source history or CPU
read replies. Original deadlines, leases, actions and gameplay are unchanged.

`worker-controls02.json` retains optimized and ASan/UBSan FIFO, blocked-sink,
bound/error controls, plus actual OpenGL opaque and CPU-keyed full-output tests,
witness lifetime, overwrite refusal and deliberately injected mismatches. These
are diagnostic-worker controls, not original-game equivalence or acceptance.

## Nominal validation and acceptance boundary

The first fresh enabled-intro, unsampled default/high execution,
`storage-worker-default-high03`, completed Save, Mini Load, two display rebuilds,
Main Load and normal Quit. It checked 7,068 COPY operations with no mismatches or
truncation. Its worker reservation peak was 450,428,032 bytes; 52,440.95 ms of aggregate hashing
ran on the worker. The complete format 3 archive held 769 Worlds in 961,986,478
bytes. The immutable acquisition is retained, but its independent comparison
was rejected: the commands stream ended with `Acceptance capture queue budget
exceeded`. Its viewport stream was complete (3,522,486,000 decoded bytes); the
commands stream retained only a contiguous prefix. This is a recorder failure,
separate from the completed native journey. It earns no nominal acceptance.

The failure exposed a burst of tiny command fragments, where the uniform 256-job
queue count could refuse far below its256 MiB byte bound. Only the commands stream
now permits 4,096 pending jobs. Images/exports retain256 jobs, bytes remain256 MiB,
and raw/total-row/stored budgets, FIFO bytes and overflow refusal are unchanged.
The corrected owned-worktree `capture-burst-controls02.json` retains optimized and ASan/UBSan controls with a
blocked real compressor: 1,024 fragments plus a CPU-read boundary retain exact order,
5,000 fragments refuse at the finite count bound, image-byte exhaustion still
refuses, and complete/refused legacy and campaign streams pass the strict reader.

`capture-burst-controls01.json` is a retained wrong-root attempt: relative
commands through the main storage guard used main-checkout sources instead of
the owned worktree. It does not validate the new count allowance. The main build
was stopped, its newly created reference files were exact-verified and moved to
this owned checkout, and its generated working output remains pinned. Corrected
build/control commands use absolute source, binary and output paths. No native
game was launched by that mistaken attempt.

The separately committed campaign work (`d84bb0594`, with its preparation
ancestors) changes shared capture/oracle sources. The remaining nominal matrix
therefore uses the combined source phase, including the corrected original
distortion oracle already on master, instead of collecting further superseded
captures. The three accounting conflicts retain both exact journals/registers
and all historical master startup ledger sections alongside the new campaign
sections. The rejected earlier comparison and source preimages remain immutable.

The storage acceptance join treats default and explicit software configurations
separately. Each needs actual 640x480 and 800x600 World comparisons. Only four
finite Save/Load/Cancel leaves per configuration are eligible: at most 8 of 156
requirements. Other journeys, operation scope, gates and both final decisions
remain pending. Finite successful runs do not retire earlier stalls or the
separate menu timeout. The existing default-only normal acceptance API is
unchanged; software credit requires its explicit flag and observed provider.

## Reviewed combined-source result

All four enabled-intro, unsampled nominal storage cases pass complete independent
original/history comparison, changed-draft Cancel, Save/Mini Load/Main Load, both
display rebuilds, normal original Quit and graphics retirement. Explicit software
is separately flagged and its actual Mesa provider is checked; the default target
on this host also uses llvmpipe. Inputs and actual World sizes are proved per run.

| Configuration | Actual World extent | World swaps | Compared pixels | COPY checked/seen |
| --- | --- | ---: | ---: | ---: |
| Default | 800x600 | 916 | 863131500 | 6915/6915 |
| Default | 640x480 | 1003 | 937464000 | 7479/7479 |
| Explicit software | 640x480 | 1202 | 1113238500 | 8192/8805 (capped) |
| Explicit software | 800x600 | 861 | 813285000 | 6518/6518 |

Across these finite cases, 3,982 World swaps / 3,727,119,000 pixels match, and
29,104 selected COPY destinations match. The remaining 613 COPY destinations
lack individual diagnostics after the declared count cap. Their original
`success: false` / `truncated: true` diagnostic is retained. Complete primary
viewport/history comparisons cover every captured presentation; unpresented or
omitted COPY outputs remain unproved. These finite cases provide no performance
benchmark or arbitrary-duration/provider guarantee. See [the scoped storage join](storage-main-stall-results-20261101/progress01.json)
and [the exact integration claims](storage-main-stall-results-20261101/joined01.json).
Only 8 of 156 selected current-source requirements pass; 148 and both final decisions
remain pending. Earlier successful source phases retain their own immutable results.

The [refreshed EP06 frontier](rendering-ep06-results-20261101/storage-worker-frontier01.json)
reuses these same captures: 22 of 36 operation/configuration rows have checked
scoped facets, while all 36 whole-operation obligations remain pending. Required
scene/caller adequacy, absent natural branches, flags/returns, font/DC and specific
resource lifetimes still need their stated proof. No new game action was added
to obtain an operation count, and no zero census became a scope exclusion.

The [incoming committed-history closeout](storage-main-stall-results-20261101/history-review03.json)
checks 9,901 transitions across the three campaign commits. 9,637 research bin/log
versions needed retrospective exact receipts; three SQLite-recorded receipts now
cover them. The prior unresolved report remains immutable. This accounting does
not assert past gate passage or renew any engine validation.

## Next bounded proof

Start with the capped software-low execution: join its 613 missing diagnostic
commands to original caller, operation and phase in the retained EP06 frontier.
The complete command stream and owned source histories remain available. Assess
whether those inputs support a bounded native replay of late outputs before
requesting another game acquisition. Label any new replay separately from the
actual-run witnesses; the original missing outputs remain missing.

Then choose the smallest remaining caller/output/history/lifetime proof in the
36-row frontier. Required scene adequacy, flags/returns, font/DC ownership and
unknown branches remain technical work. No new scope exclusion is authorized by
a zero count or a successful finite storage journey.
