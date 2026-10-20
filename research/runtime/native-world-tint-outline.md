# World tinted run-pair raster

The581200 leaf selected by kinds12..14 scales low-byte RGB inputs by
(shade+127)/127, clamps to255 and converts toRGB565. With unclipped horizontal
geometry it writes pairs(start-1,start) and, for runs longer than one pixel,
(end-2,end-1). A one-pixel run writes its preceding WORD and itself. These
pairs may cross a row or clip boundary. Horizontal clipping instead selects
palette half-blend; vertical clipping alone retains tinted pairs.

The initial prospective endpoint-only comparison exposed an incorrect model:
an original first-pixel write at the canvas origin preceded the allocation
and later aborted at heap cleanup. Preserve that failed attempt separately;
it is not evidence about an actual live game fault. The revised comparison
uses owned padding and declares the corrected pair write set before execution.

Native scene admission derives every pair before mutation. The separate policy
NP.tint-owned-canvas-writes refuses negative/past-end WORDs, and permits linear
row/clip-edge crossing inside its owned canvas. This deliberate policy makes
no equivalence claim for arbitrary original physical writes. Native cached two-pixel rectangles
implement the admitted spans without CPU pixel readback.

Owned tag10/backend15 carries the selected palette plus three raw RGB DWORDs.
The tagged input requires tight canvas rows(stride==width); padded original
storage remains outside this admission. The bounded shade domain-127..127 is
under comparison. Wider arithmetic,
return/register state, other masks, source writers/ownership and live producer
admission remain pending.


WTINT.original/native-20261020 declares its exact contracts/source closure
before execution. The final480-fixture matrix compares13,967,543 RGB565 WORDs
with zero differences:260tint cases,136ordinary-outline cases,66additive
sprites,17kind8 primitive regressions and one indexed direct-distortion/top-clip
regression. All255bounded shades and every RGB low-byte value are exercised.
The397outline/tint/distortion cases repeat through the separate renderer
process. Worker/atlas paths, raw high RGB bits, strict wire refusal, safe linear
row/clip crossings and atomic negative-span refusal pass. Native drawing uses
no normal pixel readback. Original manifest bookends and source stability pass.

The original replay harness now calls596490 for backend9 instead of routing it
to the distinct5806f0leaf. It restores the effective16-entry table with the
clipped-top phase adjustment and permits indexed RLE masks, because596490does
not read source colour words. The new top-clip fixture executes that path.
This reference correction does not resolve the separate88pixel live-capture
difference; see [the retained diagnostic](native-world-kind8-replay-gap-20261020.json).
