# Native effect throughput, 2026-11-13

This change optimizes native renderer service time on the unchanged authored
[effect workloads](../../docs/native-effect-stress.md). It changes no original
engine simulation, AI, tick rate, balance or wire layout. Native pixel equality
and original-game equivalence remain separate claims.

## Implementation and safety boundaries

`NR.keyed-overlap-chain-rendering` replaces thousands of serial one-pixel
source reads/draws with one source-rectangle snapshot and one draw per clip
piece. A translation toward a later row-major coordinate can observe earlier
writes. For each pixel the shader walks that predecessor chain from its oldest
ancestor and propagates the first non-key complete native word; if every word
matches the masked key, the fragment discards. Opposite translations sample only
the initial word. Each subsequent clip piece snapshots the current image.
The 4,096-pixel admission cap, result/format/validity checks, key masks and
piece order are unchanged. This is bounded dependency resolution, not a frozen
opaque copy or unordered parallel copy.

`NR.displacement-lookup-storage` replaces temporary node-based hash maps with a
lazy 131,072-cell open-addressed table (1 MiB), at most half full under the
existing 65,536-visited-pixel/32-draw limits. Per-sprite changes are staged before
updating the table. A scoped reset clears touched keys on success, early
fallback and exceptions. Coordinates retain the previous identity mapping and
ordered span export. Deadline, caller budgets, large-sprite/whole-run/software
fallbacks and the retain-outside policy are unchanged. Cache pressure can release
the table through `releaseCachedImages`. Local touched/staging/output vectors
remain bounded by the existing workload caps.

`NR.regional-row-sampling` converts eligible row-effect plans to the existing
bounded destination-sample API. Runs split at both source and destination row
boundaries; one regional snapshot per sprite preserves its frozen source image.
At most 65,536 pixels and spans are admitted. Larger, isolated and software
requests retain the full clone path, without splitting one operation into
multiple snapshots. Source-placement effects retain their previous path.

`NR.ordered-texture-feedback` uses capability-gated `glTextureBarrier` for
half/quarter/additive sprite blends and additive rectangles. Each fragment
reads and writes only its own texel, once, after flushing preceding copies and
issuing a barrier before each draw. Neighbour sampling still takes a snapshot.
The fallback remains available on unsupported contexts and through
`MNM_NATIVE_FEEDBACK_BASELINE=1`. Integer blend rules, including rectangle
addition wrapping before saturation, are unchanged. This use follows the
single-read/write exception and inter-draw ordering in the official
[ARB_texture_barrier specification](https://registry.khronos.org/OpenGL/extensions/ARB/ARB_texture_barrier.txt).
A core 4.5 or extension capability and resolved function are both required.

## Validation scope

The new focused test compares ordered keyed copies to an independent in-place
CPU oracle over 8-bit, RGB565, RGB555 and 32-bit storage, unused bits, masked
keys, zero/positive/negative translations, clipped pieces, outer batching,
2,047-pixel dependency chains and the unchanged capacity refusal. Independent
half/quarter blends cover overlapping masked writes over a 257×256 canvas,
RGB565 and RGB555, with feedback enabled and forced snapshots. Dense row tests
cover source row crossings, pre-sprite history, 60,000-pixel regional input,
67,500-pixel fallback and cache release. There are 2,410 comparisons per backend.
A separate llvmpipe control forces OpenGL 3.3 and disables both
`ARB_texture_barrier` and `ARB_copy_image`; the same 2,410 comparisons pass
through the portable fallback. The existing 486 row fixtures, 13 displacement comparisons, 52 additive batching
cases and 14 small-colour comparisons exercise adjacent production paths.

Exact baseline/candidate ELFs and sources, input/pixel hashes, raw warmup and
measured timings, guard/negative diagnostics and reconstruction commands are
retained in SQLite. The initial candidate pass overlapped test compilation and
is exploratory; final performance comes from a separate quiet run. Retained
World/particle comparisons test previous native pixels without another original
engine execution. No live FPS, scanout/UI/audio, original pixel equivalence,
universal hardware or wizard-death causal fix is asserted.

## Quiet unchanged-input measurements

Apple M1, eight moving measured frames after two warmups, three fresh repeats
per profile. The fresh baseline uses the exact previous ELF. Compilation and
other renderer tests were stopped for both baseline and final passes.
All 360 final images match the CPU oracle, repeats and the corresponding
baseline hashes. All 288 measured final frames finish below 50 ms.
These are serial CPU+GPU native service spans, not GPU busy time or live FPS.

| Profile | Before p95 | After p95 | After maximum |
| --- | ---: | ---: | ---: |
| wave 1 | 13.14 ms | 6.05 ms | 6.47 ms |
| wave 2 | 51.15 ms | 22.99 ms | 23.21 ms |
| wave 3 | 16.39 ms | 15.70 ms | 16.83 ms |
| row-distortion 1 | 1.91 ms | 0.87 ms | 0.92 ms |
| row-distortion 2 | 14.69 ms | 2.26 ms | 3.03 ms |
| row-distortion 3 | 43.40 ms | 5.19 ms | 5.34 ms |
| translucency 1 | 7.90 ms | 5.56 ms | 5.80 ms |
| translucency 2 | 27.66 ms | 13.19 ms | 13.32 ms |
| translucency 3 | 52.26 ms | 25.32 ms | 25.90 ms |
| keyed-overlap 1 | 32.03 ms | 0.34 ms | 0.68 ms |
| keyed-overlap 2 | 242.08 ms | 0.47 ms | 0.65 ms |
| keyed-overlap 3 | 1943.77 ms | 0.67 ms | 0.72 ms |

The largest wave profile within the batched domain falls from 51.15 to
22.99 ms p95. The larger-sprite serial-fallback wave profile remains unchanged
in algorithm. Dense row sampling no longer clones 32 canvases per frame;
regional snapshot pixels peak at 238,760. Dense keyed overlap uses four draws
in place of 16,384. Same-pixel translucency needs no destination snapshots
on the supported M1 feedback path. Final batch 3 is authoritative; batch 2
is retained as exploratory because its early profiles overlapped compilation.
