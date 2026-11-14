# Native effect stress findings, 2026-11-13

The authored [effect suite](../../docs/native-effect-stress.md) exercises real
SceneRenderer/GlBlitter paths with fixed-seed moving inputs. It runs without
original simulation, AI, clocks, balance changes, global input or a game session.
The 800×600 RGB565 source pattern, four SPR frames, placements, row offsets,
overlap directions and operations are reproducible from pinned source and exact
tested executables; each frame records its input and pixel hashes.

## Confirmed native ordering failure

An 8×8 keyed RGB565 self copy matches the independent sequential CPU oracle
outside a GL batch and differs inside one on the pre-fix M1 renderer. The same
oversized 65×64 request correctly refuses without changing pixels/draws. This
isolates the failure to native batching, rather than the source-key rule or
capacity guard. The first moving keyed case also independently detects it.
Exact pre-fix source and executable bytes plus the failures remain in task
SQLite; historical reports are unchanged.

`copySharedKeyed` reused one pixel texture while its keyed copies were still
queued, and later reads could miss preceding writes. A flush before each next
source read ensures that the preceding copy has consumed the old source and
updated its destination. Both batched and unbatched copies now match the CPU
oracle; the 4,096-pixel limit is unchanged. This preserves existing row-major
semantics and bounds. Validation covers selected un-clipped RGB565 Blt/key0
inputs; other admitted API/format/clip behavior retains its existing historical
proofs and pending applicability. No live occurrence or connection to the
earlier crowded wizard-death slowdown is established.

## Controlled authored load measurements

Apple M1, three fresh renderer repeats per profile, two warmup frames followed
by eight measured moving frames: 24 measured samples per profile, 288 total.
No other renderer benchmark, game or compiler ran alongside this primary pass.
The final input schema 2 pass fingerprints the full selected palette, clipping,
visibility, row state and copy arguments. Initial pass01 remains immutable;
all360final pixel hashes also match all360initial images. All 360 frames, including warmups, match the independent CPU oracle and repeat
exactly. These are native CPU submission and serial CPU+GPU completion spans;
source authoring/I/O/base upload/readback/hash are outside the measured window.
Diagnostic checks between frames can affect thermal/frequency state. This is
renderer service evidence, not live FPS, GPU busy time, production combat or
original pixel equivalence.

| Case/profile | p95 | Maximum | Above 50 ms |
| --- | ---: | ---: | ---: |
| Wave1: 128 small mixed draws | 13.06 ms | 13.15 ms | 0/24 |
| Wave2: 512 small mixed draws | 51.34 ms | 51.48 ms | 24/24 |
| Wave3: 128 larger serial-fallback draws | 15.83 ms | 16.47 ms | 0/24 |
| Row1: 4 sampling sprites | 1.92 ms | 1.93 ms | 0/24 |
| Row2: 16 sampling sprites | 15.75 ms | 16.44 ms | 0/24 |
| Row3: 32 sampling sprites | 42.46 ms | 44.83 ms | 0/24 |
| Translucency1: 64 mixed draws | 8.18 ms | 8.70 ms | 0/24 |
| Translucency2: 192 mixed draws | 26.16 ms | 27.35 ms | 0/24 |
| Translucency3: 384 mixed draws | 52.30 ms | 53.80 ms | 11/24 |
| Keyed1: 1×256 pixels | 33.29 ms | 34.62 ms | 0/24 |
| Keyed2: 2×1,024 pixels | 245.64 ms | 249.20 ms | 24/24 |
| Keyed3: 4×4,096 pixels | 1,926.05 ms | 1,933.36 ms | 24/24 |

Wave2 submits at most80draws but samples up to6,846,379destination pixels; its
CPU/driver submission dominates that profile. The initial pass records51.33ms p95. The lower draw count does not imply a cheap
native path. Distinguishing map-planning CPU cost from driver waits requires
further targeted attribution; these spans do not establish GPU saturation.
Row3 clones32full canvases per frame. Translucency3 reaches384draws and2,695,424
sampled pixels. Keyed3 reaches16,384ordered one-pixel draws; its correctness fix
preserves serial dependencies and does not optimize this pathological cost.

An additional opt-in timestamp pass validates all four GPU timeline exports.
The initial diagnostic pass could overlap separate World replay compilation;
the final input schema 2 field control runs after compilation. Both validate
fields only and do not support a GPU busy-time or controlled timing comparison.
Primary numbers above are unprofiled.

## Controls, retention and remaining boundaries

The isolated admission tests reject missing/reordered frames, frozen inputs,
repeat/pixel mismatches, ordinary readbacks and incompatible baselines. Native
controls additionally exercise all families, driver identity, fresh repeats,
comparison with a retained successful batch, separately corrupted baseline
refusal, argument rejection before renderer startup and keyed capacity/order.
Software and M1 controls remain separate from M1 performance measurements.

The shared blitter change also receives a full1,107World retained native pixel
regression against the exact prior renderer. This does not renew original/live
milestones. No new original game run is needed; asset-consuming offline runs
retain original-manifest verification before/after.

Cleanup shares identical1MiB ranges among four verified immutable backup
versions, retaining every whole-container hash, file path, page encoding,
catalog and pin. Actual free-space gain is257,323,008bytes (about245MiB),
distinct from kernel-accepted logical sharing bytes. Ninety reviewed
reconstructible .o/.a files (5,840,490logical bytes) are removed after host
consumer/pin/rebuild checks; tested ELF files, source worktrees, installations,
captures and evidence remain. Exact plans/results and build recipes are in
task SQLite. Later compiler output is classified temporary at creation.

`NR.native-effect-stress-suite` and `NR.batched-keyed-overlap-order` remain
bounded native policies. Heavy over-budget profiles are successful test
observations, not passed performance targets. Actual game frequency, combined
fight costs, original pixel equivalence, other formats/API/clip combinations,
Qt/HUD/audio/scanout and long-session behavior remain separate work.
