# Owned minimap markers and camera corners

This increment is independent of the interactive World batch launcher and
profiling pipeline. The standalone `renderer/minimap/overlays` library consumes
owned view/colour/visibility/corner inputs and writes owned positive-stride
16-bit storage. No game pointers, Qt types, original destination oracles or
simulation calculations enter its interface. RGB565 and RGB555 selected output
constants are separate recovered cases. The existing terrain implementation,
producer wire and runtime hooks are unchanged.

## Cell markers — `RS.minimap-cell-marker`

No-CD `0x5536c0` is a thiscall pixel writer with four stack arguments: source
cell X/Y, palette index and emphasis flag. It wraps the source coordinates around
centres `+0x61a1/+0x61a5` using global grid dimensions `0x6c5494/0x6c5498`.
The selected nine-word palette begins at `+0xcb`. Emphasis becomes the eight-word
ring only when `+0x6141` is also nonzero; otherwise the writer emits a 2x2 square.
The return EAX is a destination pointer, not a success flag. Normalized to the
unemphasized projected anchor, the normal return is `(0,1)` and ring return
`(2,2)`. Native return values are owned word offsets, never original addresses.

The wrapped source coordinates are `u=wrap(x+floor(w/2)-centerX,w)` and
`v=wrap(y+floor(h/2)-centerY,h)`. Rotation consumes the original object's
independent `+0x103/+0x107` extents `rw/rh`, rather than assuming grid counts or
their minus-one values:

| Orientation | U | V |
| --- | --- | --- |
| 0 | `u` | `v` |
| 1 | `v` | `rw-u` |
| 2 | `rw-u` | `rh-v` |
| 3 | `rh-v` | `u` |

Projected placement is `originX+U-V`, `originY+trunc((U+V)/2)` with origins
`+0xf7/+0xfb` and destination word stride `+0xeb`. The original pointer-return
checks bind this geometry to caller-visible output as well as pixels. Palette
construction, unknown indices/callers and machine scratch state remain separate.

## Creature markers — `RS.minimap-creature-markers`

No-CD `0x553850` refreshes object grid fields `+0x6199/+0x619d` from the globals,
including on empty/all-hidden lists. Its `+0x614d/+0x6151` range contains 12-byte
X/Y/Z records. Visibility reads the signed byte at
`[0x6c5c5c] + [0x6cb942 + y*4] + [0x6cb8c2 + (z>>1)*4] + x`, compares it with
`0x5e18f0`, and skips that record only while fog `0x5e1404` is enabled. Native
input is the closed per-record hidden decision; native visibility generation
from simulation and source-table lifetime remain outside this raster contract.

Visible markers use the same four-view projection but draw thirteen words with
RGB565 `0xfc00` or RGB555 `0x7e00`. The native result returns the refreshed grid
dimensions so a future adapter can apply the required object updates. Its drawn
count is a native diagnostic value, not a recovered original return contract.
The original's row-pointer table `+0xf3` is replaced by owned positive-stride
storage; arbitrary original row-pointer aliases/layouts are not claimed.

## Camera corners — `RS.minimap-camera-corners`

Four original entry functions `0x5527a0/0x552b50/0x552f20/0x5532f0` correspond
to views0/1/2/3. The selected contract admits `+0x6207 != 0`, skipping the outer
diamond border, and an empty subsequent cell-marker list `+0x6119 == 0`.
Terrain is a separately validated producer; the private reference replaces
only signature-checked `0x553a40` with return for these outline calls. Outline
functions, their corner loops and RGB helper `0x555e20` execute unchanged.

View 0 retains the caller origin. Views 1/2 set `originX=stride-rh-8`, view 3 sets
`originX=stride-rw-8`; views 1/2/3 set `originY=6`. Half extents A/B are rw/rh
for views 0/3 and rh/rw for views 1/2. The viewport rectangle comes from
`0x6c482c`; `sw=trunc(width/128)+1`, `sh=trunc(height/64)`. Each supplied corner
direction pair `dx/dy` at `+0x6121+i*8/+0x6125+i*8` yields
`X=trunc((A+B)/2)-2*dx*sw-B`,
`Y=trunc((A+B)/2)-1-dy*sh+(dy<0)`. Four horizontal and four vertical writes
form each corner, with a repeated anchor, using RGB565 `0x400` or RGB555 `0x200`.
Native results return the selected origin; the adapter must preserve the original
origin writes. The service does not generate camera direction vectors, draw the
outer border, terrain or compose the subsequent marker list.

## Admission and validation

Native bounds, closed-input checks and complete preflight before any mutation
are intentional safety policy. Every addressed word must lie inside the logical
plane; row padding is preserved. Invalid planes/views, palette indices, source
domains, excessive creature lists, camera directions/viewports and unsupported
outer-border requests refuse atomically. Offscreen primitives are refused,
not silently clipped. Real allocation/lock/recovery, arbitrary source geometry,
caller flags/register preservation, live wire integration and bypass remain gaps.

The pixel fixtures map the pinned No-CD PE privately, verify entry/adapter bytes,
execute original marker routines and four complete outline entry paths within
the selected empty-list/border-skipped domain, and check source/object mutations.
Inputs are generated independent colours, visibility and view data. Outputs
include the complete destination allocation, padding and normalized return/state
results. Both normal and ASan/UBSan native fixtures are compared independently
with that original output. The original-file manifest is checked before/after.

```sh
python3 tools/draft-coverage-claims.py \
  --behavior RS.minimap-cell-marker --behavior RS.minimap-creature-markers \
  --behavior RS.minimap-camera-corners --output working/tests/overlay-claims.json
python3 tools/test-minimap-overlays.py --claims working/tests/overlay-claims.json
```

The bounded corpus covers all small 3x2 source/centre combinations for normal and
ring cell markers, selected negative/far coordinates and all nine palette slots,
both emphasis/flash gates, odd/asymmetric grids/extents and centre boundaries,
empty/visible/hidden/mixed/overlapping creature lists under both fog settings,
all four outline entries and viewport division boundaries in both word formats.
Confidence/status promotions require the new source/scope-bound execution record;
the original terrain evidence and previous live producer results retain their
historical hashes and separate scopes.

## Recorded comparison, 2026-10-09

[Current range-linked comparison record](native-minimap-overlays-comparison-range-20261009.json)
records 4,752 exact cases: 3,744 cell-marker, 768 creature-marker and 240
camera-corner cases, with 53,312,088 destination words and 4,752 return/state
triplets compared. All four orientations and both RGB565/RGB555 formats match
the isolated original in normal and ASan/UBSan executions. All 16 native atomic
refusal checks pass. Original source storage and expected object mutations are
checked; source hashes remain stable and both original-manifest checks pass.

This establishes bounded offline pixel and selected result/state equivalence.
Integration and live replacement remain none. Outer borders, nonempty subsequent
outline marker lists, camera direction generation, simulation visibility, real
locks and general caller machine state remain unvalidated. The original terrain
raster is a private signature-checked no-op during these outline comparisons;
its independent terrain comparison is preserved without changing its sources.

The binary inventory does not own the fourth outline entry `0x5532f0`.
Disassembly identifies its entry through the return at `0x5536bb` (exclusive
range end `0x5536bc`); the explicit recovered-range exception is supported by
a hash-pinned original execution with checked entry bytes `83ec28535556`.
This preserves the inventory gap rather than claiming a Ghidra function export.

The earlier comparison record is retained as historical evidence. After adding
the explicit fourth-entry range link and report anchors, a new prospective
declaration and complete rerun repeat all 4,752 exact cases and sanitizer/refusal
checks; both manifest checks pass. Native pixel implementation sources are unchanged.
