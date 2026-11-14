# Native particle submission cost, 2026-11-13

The retained [4× particle stress case](heavy-render-stress-20261113.md) exceeds
the 50 ms native service budget. Fine opt-in diagnostics on frames 858–860 show
that combining source colours costs only about 0.15–0.17 ms per frame. Upload and
composite submission spans are much larger. These are perturbed CPU/driver wall
spans, not GPU busy time; they identify excessive small submissions rather than
expensive CPU colour arithmetic.

Synchronous CanvasWorld completion now admits 512 draws per private scene turn,
versus 32, and permits up to 512 consecutive additive sources per batch. Completion
already loops through the whole World without yielding, so this changes grouping
rather than simulation or presentation scheduling. Prepared replay keeps its
existing outer draw budget and uses the same additive cap. Generic scenes retain
the default 32-source cap; invalid caps outside 1–512 refuse before GPU allocation.

The existing 128×128 union, 65,536 visited source pixels, caller upload/time/draw
budgets, ordering barriers and serial fallback remain. Exact source masks,
selected palettes, opaque index zero, signed origins, clipping and per-channel
RGB565 saturation are preserved. Two reusable 128×128 GPU source/mask planes and
at most 64 KiB of pixel accumulator remain; source descriptors are now bounded by
512 instead of 32. Isolated recovery stays serial. Original simulation, AI,
clock, balance, wire protocol and guarded callers are unchanged.

On the diagnostic frames, successful particle batches fall from 208–219 to
74–82, and uploaded pixels from 345,586–384,722 to 170,665–188,697. The optional
`additive_profile` records successful batch planning, source combination, upload
and composite submission, plus source/visited/upload counts. It excludes
singleton/fallback work and final GPU drain. The enclosing scene timing covers
total work. Default rendering makes no additional diagnostic clock calls.

Controlled unprofiled Apple M1 comparison uses identical retained case/asset
hashes, frames 790–868, three fresh repeats per executable and 237 samples each.
The baseline is the exact pre-task f0933b964 renderer, including earlier
32-source batching. No game, compiler or other renderer test runs concurrently.

| 4× particle native completion | Before | After |
| --- | ---: | ---: |
| Median | 27.853 ms | 19.225 ms |
| Mean | 27.469 ms | 19.714 ms |
| p95 | 50.054 ms | 33.824 ms |
| Maximum | 69.609 ms | 38.234 ms |
| Samples above 50 ms | 14 | 0 |

The unchanged 1× control improves too: median22.295→17.672ms,
p9537.041→31.627ms and maximum41.179→35.578ms, with zero samples above50ms
before and after over the same range and repeat count.

For4×stress, p95 falls 32.4%, maximum 45.1%. These are serial native CPU+GPU completion
measurements with GPU drain. Decode, I/O, source handoff and diagnostic
readbacks/hashes are excluded. They do not establish live game FPS, GPU busy time,
Qt/HUD/audio performance, swap pacing or scanout.

Pixel and contract validation is independent of timing:

- All 868 unmodified captured World frames match the retained earlier native
  renderer across two fresh replays. Each of the 79 selected 2× and 4× stress
  frames also matches its retained native pixel reference across two replays.
- Twenty additive controls pass on llvmpipe and M1 against an independent
  sequential scalar compositor. New 768-source overlap cases check caps 32/512,
  palette changes, channel saturation, copy barriers, counters and no ordinary
  readback. Existing clipping, fallback, budgets and lifetime cases remain.
- Fourteen small-colour comparisons and thirteen displacement comparisons pass
  on both backends, covering other batching that can span the old turn boundary.
- All twelve replay controls pass on both backends, including opt-in additive
  counter export, corruption/pixel refusal, history restoration and viewer rewind.

The original capture stops before World 869's existing caller `0x54a563` refusal.
No original game execution or original pixel-oracle acceptance is added. The
user was not watching the earlier preview, so manual artifact-free acceptance
remains unconfirmed. Cold setup and untested workloads remain outside the
measured combat window. Historical evidence keeps its original hashes and
broader pending applicability.

SQLite evidence `NATIVE13.native-particle-cost02` retains exact source/ELF bytes,
phase-specific diagnostics, baseline/candidate timings and hashes, controls,
commands and original manifest verification. Raw task results under
`working/native-particle-cost-20261113/` receive verified local backups before
finish; parent/derived cases and assets remain shared immutable inputs.
Compiler intermediates and successful interchange tapes are reconstructible
and temporary. No evidence cleanup or publication accompanies this change.

The first report remains retained with nearest-rank percentile calculations.
Report02 standardizes p95 on the existing replay CLI estimator
`sorted[int(.95*(n-1))]`; raw samples and source hashes are unchanged.
