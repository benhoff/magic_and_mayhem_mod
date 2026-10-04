# Orientation-zero native terrain scene production

This is the first world-scene milestone: owned MAP cells selected by recovered
camera traversal, ordinary terrain submission, original queue ordering and
optional SPR visibility, followed by native clipped OpenGL presentation.
It replaces the nine-tile local layout with a camera-selected multi-layer
terrain scene. It remains offline and does not implement a complete game scene.

Pinned No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
No executable instructions are changed; selected original functions execute in
a private PE32 mapping without Win32/import calls or a game process.

## Recovered contract

Orientation-zero traversal is `0x004f83f0..0x004f8952`; it calls camera origin
`0x004f8230` and ordinary producer `0x004f8960`. Packed receiver fields are
annotated host inputs, never native object layouts or stable live pointers.

| Receiver field | Selected meaning |
| --- | --- |
| +0x21/+0x25 | Grid object and dimension header pointers |
| +0x11/+0x15 | Base screen origin |
| +0x31 | Orientation |
| +0x39/+0x3d | Camera column/row |
| +0x41 | Vertical origin term |
| +0x45/+0x49/+0x4d | Origin corrections |
| +0x51/+0x55 | Signed halves added to origin |
| +0x59/+0x5d | Square traversal span and starting diagonal offset |
| +0x61/+0x6d | Layer cut and admission mode |
| +0x79/+0x7d/+0x81/+0x85 | Current cell pointer, column, row and layer |
| +0x89/+0x8d | Current screen anchor |
| +0x9d/+0xa1/+0xa5 | Accumulated priority and X/Y wrap adjustments |

For orientation zero, origin is:

```
Ox = f11 + trunc(f51/2) - f45 + f49
Oy = f15 + 16*(f41 - 2*diagonal) + trunc(f55/2) + f4d
     - (uint32(f45+f49) >> 1)
```

The final half is a logical shift, including negative sums. Selected evidence
agrees with the earlier [audio camera origin](audio-camera-projection.md);
the rendering model stays separate from audio services.

Each layer starts at camera column/row minus the diagonal, normalized by one
map extent when negative. Traverse square rows and columns in original order.
Row offset r and column offset c yield screen anchors
`Ox+32*(c-r), Oy+16*(c+r-layer)`. Wrap cell coordinates, adding +0xa1 when X
crosses its extent and +0xa5 when Y crosses its extent. Initial negative-start
normalization adds no priority. Row-reset adjustments remove X wraps from the
previous row. The native bounded domain admits at most one wrap per axis.

The layer loop covers the map layer count. Mode zero skips layers strictly
above the cut: the cut is inclusive here. Before producer admission, flag
WORD +10 bit 0x4000 always skips the cell. Flag WORD +8 bit 0x0080 skips cells
except on the grid's special layer at +0x7f0. These are traversal gates, separate
from the producer's concealment/player/body/layer decisions.

Viewport admission is not a per-sprite bounding-box test. Each row first skips
whole steps against left-64 and top-48, using floor divisions by 32 and 16.
It rejects rows whose adjusted starting anchor reaches right/bottom, then limits
the submitted run with `(right-X+32)/32` and `(bottom-Y+16)/16`. The latter
may admit an anchor exactly on a far boundary. Native sprite clipping is a
subsequent presentation policy, not a reconstruction of original draw clipping.

The other traversal exports are `0x004fbe00`, `0x004fc3f0`, `0x004fc930`.
They are statically inspected only. Their layer loops use receiver +0x61 rather
than the orientation-zero header layer count; their clipping/run loops also
differ. The +1/-1 start in the inspected orientation-one block must not be
silently replaced with the diagonal field. Native world traversal rejects these
orientations until separate original comparisons establish their contracts.

## Native application integration

`reconstruction/rendering/terrain_traversal.*` emits owned visit records through
a read-only flag callback. The application service resolves those visits against
`MapAsset` and constructs terrain states with actual definition, coordinates,
raw flags, recovered anchors and priorities. Reconstruction contains no widgets,
Qt, asset pointers or process addresses. The widget presents the completed image.

```bash
cmake -S apps/terrain-preview -B working/build/terrain-world -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/terrain-world --target mnm-terrain-preview
working/build/terrain-world/mnm-terrain-preview --root working/game-clean \
  --map 'Realms\Celtic\Forest\CFsec01.map' --world \
  --camera 20,20,20,19 --pan 256,64 --visibility
```

`--camera` is column,row,span,cut-level; `--pan` is the base screen origin.
Defaults select map center, a span up to 20, the map layer count, and pan 256,64.
The application sets diagonal=span/2 and f41=2*diagonal. These are explicit native
camera configuration policies, not recovered camera initialization. The model
supports the independently supplied origin corrections and wrap priorities;
the application currently keeps those corrections/priorities zero.

Bounds: positive dimensions up to 128x128x32, camera within the map, span and
diagonal no larger than either XY extent, cut 1..layers and mode 0/1. Signed
arithmetic overflow and unsupported orientations reject. The 512x256 presentation
limits visits to 16,384 and keeps at most 16 resident sprite uploads; existing
renderer budgets also apply. Empty admitted scenes produce a background image.
The previous explicit-definition and one-plane slice modes remain available.

## Executed validation and confidence

[Machine-readable report](terrain-world-traversal.json) records 4,096 unchanged
original traversal/producer comparisons, matching 605,236 queue records and full
cell flag mutations. Those include 146,478 visits with nonzero wrap priority and
122,150 admitted anchors in the viewport margins. Synthetic cases vary map
size/layers, camera anchors, span/diagonal, signed origin corrections, odd halves,
viewport bounds, mode/cut, special layer and traversal flags. A sanitized native
fixture checks known wrapping, layer/flag gates and seven rejected domains.
Queue and visibility CTests also pass.

Installed comparisons use CFsec01, CFsec02 and CFsec38 with centered, boundary-
wrapping and panned cameras; full available layers and selected cuts; visibility
on/off; normal and ASan/UBSan builds. Each checks original traversal, producer,
sort and optional visibility queues, selected cell/owner states, independent
complete clipped RGB565 pixels and presented RGBA hashes. All surfaces are
released after each image. See the report for exact counts and source/input/
helper/binary hashes. There are 18 cases / 36 images, 3,692 hidden draws and
3,688 partially clipped draws across both builds. Artifacts:
`working/tests/world-terrain/run-zjvxrq88/`. The retained source snapshot
`working/tests/run-tmi25frt/` isolates concurrent work, using committed baseline
`154da823b5baa2a3505b0e424e2d6c87900df272` plus this chunk's code overlay.
Build its `apps/terrain-preview` and pass `--source-root` to reproduce the run.

```bash
python3 tools/test-world-terrain-preview.py NORMAL --sanitized SANITIZED
```

Original manifests pass before and after. LeakSanitizer is disabled for managed
execution; prebuilt Qt/driver libraries and the original PE are uninstrumented.
Confidence is high for the selected orientation-zero bounded traversal and
ordinary terrain queue. Pixel evidence validates native clipping/composition
against the independent oracle, not original whole-world draw output.

The installed oracle disables object references and the creature-presence branch,
restoring the ignored creature flag for owner comparisons. Native MAP bytes and
flags are preserved, but references are not instantiated. Raw MAP flags are used
without original post-load normalization. Celtic Forest has no first/second
terrain frames; their producer branches retain separate synthetic evidence.
Remaining work: three orientations and their exceptional boundaries, camera
initialization/updates, map initialization and surface/admission semantics,
lighting/palette chains, water/overlays/objects/creatures, picking and live routing.
