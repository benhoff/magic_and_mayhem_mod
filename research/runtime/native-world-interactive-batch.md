# Bounded interactive World batching and consumer profiling

The public launcher offers `./tools/run-native-world.py --world-batch`.
Use the original window for Single Player > Quick Battle, map 2 and zero magic
items. The native window consumes owned menu/loading/HUD sources from startup.
The first sixteen World queues use the existing guarded V3 batch transport.
The session ends at queue 16; closing either window earlier cancels the isolated
session. Unsupported sources, destination accesses and incomplete chains refuse.
This is a bounded experiment, not continuous gameplay replacement.

`./tools/run-native-world.py --world-batch --startup-history` runs the automatic
map 2/zero-items fixture. Completion still checks every producer checkpoint.
Post-bypass checkpoints contain native contributions; independent exact-original
intermediate and AX comparisons establish replacement equivalence separately.

`native-producers/live-report.json` contains consumer totals under `profile` and
per-queue timings under `world_frames[].profile`. Units are milliseconds from a
host steady clock. Resource preparation, GPU submission, readback and comparison
are separated. Readback may include deferred GPU work; submission is host time.
Checkpoint diagnostics include GPU mirroring/readback, RGB565 packing, hashing
and disk output. The independent CPU reference remains enabled. Queue wall time
runs from consumption of entry to consumption of return, includes producer/timer
waits and excludes original simulation/pre-entry work. These are Debug diagnostic
measurements, not end-to-end frame latency or a hardware benchmark.

Prospective behavior IDs: `NR.world-interactive-batch-launcher` and
`NR.world-batch-profiling`. Automatic completion, native-window cancellation,
precise original equivalence and manual completed menu journeys are separate
validation boundaries. Automatic completion and profiling passed in two isolated runs; exact original intermediate revalidation and native-window cancellation are tracked separately below.

## Measurements (2026-10-09)

Two automatic runs of the public launcher completed all sixteen guarded batches,
with one World readback and writeback per queue and every observed checkpoint
matching. The runs used the Debug build and `LIBGL_ALWAYS_SOFTWARE=1`; variation
between runs is retained rather than treated as a hardware-independent result.

| Consumer cost | Queue 1, runs A/B (ms) | Median queues 2–16, A/B (ms) |
| --- | ---: | ---: |
| Resource preparation | 1005 / 882 | 349 / 276 |
| GPU submission (host) | 307 / 210 | 298 / 260 |
| Independent CPU composition | 166 / 160 | 175 / 140 |
| GPU readback | 1.27 / 1.26 | 1.27 / 1.30 |
| Batch publication and reply accounting | 43.1 / 42.8 | 43.7 / 42.1 |
| Consumer entry-to-return wall time | 1690 / 1446 | 1044 / 873 |

Checkpoint diagnostics across the entire menu/loading/startup sequence cost
11.47 / 11.08 seconds; CPU composition totals were 6.28 / 6.01 seconds. These
session totals include work outside the World queues. They cannot be divided
by sixteen to infer ordinary gameplay frame cost. Per-queue checkpoint work
was about 11 ms in the warm queue medians.

The next rendering optimization should reuse resource/frame binding work across
queues and investigate GPU submission batches. Readback has already become small
in this measured domain. Separately, repeated diagnostic mirror/readback/capture
work should be moved out of any future ordinary interactive mode, while retaining
a verification mode. Unbounded live rendering and a completed manual menu journey
remain unvalidated.

Raw runs: `working/tests/native-world-batch/run-g3bcneee/report.json` and
`working/tests/native-world-batch/run-hzd4kc9t/report.json`. Both bind the same
180 sources and prospective scopes. The recovered first run completed after the
server interruption; the second confirms that result independently.

## Instrumented native equivalence

Fresh frozen-source replay of run A matched all **29,323** admitted raster
intermediate canvases and low16 AX returns, sixteen final native World canvases
and **1,062** completed checkpoints. All five focused native checks passed.
[Immutable scoped replacement record](native-world-batch-instrumented-replacement-20261009.json)
binds the capture to the independent original comparison. Its133 required batch
sources include all80 compiled dependencies. Raw reports retain the full180-source
launcher declaration; launcher cancellation/reporting policy is validated separately.
The final original-refusal reporting fix changes only the launcher policy module
and its synthetic tests, outside that recovered/native batch source closure.

## Final launcher checks

[Final automatic completion/profiling](native-world-batch-public-complete-20261009.json)
and [real native-window cancellation](native-world-batch-public-cancel-20261009.json)
passed after the launcher-only refusal-reporting fix. Synthetic checks cover
explicit original failure records, access-guard faults, interrupted source writes,
complete/cancel/incomplete/worker-failure dispatch and isolated-process launch.
The final automatic record includes current per-queue timings and source bindings.
Manual completed menu journeys and original-window cancellation remain pending;
closing the native window is the directly exercised cancellation boundary.

### Extended finite route

The guarded experiment can explicitly request32 queues, preserving strict
comparisons and destination protection:

```sh
LP_NUM_THREADS=4 xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-native-world-batch.py --queues 32
```

The public launcher counterpart is `tools/run-native-world.py --world-batch
--startup-history --world-queues 32`. The16-queue default remains supported.
The installed `xvfb-run` defaults to640x480, which cannot host the pinned
800x600 original DirectDraw mode; use the explicit screen above. The32 route
uses bounded immutable source-packet reuse documented in
`native-world-raster-batch.md`; complete original-raster comparison still runs
separately with the newly printed capture report and prospective claims.
Manual32 input, rolling/unbounded sessions and real-time guarded gameplay have
not been validated. Public diagnostic builds now use RelWithDebInfo and record
that build type; the independent frozen comparator still builds Debug and does
not claim frame-rate evidence.
