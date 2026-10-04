# Native installed MAP terrain slice preview

The Qt terrain preview now accepts an installed version-6 MAP and an explicit
1..3 by 1..3 slice from one vertical layer. Native AssetStore resolves Windows
separators and case-insensitive filenames, the [owned MAP reader](map-grid-loading.md)
decodes the grid, and the application service passes actual definition IDs,
absolute X/Y/layer coordinates and initial flag WORDs to the selected
[ordinary terrain producer](terrain-submission.md). TTD/SPR loading, original
queue ordering, optional reconstructed visibility and native OpenGL presentation
follow the existing terrain path. The Qt widget presents the completed image.

Example, after building `apps/terrain-preview`:

```bash
working/build/map-preview-frozen/mnm-terrain-preview \
  --root working/game-clean --map 'Realms\Celtic\Forest\CFsec01.map' \
  --region 8,7,1,3,3 --view 0
```

The caller selects the companion realm catalog with `--realm` when needed.
`--map` and explicitly supplied `--definitions` are mutually exclusive.
`--region` requires `--map`; malformed, zero-sized, oversized and out-of-grid
regions fail before output. Output JSON records map dimensions, requested slice,
input cells, queue and final owner flags alongside the image hashes.

## Executed evidence

[Normal and ASan/UBSan comparison report](native-map-terrain-preview.json)
records seven installed slices from CFsec01, CFsec02 and CFsec38, using layers
0, 1 and 6, all four orientations and visibility disabled/enabled. There are
56 cases and 112 complete native images across the two builds. These include
absolute coordinates away from zero, raw flags and empty/body-less definitions.
Mixed-case Windows-style requests exercise native resolution. Six invalid
region requests also fail before output.

The reference maps unchanged functions from pinned No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`
into a private PE32 mapping: ordinary producer `0x004f8960`, queue sorter
`0x004fff60`, and optional reverse visibility pass `0x005015f0`.
Its cells come from independently Python-decoded MAP bytes; native cells come
from the owned reader. Original and native results agree for definition/coordinate/
flag inputs, every queued frame, anchor, key, kind and shade, and final owner
flag WORDs. Independent Python SPR decoding and composition using those
original queue orders matches every RGB565 pixel and presented RGBA hash.
Native surface count returns to zero after every frame.

The chosen installed slices produce no hidden draws; agreement with visibility
on does not add hidden-decision coverage. The earlier [TTD preview fixtures](native-terrain-preview.md)
cover 28 hidden decisions per build. The Celtic Forest catalog has no first/
second layer frames, so these MAP cases exercise body draws; synthetic producer
comparisons cover those other branches separately.

The reproducible command is:

```bash
python3 tools/test-map-terrain-preview.py NORMAL --sanitized SANITIZED
```

This run used a retained source snapshot to isolate concurrent repository work:
`working/tests/map-preview-source/run-1j862uu9/`. Build the snapshot's
`apps/terrain-preview` normally and with `-fsanitize=address,undefined
-fno-omit-frame-pointer`, then pass the snapshot to `--source-root` to reproduce
that exact source set. Source, input, helper and binary hashes are recorded.
Run artifacts: `working/tests/map-terrain-preview/run-azoagx9u/`.
Original manifests pass before and after. No original game session, Wine or
runtime patch is involved. LeakSanitizer is disabled for managed execution;
prebuilt Qt and driver libraries are not instrumented.

## Bounds and confidence

Confidence is high for owned installed slice selection, the selected ordinary
terrain queue and these unshaded complete images. To isolate the ordinary
producer, the original reference clears cell object references to 0xffff and
rejects fixture cells with creature-presence bit 0x2000. Native MAP loading
preserves all references, but the preview does not resolve or instantiate them.
It uses raw on-disk flag WORDs without original runtime post-load normalization.

Screen anchors are an explicit local diamond/common-anchor preview policy;
absolute map coordinates and layer index feed depth calculation. Base priority
and light are zero, palette colors are embedded and unshaded, and out-of-canvas
draws are rejected. This does not recover original camera projection or clipping.
The preview deliberately selects one plane and does not choose a highest surface.
Full world traversal/admission, camera/priority production, lighting/palette
chains, water/overlays, objects/creatures and live integration remain separate.
