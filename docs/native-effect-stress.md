# Native effect stress tests

Run all four effect families, each at three load levels:

```sh
./tools/effect-stress.py run
./tools/effect-stress.py show
```

The first command builds the small native runner when needed. It uses the real
renderer on the current display, without launching the game or injecting input.
The default runs eight measured moving frames after two warmup frames, with
three fresh renderer repeats per case. Inputs use a fixed seed and an 800×600
RGB565 canvas. Every frame is checked against an independent sequential CPU
oracle. Input schema 2 also fingerprints the selected palette, clip/visibility/row
state and copy arguments, so changed test data refuses baseline matching. Exact source/executable bytes, input/pixel hashes, warmup observations,
timings, counters and failures go directly into SQLite under
`working/effect-stress/results.sqlite3`.

| Case | Loads exercised |
| --- | --- |
| `wave` | 128 small displacement/copy draws; 512 interleaved small draws; 128 larger draws that exceed the existing 128-pixel batching extent |
| `row-distortion` | 4, 16 and 32 destination-sampling sprites, increasing from 31×15 to 127×47; whole-canvas clones and per-run copies |
| `translucency` | 64, 192 and 384 overlapping moving draws, mixing half/quarter-source blends with projected shadows and four changing source frames |
| `keyed-overlap` | 1×16², 2×32² and 4×64² ordered self copies; alternating overlap direction and transparent key pixels |

Levels exercise different count, area and batching branches; elapsed time can
decrease between levels. Each profile has its own fixed input definition.

The existing keyed-overlap limit is **4,096 pixels per operation**. The largest
case stays at that limit; the guard control also checks that 65×64 refuses before
pixel mutation. No renderer guard is increased for testing.

Select a smaller run or retain it at an explicit destination:

```sh
./tools/effect-stress.py run --case wave row-distortion --levels 1 2 \
  --frames 8 --repeat 3 --results working/effect-stress/distortion.sqlite3
./tools/effect-stress.py show --results working/effect-stress/distortion.sqlite3
```

`--backend software` selects llvmpipe under Xvfb. Hardware runs use the real
display; the actual driver is recorded and a software fallback refuses hardware
acceptance. Never run a performance comparison alongside a game, compiler or
another renderer benchmark.

The command reports p95/maximum completion, frames above the default 50 ms
budget, draw/upload counts, sampled pixels and full-canvas clone counts. A slow
case is an observation, separate from a pixel/contract failure. Use
`--require-budget` to make a p95 overrun fail the command. `--budget-ms` changes
the reported threshold; it does not change the drawing workload.

Use a successful batch as a pixel reference for a later renderer change:

```sh
./tools/effect-stress.py run --against 1
```

The case/level set, generated inputs and every frame's pixels must match. The
same source and executable bytes remain in `inputs`; old results are preserved.
Use matching frame/repeat/warmup settings. A different driver is identified in
the comparison and does not establish a controlled performance result.

`--gpu-timing` enables diagnostic GPU timestamps. These perturbed timeline
intervals include idle gaps during CPU submission; they are **not GPU busy
time**. Primary submission/completion measurements exclude source authoring,
file I/O, base uploads and diagnostic readback/hash work. Scene admission,
background copy and effect work are included. CPU oracle/readback checks occur
between completed samples and can affect thermal/frequency state; these are
bounded renderer service measurements, not live FPS, Qt pacing, HUD/audio or
original-game pixel equivalence.

The native runner accepts at most 256 total frames per case invocation; the CLI
also imposes a 180-second timeout per case. Use bounded levels rather than
increasing production limits. Synthetic movement and draw inputs exercise the
renderer; original AI, simulation, clocks and balance are unchanged.

## Keyed overlap regression

The initial stress run exposed a native ordering bug: an 8×8 keyed self copy
matches the CPU oracle unbatched, but differs inside `GlBlitter::batch`. Queued
copies had not consumed the reusable one-pixel texture before its next update,
and later source reads could miss preceding writes. Flushing pending copies
before each pixel read preserves row-major dependencies and source lifetime.
The same batched/unbatched/capacity controls now pass. This fixes native batch
ordering; no live occurrence or connection to the earlier wizard-death slowdown
is established.

Focused controls can run independently:

```sh
python3 tests/test-effect-stress.py
python3 tests/test-effect-stress.py --renderer working/build/render-replay/mnm-effect-stress --hardware
```

Omit `--hardware` for the software renderer. Pixel discrepancies, frozen inputs,
missing/reordered frames, repeat changes, baseline corruption and ordinary
readbacks refuse acceptance. Cold/warm observations remain separate. Tested
executables and unique results are durable; compiler intermediates and authored
temporary fixture directories are reconstructible.

Ordinary CI runs the isolated Python admission controls. Actual GPU controls
remain explicit local runs. Measured findings and retained source phases are in
[the runtime record](../research/runtime/native-effect-stress-20261113.md).

Native throughput improvements and their pixel/fallback boundaries are recorded
in [the optimization review](../research/runtime/native-effect-throughput-20261113.md).
`MNM_NATIVE_FEEDBACK_BASELINE=1` selects the snapshot fallback for supported
same-pixel blends, for controlled comparisons. It does not change the authored
inputs or the original engine.

Build focused optimization controls with:

```sh
cmake --build working/build/render-replay --target mnm-native-effect-optimization-test mnm-row-effect-test -j2
QT_QPA_PLATFORM=xcb working/build/render-replay/mnm-native-effect-optimization-test
# Choose a new output prefix for each row-effect run.
QT_QPA_PLATFORM=xcb working/build/render-replay/mnm-row-effect-test working/row-effects-new
```

The optimization control independently checks ordered keyed copies, blend
arithmetic, row-crossing history and oversized fallback. Set
`LIBGL_ALWAYS_SOFTWARE=1` under `xvfb-run -a` for the software route.
