# Owned World row effects

The native implementation separates indexed source-write displacement580260
from destination-sampling displacement5806f0. Both use owned SPR run history,
one effective shift per source row and tight RGB565 storage. Source placement
clips coverage before shifting writes. Destination sampling preserves encoded
whole-run horizontal clipping and freezes native GPU history once per draw.
Covered linear spans may cross row/clip boundaries. A separate native policy
validates the complete read/write set before changing the canvas and rejects
out-of-owned-span access. Rendering uses source regions or GPU copies without
normal-path pixel readback. World preparation and both synchronous assembly
paths share the same descriptor conversion.

## Original inputs and evidence boundary

Static inspection pins580260 to cached table5f1450, amplitude5e15f8 and phase
6c4830. Unclipped rows index the table with unsigned phase plus screen row,
modulo twice the amplitude. Horizontal clipping instead uses
abs(amplitude - ((phase + row) % (2*amplitude))). The lazy initializer uses a
fixed angular constant5c7858; an ideal sine table parameterized by amplitude is
not established. This cache overlaps destination-wave table blocks.

Unclipped5806f0 indexes5f14d0 +4*(16*amplitude + ((phase+row)%(2*amplitude))).
Its clipped branch uses abs(8 - ((phase+row)&15)), independent of the supplied
amplitude, and its encoded whole-run clipping has an asymmetric right edge.
Initialization writes16 entries per block; amplitudes9..16 may read adjacent
blocks. Initializer state, table writers and their ordering remain separate
from comparison of the pixel bodies.

The comparison authors frozen raw original tables, amplitude and phase in a
private test-only input.bin.rows sidecar. The original reference ignores the
native effective row vector, sets these globals and calls unchanged original
instructions. The native renderer never consumes that sidecar or table/global
addresses. The corpus includes unsigned phase wrap, sparse/adjacent RLE runs,
vertical/horizontal clipping, linear row crossings, two ordered draws, atlas and
non-atlas upload, worker descriptors and a separate rendering process. Rejected
past-end read/write inputs remain native policy controls and are not executed
against arbitrary original pointers.

Original returned status/registers, table generation/writers, source ownership
and lifetime, padded physical storage, arbitrary alias layouts and overflowing
legacy anchor-minus-origin arithmetic remain outside admission. No live
producer emits the new operation11/12 tags yet. These leaves do not close the
broader World dispatcher, kind8 or full live replacement contracts.

## Current result

WROWS.original-20261020 and WROWS.native-20261020 retain486 matching original
canvases,1251666 exact RGB565 WORDs and486 matching separate-process canvases.
Amplitudes1..16 are authored for source placement and9..16 for sampling. Five
phases include unsigned wrap; four sparse geometries cover each clipping branch.
Dense256-pixel runs, ordered overlaps and width-one multi-row jumps are explicit
controls. Past-end read/write refusals preserve the completed native frame.
A preparation-worker descriptor loss discovered by the first test was repaired
with the shared conversion. The sandboxed startup failure and subsequent
reference-build filename failure are retained separately from the successful
comparison. No failed attempt establishes an original pixel mismatch.
