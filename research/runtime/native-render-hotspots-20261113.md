# Remaining native render hotspots, 2026-11-13

This work targets native rendering and asset preparation only. It changes no
original simulation, AI, tick rate, gameplay balance, original binary or shared
wire layout. The accepted parent is local master `955f9d22d`.

## Ordered overlapping blends

`NR.ordered-composite-tiles` groups consecutive half/quarter/additive draws with
matching source, mask, destination and packed-colour storage. Each 32×32 tile
receives an ordered list of covering commands. A fragment reads its destination
once, evaluates those commands in their original order with the original mask,
palette and integer rounding rules, then writes once. Transparent commands leave
the running value alone. This admits overlapping sprites without changing their
history or relying on inter-primitive texture feedback.

A pass holds at most 512 commands and 65,536 tile references. The largest owned
canvas has 4,096 tile headers. Three bounded texture buffers retain descriptors,
headers and references; no destination CPU readback occurs. Capability-gated
texture barriers separate passes; unsupported contexts and
`MNM_NATIVE_COMPOSITE_BATCH_BASELINE=1` use the existing serial path. The same-
texel single-read/write boundary follows the official
[ARB_texture_barrier specification](https://registry.khronos.org/OpenGL/extensions/ARB/ARB_texture_barrier.txt).
Neighbour-sampling effects retain their frozen source path.

## Uploads and copies

`NR.upload-dependency-flush` validates row uploads before mutation, then checks
every pending copy/blend source, mask and destination rectangle. An intersecting
upload flushes prior work. A disjoint upload can leave commands queued because
integer texelFetch has no filtering halo. Palette-row eviction still flushes
before reuse. Ordinary updates, snapshots, reads, exports, self copies and
surface deletion retain their ordering boundaries. Set
`MNM_UPLOAD_FLUSH_BASELINE=1` to force the earlier unconditional flush.

`NR.opaque-row-copy-batching` admits opaque copies of at most 4,096 pixels into
the existing 512-instance copy batch. Distinct compatible SurfaceCopy calls can
share a batch; ordered same-surface copies still flush and freeze each piece.
Larger transfers retain the direct copy path. This reduces many short row
copies in large-effect fallback without widening the destination-sample API's
65,536-pixel limit or changing the full-clone fallback policy. Set
`MNM_OPAQUE_COPY_BASELINE=1` for the prior small-copy submission path.

`NR.regional-producer-copy-snapshot` freezes only the cropped rectangle's words
and authority states for native CPU producer self copies, rather than copying
the entire surface and dirty-row bookkeeping. CPU frozen-source semantics,
clipping, source validation, opaque/keyed propagation, undefined/GPU-only states
and mirror operation order remain independent of the GPU keyed-overlap policy.
This changes the native canvas reconstruction service, not the original engine.

## Displacement ordering

`NR.displacement-radix-order` uses two stable 11-bit radix passes for destination
coordinates, which occupy at most 22 bits in a 2048×2048 canvas. Unique map keys
retain exactly the previous ascending destination order and span merging.
Touched keys, per-sprite changes, ordered output, radix scratch and spans reuse
owned vectors. The existing 32-sprite/65,536-visited-pixel bounds, pre-sprite
staging, deadline/caller budgets and serial/whole-run fallbacks remain intact.
Cache-pressure release retires this reusable scratch as well as the lookup.

## Cold JPEG discovery

`NR.jpeg-codec-discovery` replaces enumeration of every Qt image codec with the
explicitly selected JPEG reader's readiness check. A fresh-process authored
probe measured 36 ms for all-format discovery versus 2 ms for JPEG discovery;
a previous discovery-only probe measured 95 ms. These are setup microprobes,
not renderer frame timings or a guaranteed startup reduction.

Complete SOI/EOI framing, encoded/decoded allocation limits, preallocation
checks, RGB conversion and orientation policy remain intact. Malformed JPEGs
must still report data errors; an unavailable JPEG handler must retain the
codec-unavailable category. Independent Pillow comparisons cover eight authored
RGB/grayscale/progressive/orientation fixtures and all 752 installed JPEG files
(126,954,548 decoded pixels), with original-manifest checks around consumption.

## Controls and evidence boundaries

The new hotspot test evaluates 7,558 authored scalar operations across optimized
and forced serial paths: overlapping/disjoint RGB565 and RGB555 blends, palette
changes/eviction, the command/tile-reference caps, transparent masks, disjoint
and intersecting source/mask/destination uploads, rejected uploads, source
retirement, opaque copy batches and clipped frozen producer copies. Existing
native optimization, row, displacement, additive, small-colour, atlas, canvas
and JPEG controls cover adjacent paths.

The additive aggregation control explicitly disables the newer lower-level
blend batching in its own renderer. Otherwise its former serial reference also
batches, invalidating its draw-reduction assertion. Pixel/scalar and retention
assertions remain; default tile integration is tested independently. The original
failed assertion and exact source/ELF state are retained, rather than presented
as a passed execution.

Exact phase-specific sources/ELFs, raw samples/counters, unchanged input/pixel
references, setup diagnostics and controls live in task SQLite with verified
backups. The first disjoint-only prototype remains historical evidence; ordered
tiles are the final algorithm. No original-game run or original pixel-equivalence
promotion is implied. Correctness replays with diagnostic reads are separate
from isolated performance measurements, live FPS, UI/audio and scanout.

A tool-isolated llvmpipe invocation aborted before useful renderer diagnostics;
the retained unmodified parent ELF also aborts in that environment. Both the
optimized executable and all controls pass on host llvmpipe. Preserve both
negative setup records; host software validation is the supported execution
context, and the isolated abort is not presented as a renderer regression.

## Quiet M1 rendering measurements

Three fresh renderer repeats per profile, two warmups followed by eight measured
moving frames each. All 360 images match the accepted parent, independent CPU
and repeats; all 288 measured frames remain below 50 ms. The measured blitter,
canvas and scene source bytes match the final renderer exactly. Subsequent JPEG
readiness and test/build changes have fresh software/codec validation; these
hardware observations are not relabelled as execution of that final linked ELF.

| Profile | Parent p95 | Ordered-tile/radix p95 | Draw calls |
| --- | ---: | ---: | ---: |
| wave 1 | 6.05 ms | 3.87 ms | 20 |
| wave 2 | 22.99 ms | 12.40 ms | 80 |
| wave 3 | 15.70 ms | 15.21 ms | 128 |
| row-distortion 1 | 0.87 ms | 0.96 ms | 4 |
| row-distortion 2 | 2.26 ms | 2.67 ms | 16 |
| row-distortion 3 | 5.19 ms | 4.85 ms | 32 |
| translucency 1 | 5.56 ms | 1.01 ms | 1 |
| translucency 2 | 13.19 ms | 1.19 ms | 1 |
| translucency 3 | 25.32 ms | 3.02 ms | 1 |
| keyed-overlap 1 | 0.34 ms | 0.38 ms | 1 |
| keyed-overlap 2 | 0.47 ms | 0.49 ms | 2 |
| keyed-overlap 3 | 0.67 ms | 0.96 ms | 4 |

The first cold hardware frame still measured 273.61 ms before the final JPEG
readiness change; later fresh-renderer repeats measured 113.80 and 114.75 ms.
Those samples retain startup/driver/source setup and are not passed 50 ms
startup targets. The final JPEG-linked hardware cold-start total is unclaimed
while the user's game remains active. One-thread host llvmpipe validates the
final linked build against every retained World and particle pixel instead.
No live-game input, termination or save-state manipulation was performed.
