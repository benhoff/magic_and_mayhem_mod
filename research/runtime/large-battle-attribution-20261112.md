# Large-battle rendering versus original-engine cost

The current native rendering path is the first optimization target for this
reproduced slowdown. A four-owner Quick Battle with twelve ordinary player
Zombie summons completed its movement/contact/battle probe and recorded 89
cross-owner melee damage events. Movement periods observed 42 living actors and
up to 26 actors changing positions. In the final melee periods,29–34 actors remained
alive and the original source cadence fell to 6.6–9.9 Hz, averaging 8.18 Hz.

| Same production run | Source updates/s | Native wall time/world | Submission/world | Original ACK wait |
| --- | ---: | ---: | ---: | ---: |
| Initial quieter period,18 living actors |19.97|16.96 ms|12.13 ms|4.7 ms|
| Movement and approach |11.31|66.51 ms|62.64 ms|54.6 ms|
| Six melee periods combined |8.18|97.92 ms|93.42 ms|91.0 ms|

These are source updates, not repeated Qt paints. Native values come from exact
cumulative profile counter differences:64→192,1280→1856and1920→2240respectively.
Their publication boundaries differ slightly from the actor/input periods;
source rates and ACK durations use consecutive completed shared-channel headers
within those periods. Native timings include CPU/driver waits and can overlap
other pipeline work; they are not GPU execution counters or additive simulation
self-time. Individual movement/battle periods and all raw boundaries are retained.

The original pacing wait drops from 19.3 ms in the initial period to zero in the
melee periods. The producer spends about 91 ms waiting for native acknowledgement
while native submission consumes about 93 ms per world. The original main thread's
92%of one core is not proof of expensive AI: the adapter's `continuous_wait`
loop uses `Sleep(0)` while awaiting that acknowledgement. CPU samples include
translation, bridge work, polling and game work. No original AI self-time is
claimed. Native thread runnable delays, audio request/reply samples and host
pressure observations are retained separately.

## Replay without the original engine

After stopping the owned test, one verified completed packet remained available:
World 2274, SHA256
`58873c3ba289c2defd11020c816b77619561063ebbbf0335ce5761108965df75`.
Source/request CRC32C and stable header checks pass; its matching request and
actual return bytes are retained. This is a complete individual packet, not a
complete preceding World history.

The existing `combat-world-replay` test renders that packet independently on
Apple M1 (G13G B1). It compares the existing serial path with current small-colour
batches on an authored RGB565 background. All50 full-image comparisons match.
After five warmup passes per mode:

| Native mode | Mean submit | Mean synchronized completion | Draw calls | Texture uploads |
| --- | ---: | ---: | ---: | ---: |
| Current batches |63.33 ms|64.29 ms(range61.98–67.38)|694|0|
| Serial control |207.94 ms|209.03 ms|1880|206|

Even the warm current replay exceeds the50 ms budget for20 FPS without running
original simulation or issuing sound requests. Completion includes a diagnostic
readback after ordinary submission; ordinary drawing itself adds no readback.
This is independent evidence of native draw/compositing overhead, not a claim of
original pixels, original history or physical scanout equivalence. Existing
batching helps substantially, but does not close the large-scene budget.

The retained source has 1434 colour-rectangle requests, mostly 2×2blends, and1315
sprite/effect raster requests. Binding produces 2746 draw commands. The live melee
profile averages1370.59 native draw calls and104.52 texture uploads(1.405MB) per
world, versus123.41 draw calls and2.13uploads in the initial period. Frozen replay
has less cache churn and different pipeline contention; its difference from live
time cannot be assigned entirely to texture uploads. Reduce native draw/pass
and destination-synchronization overhead first, then investigate animated-frame
cache/upload churn. Reassess original-engine costs after removing this bottleneck.
A native instruction/GPU-counter profile is still needed to split driver CPU,
GPU execution and synchronization precisely. No optimization is shipped here.

## Preserved failures and limits

The initial setup routed to the old input helper because its PATH was wrong.
It was stopped and is not accepted. The corrected runner checks its exact
viewport-only helper before launch and refuses the global combat-input fallback;
all accepted mouse actions require acknowledgements from `nativeGameViewport`.

The next run observed 43 living actors and52 cross-owner melee hits, maintaining
19.89–20.01 Hz in its warm five-second windows, then stopped at original exit 95.
The partial unacknowledged source records failure 43,tag 14,caller`0x54a563` in
attempted World 657(after 656 completed worlds). This is a separate native raster
admission gap. It remains unadmitted and its failure is preserved. The later
successful battle uses the same unchanged production renderer/guards.

Both recording-free runs use production ELF
`d91146e94040430a491fc5edf005f2f7eab2ae9e05332b41c44122291e8a2be0`
from source revision`fee84e6265a5623ca1939bf216eaed30304ce223`, ordinary map 2
Quick Battle rules(mana 200,health 800,control 30), current audio handoff, and owned
private prefixes. No game clock, original AI, balance, graphics admission or user
save changes occur. Original2927-file manifests pass before/after each run and
the asset replay. Tests end by diagnostic cancellation, not normal Quit.

Battle03 did not retain the requested terminal per-turn trace on cancellation;
its live aggregate reports, passive headers and thread/actor observations survive.
The per-turn-window analyzer's refusal is retained. Battle02's terminal trace
survives, but does not substitute for fresh per-turn data in the slower run.
Different placements, finite scenarios and observer/sampling uncertainty limit
generalization. Preserve historical milestones and source hashes independently.

[Machine-readable results](large-battle-results-20261112/attribution01.json) are
SQLite-backed. Raw inputs, exact scripts, failures, packet and tested replay ELF
are held in `working/battle-attribution-20261112/` with verified SQLite backups;
the unchanged production ELF reuses its previously retained exact backup.

## Native optimization: ordered pixel chains

The selected change replaces the small-colour renderer's overlap-split snapshots
with one ordered operation chain per destination pixel. A maximum of 512 small
rectangles expands to at most 2048 pixel operations. Stable grouping preserves
all per-pixel copy/half/quarter blend steps and integer rounding. One destination
snapshot and one instanced draw evaluate the group; non-small primitives and
existing caller budgets remain barriers. No game timing, AI or balance changes
are involved.

On the same retained World 2274, warm completion falls from **64.29 ms to
52.83 ms** (17.8% less time); submission falls from 63.33 ms to 51.25 ms and
submitted draws from 694 to 596. The 50 complete image comparisons pass with the
same `6b53563bcc7377224bffc54df90fcb2f01d5aef1880997476239e3251b33bf25`
output hash. This still exceeds the 50 ms budget before live simulation and
resource changes. The result is a bounded improvement, not a 20 FPS guarantee.

Fourteen scalar/serial colour comparisons per backend pass on llvmpipe and Apple
M1, including a 512-write dense overlapping chain, clipped geometry, mixed blend
modes, hidden/larger primitive barriers, incremental budgets, invalid input and
undefined destination refusal. Cache, sprite-atlas and retained-image reuse
regressions also pass on both backends; the retained-image test includes 92
destination-dependent repeats and verifies no ordinary readback.

A broader disjoint-sprite blend candidate passed 27 serial comparisons per
backend but did not reduce the optimized battle's 596 draws or improve its
completion time (53.91 ms). It was withdrawn; those candidate sources, binaries,
results and its initial recursive-snapshot allocation failure remain retained.
The earlier shader compile refusal is also preserved. Neither prototype failure
was exercised in a live game. The final production changes are confined to the
small-colour shader and its bounded CPU/GPU operation buffers.

### Moving battle with pixel chains

The first optimized live run (`battle04`, native ELF
`971783872b514cb1e8b10d6db0f5b628d243b00e972b380fccee3b22505ad75f`)
accepts 12 ordinary Zombie casts, samples 28 living actors and 26 moving actors,
and records 163 original cross-owner melee hits. Movement/approach averages
19.90 source FPS and melee averages **19.66 FPS**. During the melee aggregate,
native submission averages 8.58 ms/world, total native work 13.89 ms/world and
original ACK wait 3.27 ms. The user reports **no visible artifacts** in this run.

This scene is substantially lighter than `battle03`: about 329 native draws and
9 texture uploads per melee World versus 1371 draws and 105 uploads. Different
creature populations, effects and trajectories prevent attributing that entire
live FPS difference to the code change. The retained-frame replay above supplies
the controlled performance comparison. This live run ends by intentional
diagnostic cancellation after its movement/melee proof, not normal Quit; its
source, timings, input acknowledgments and original manifests are retained.

### Rejected texture-barrier variant

A separate experimental path used the single-read/single-write, same-texel rule
in [ARB_texture_barrier](https://registry.khronos.org/OpenGL/extensions/ARB/ARB_texture_barrier.txt)
to avoid explicit small-colour snapshots. Both the enabled and forced-snapshot
paths passed 14 scalar/serial comparisons on each backend and 50 full retained
battle-frame comparisons per policy. However, Apple M1 warm completion regressed
to 76.15 ms, versus 53.70 ms for the forced snapshot fallback in the same build.
Its 43.33 ms submission time alone would have hidden that regression. The
candidate was withdrawn in full. Final code retains the tested snapshot-based
pixel chains; no texture-barrier capability path or new environment flag remains.
