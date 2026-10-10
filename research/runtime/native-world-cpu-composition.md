# Native World CPU composition reuse

The native producer CPU reference previously decoded the same closed SPR frame
on every glyph/raster operation. It also cloned the entire canvas and defined
mask for each nonzero raster mode. Both costs are separate from GPU atlas and
World identity reuse.

`CanvasFrameCache` uses exact owned encoded bytes and the indexed/direct storage
tag. The default LRU admits at most 4,096 frames and 64 MiB of encoded and decoded
vector capacities (pixel, opaque mask and auxiliary planes); map/list/shared
pointer/object overhead, transient decoding allocations and evicted frames still
held by callers are additional.
Lookup reads caller bytes without copying on a hit. Cache keys and decoded output
own their storage. Immutable frame handles survive eviction. Valid oversized
frames decode without admission. Closed size/palette-pointer checks precede
lookup; decoding failures neither enter the cache nor evict valid frames. Draw
colour tables, positions, clips and effect state remain per-operation inputs.

Blends read only their own destination pixel. Displacement admits offsets
0 through 16 and traverses each row from left to right; its source is therefore
the current destination or a pixel still untouched by that draw. Reading the
current owned canvas preserves pre-draw samples without a whole-canvas snapshot.
Negative/out-of-bounds offsets still refuse. Both destination and displacement
sample definedness remain checked. Projected shadow, raster formulas, clipping,
original AX fallback and CPU/GPU equality checks are unchanged. This ordering
argument would need review before admitting negative offsets or changing traversal.

`tests/canvas-sequence-test.cpp` tests repeated blends/displacement against an
independent pre-draw snapshot, transparent/clipped samples, offsets 0/1/16,
negative/out-of-bounds/undefined destination and undefined sample refusals.
Cache tests cover in-place caller mutation and restoration, distinct storage
tags, per-draw palette changes, malformed retry without eviction, LRU ordering,
count/capacity eviction, oversized bypass and handle lifetime after eviction.
The existing World, scene/history, batch and V2 minimap producer tests remain
separate regressions.

The reproducible same-input comparison uses an unchanged isolated HEAD
`59388b1` Debug consumer as its baseline and the owned V1 map-2/zero-items
first-16-queue fixture. `tools/test-world-resource-reuse.py --compare-original`
also runs `tools/compare-world-cpu-original.py`. That helper verifies captured
batch descriptors/checksums against every closed operation, then streams current
native intermediate canvases and low16 AX to the pinned No-CD original routines
through a FIFO. Original routines run only in the independent comparator.
Original diagnostic checkpoints never become native inputs. The historical
capture retains its own source provenance; it does not validate current hook
execution, live writeback, caller ABI or minimap motion branches.

Execution results and remaining boundaries are recorded in the coverage ledger
and the separately preserved native evidence record. Timings describe this
Debug/software-OpenGL diagnostic replay, exclude original simulation and
pre-entry work, and do not establish interactive FPS or hardware-general gains.

[Current preserved native result](native-world-cpu-composition-native-20261010.json)
passes six native tests, all1,058 same-input checkpoints and all23,843 precise
original intermediate canvases/low16 AX across16 queues. It includes concurrent
minimap integration`b0bd799` and prospectively binds100 source/build/test/helper
paths. Warm median CPU composition falls141.556ms to78.498ms
(44.55%). GPU timings fluctuate and establish no GPU gain.
The earlier pre-integration result remains separate historical evidence.
