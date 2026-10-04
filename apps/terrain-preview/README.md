# Native terrain tile preview

This offline Qt/OpenGL app reads installed `Terrain.ttd` and `Terrain.spr`
through the native asset API. The selected recovered producer supplies body and
up to two layer frames, queue keys/kinds, light values and visibility ownership.
No ANI fixture, Wine or running game is required.

```sh
cmake -S apps/terrain-preview -B working/build/terrain-preview -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/terrain-preview --target mnm-terrain-preview -j4
working/build/terrain-preview/mnm-terrain-preview --root working/game-clean \
  --definitions 5,9,13,17,21,25,29,33,37 --view 0
```

`--realm Realms/Celtic/Forest` selects the catalog/sprite pair. Supply 1..9
explicit definition IDs and view 0..3. `--visibility` applies the recovered
reverse coverage pass with resolved owner identities, including shared encoded
frame aliases. `--overlap` places all selected tiles at a common anchor.

The 512x256 canvas uses explicit preview layout: a three-column diamond, zero
world height/priority/light and initially clear owner flags. Layout is not a
reconstruction of the upstream world traversal, camera, map or light system.
Draws outside the bounded canvas are rejected. There is no frame ticking.
Pixels use unshaded embedded palettes; recovered shade values are reported but
original lighting/palette chains are not applied. Maximum uploads are 27 owned
frames (body plus two layers per tile), released before returning the result.

`--output working/tests/my-tile-preview` writes new `.565`, `.png`, `.json`
files and exits. Parent directory must exist; existing output files are refused.
JSON records tile/frame/role/key/kind/anchor/shade, owner flags, RGBA hash and
remaining surfaces. The widget only presents the completed image; asset and
reconstruction orchestration lives in the separate scene service.

```sh
python3 tools/test-terrain-submission.py
python3 tools/test-terrain-preview.py \
  working/build/terrain-preview/mnm-terrain-preview \
  working/tests/terrain-submission/run-<id>/report.json
```

The latter owns an isolated Xvfb display for its entire run and compares all
queue fields/owner flags plus complete RGB565 and RGBA images using independent
Python SPR decoding and original-generated queue/sort/visibility outputs.
See [producer evidence](../../research/runtime/terrain-submission.md),
[TTD format](../../research/formats/ttd-native-loading.md) and
[native validation](../../research/runtime/native-terrain-preview.md).

## Installed MAP slice

```sh
working/build/terrain-preview/mnm-terrain-preview --root working/game-clean \
  --map 'Realms\Celtic\Forest\CFsec01.map' --region 8,7,1,3,3 --view 0
```

`--map` replaces `--definitions`; select the matching TTD/SPR pair with `--realm`.
`--region x,y,layer,width,height` selects one in-bounds 1..3 by 1..3 plane slice.
Its default is `0,0,0,3,3`. Definition IDs, absolute column/row/layer and initial
flag words come from the owned MAP reader. Layer supplies recovered queue height;
anchors still use the local preview diamond/common-anchor layout. This does not
select the top surface or recover camera placement. The app does not resolve
object references, normalize post-load flags, create entities or shade pixels.
JSON includes map dimensions, region and original tile inputs for review.

```sh
python3 tools/test-map-terrain-preview.py working/build/terrain-preview/mnm-terrain-preview \
  --sanitized working/build/terrain-preview-sanitized/mnm-terrain-preview
```

The comparison uses installed regions at layers 0, 1 and 6, including empty cells,
original terrain queues and visibility decisions, independent complete image
composition and explicit region rejection checks. See
[MAP loading](../../research/formats/map-native-loading.md) and
[map slice validation](../../research/runtime/native-map-terrain-preview.md).
