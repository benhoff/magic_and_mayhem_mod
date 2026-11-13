# Heavier native renderer stress cases, 2026-11-13

The new [particle stress generator](../../tools/render-stress.py) derives
repeatable authored cases from the retained 868-World combat recording. It adds
translated copies only of in-World operation-8/backend-13 indexed additive
particle requests. Terrain, creatures, shadows, other effects and source-history
commands stay single. This adds rendering work, not simulated actors or AI.
See [usage](../../docs/render-replay.md#create-a-heavier-particle-replay).

Every derived case retains history from generation 1. The selected stress range
is 790–868; earlier Worlds use the neutral 1× conversion. Factors 1/2/4/8 and
spreads 0–128 pixels are bounded; default spread is 24. Source definitions and
references are renumbered after insertion. Captured requests are admitted before
conversion, then replaced by explicitly authored native queue closures, never
presented as original guarded returns. Parent and child wire preflight starts no
renderer; drawing/resource correctness is a separate execution check. Existing
12,320-raster, 131,072-record, 16 MiB source and SQLite case bounds stay enforced.

Three M1 native completion benchmark repeats per case, identical range 790–868,
237 samples each, sequential runs without competing game/compiler/test work:

| Particle requests | Peak per World | Median | p95 | Maximum | Samples >50 ms |
| --- | ---: | ---: | ---: | ---: | ---: |
| 1× control | 1,228 | 21.665 ms | 37.831 ms | 43.947 ms | 0/237 |
| 2× | 2,456 | 24.008 ms | 40.221 ms | 54.415 ms | 3/237 |
| 4× | 4,912 | 28.284 ms | 50.016 ms | 68.532 ms | 13/237 |

Factors count requests; translated clipping/overlap and batching make native
cost nonlinear. The peak total raster counts are 2,061 / 3,289 / 5,745, within
the existing cap. Frame 860 reaches 68.532 ms at 4×, versus 43.947 ms in the
control. A separate natural later window, 821–868 in the unmodified parent,
measures p95 39.820 ms / maximum 43.715 ms across 144 samples. No causal
performance comparison is made across different selected ranges.

The 4× workload exposes occasional native budget overruns: 13/237 samples
(5.5%) exceed the 50 ms budget for 20 updates/second. This is a retained test for
future rendering optimization, not evidence that AI or the original engine needs
changing. Existing owner-counter diagnostics at 858–860 put approximately
46.7–49.1 ms in scene drawing, 2.3–2.5 ms in preparation and 1.3–1.5 ms in
producer work. That perturbed diagnostic overlapped correctness controls and is
not another performance benchmark or a CPU-self/GPU-busy attribution.

Correctness and acceptance boundaries:

- Both 2× and 4× cases match the pre-optimization serial renderer at every
  selected frame across two fresh optimized replays (79 frames per repeat).
- All 868 neutral-conversion frame hashes match the retained parent reference
  across two fresh replays. Packet closure/reference repair changes no pixels.
- Seven stress-authoring tests pass on llvmpipe and M1: particle-only changes,
  later source-reference repair, signed anchors, bounds/closure/raster limits,
  history preservation, no-particle/existing-output refusal, neutral native
  pixels, serial/batched stress pixels and malformed-parent native refusal.
- Existing 12-test replay suites pass on llvmpipe/M1, including history, rewind,
  baseline refusal, codec and diagnostic readback contracts.
- The actual Qt 4× viewer reaches frame 868 and closes normally, with zero
  ordinary readbacks and one explicit diagnostic read. No game or global input
  injection is involved. The user was not watching; manual visual acceptance
  remains unconfirmed.

No new original-game acquisition or engine/simulation/balance/clock/guard change
was made. These are World-only renderer service times, excluding source decode,
handoff, I/O, diagnostic reads and hashes; not live FPS, full HUD/audio/presentation
or scanout measurements. The parent acquisition's guarded World-869 failure
remains outside the retained prefix. Native pixel regression equivalence is not
an original-game pixel oracle for authored stress inputs.

Cases are retained in `working/heavy-render-stress-20261113/` in the owned task
checkout: `control-1x.sqlite3`, `particles-2x.sqlite3`, `particles-4x.sqlite3`.
The generator and normal replay CLI make them reusable. Exact inputs, ELF/tool
bytes, parent/case hashes, generation counts, failure controls, per-frame timings
and hashes, commands and viewer reports are retained in task SQLite stores with
verified local backups before finish. Original capture/assets remain shared
read-only; successful compressed interchange tapes are reconstructible temporary
output. SQLite evidence `NATIVE13.heavy-render-stress01` binds the scoped native
behavior and execution sources; prior evidence keeps its original hashes.
