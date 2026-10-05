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
By default, pixels use unshaded embedded palettes; recovered shade values are reported but
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
object references, normalize post-load flags, create entities or generate spatial lighting.
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

## Camera-selected terrain scene

```sh
working/build/terrain-preview/mnm-terrain-preview --root working/game-clean \
  --map 'Realms\Celtic\Forest\CFsec01.map' --world \
  --camera 20,20,20,19 --pan 256,64 --visibility
```

`--world` selects recovered four-orientation traversal across map layers, with
viewport admission, wrapping and actual camera-produced anchors. `--camera`
is column,row,span,cut-level; `--pan` supplies base screen origin. Omit them for
map center, span up to 20, all available layers and pan 256,64. Use `--view 0..3` to select orientation. World mode rejects
`--region` and `--overlap`. The native camera defaults are explicit
configuration policy. Sprite clipping keeps the canvas bounded and a 16-frame
upload cache keeps resource use bounded. This is ordinary terrain scene
production: map initialization, lighting, water and entities remain separate.

```sh
python3 tools/test-world-terrain-preview.py NORMAL --sanitized SANITIZED
```

Views 1..3 have an exclusive layer cut and retain distinct row clipping/reset
behavior. Repeated visits share physical cell ownership. Unsafe grid carries
are rejected before rendering; original runtime map initialization is still
separate. See [four-orientation traversal and validation](../../research/runtime/terrain-traversal-rotations.md).

Use `--recovered-camera` for recovered map binding, viewport and position setters,
plus repeatable `--scroll x,y` steps in screen-direction coordinates. It uses
map-minimum span and selects map center at height zero as preview policy. It
excludes `--camera` and `--pan`; `--scroll` requires this mode. Example:

```sh
working/build/terrain-camera-frozen/mnm-terrain-preview --root working/game-clean \
  --map Realms/Celtic/Forest/CFsec01.map --world --recovered-camera \
  --view 1 --scroll 19,-7 --scroll -91,41 --visibility
```

See [selected camera setters](../../research/runtime/terrain-camera-setters.md)
for original-code comparisons, replay validation and remaining lifecycle work.

`--world --initialize-terrain` prepares an owned ordinary-terrain map projection
before traversal: references/object branches are omitted, selected classification
flags are initialized and recovered edge/empty/enclosed-cell decisions are
applied. Combine it with `--recovered-camera` for map-bound camera setup. The
output records projection and geometry counts. Entities, lighting and the full
original map lifecycle remain separate. See
[geometry initialization](../../research/runtime/terrain-map-initialization.md).

```sh
xvfb-run -a env QT_QPA_PLATFORM=xcb LIBGL_ALWAYS_SOFTWARE=1 \
  ASAN_OPTIONS=detect_leaks=0 python3 tools/test-terrain-map.py \
  --source-root SOURCE_SNAPSHOT --preview NORMAL --sanitized SANITIZED
```

## Explicit assembled terrain region

```sh
working/build/terrain-sections-frozen/mnm-terrain-preview \
  --root working/game-clean --world --initialize-terrain --grid 2,2,20 \
  --section Realms/Celtic/Forest/CFsec01.map,0,0,0,0,0 \
  --section Realms/Celtic/Forest/CFsec01.map,20,0,1,0,1 \
  --section Realms/Celtic/Forest/CFsec01.map,0,20,0,1,2 \
  --section Realms/Celtic/Forest/CFsec01.map,20,20,1,1,3 \
  --camera 20,20,20,19 --view 0 --visibility
```

`--grid columns,rows,side` requires one `--section` per slot, each specifying
`path,sourceX,sourceY,column,row,rotation`. Sources share a layer count; square
crops are bounded by each MAP. Rotations 0..3 apply recovered definition and
flag transformations along with placement. Source assets remain unchanged.
Object flags/references are explicitly projected out before rotation; geometry
runs after assembly so section neighbors participate in surface admission.
JSON records selections and whole-source projection counts under `map.assembly`.
Grid dimensions are bounded to 128, layers to 32, and aggregate source storage
to 2,097,152 cells. Grid excludes `--map`, requires initialization/world mode,
and supports the existing explicit or recovered camera controls.

This is explicit terrain assembly. Random region selection, runtime entities,
lighting and live replacement remain separate. See
[copy/rotation evidence](../../research/runtime/terrain-section-assembly.md).

```sh
xvfb-run -a python3 tools/test-terrain-sections.py \
  --preview NORMAL --sanitized SANITIZED
```

## Installed authored region

```sh
working/build/terrain-region-frozen/mnm-terrain-preview \
  --root working/game-clean --region-config 'Realms\Celtic\Celtic.cfg' \
  --region-id 6 --world --initialize-terrain --view 0 --visibility
```

`--region-config` and `--region-id` select a complete authored recipe from the
installed realm CFG. Celtic regions 6 and 9 are supported installed examples.
The service selects MAP files from Path and TTD/SPR from SpritePath, expands
multi-block sections around their authored anchors with wrapping, and supports
mixed source layer counts. Empty layers above shorter sources remain empty.
Geometry initializes after the assembled grid is complete. JSON provenance is
under `map.recipe`; source assets remain unchanged.

This mode excludes `--map`, `--grid`, `--section` and `--realm`, and requires
world mode plus initialization. Recipes needing Random candidates or wildcard
placement/rotation are rejected. General edge correction, entity placement,
lighting and water remain separate. See
[authored-region evidence](../../research/runtime/terrain-authored-regions.md)
and [recipe syntax](../../research/formats/region-terrain-recipes.md).

## Generated recipe terrain

```sh
working/build/terrain-generated-scenes/mnm-terrain-preview \
  --root working/game-clean --region-config 'Realms\Greek\Greek.cfg' \
  --region-id 1 --generation-seed 123 --world --initialize-terrain \
  --view 2 --visibility
```

`--generation-seed` selects the recovered generator with an explicit unsigned
32-bit seed, including zero. It requires `--region-config`; omitting it preserves
the authored-only path above. The loader reads complete Specific then Random MAP
inputs, preserves repeated source-entry identities, performs up to ten recovered
attempts and assembles successful assignments into an owned ordinary terrain MAP.
Geometry runs after assembly. Source payloads are temporary and remain unchanged.
Source storage is bounded to 2,097,152 cells; the existing catalog/placement bounds
apply. Exhausted generation reports the attempt count and next seed and produces
no frame. Missing inputs are refused, including Medieval REGION0's absent Test
sections.

Output provenance includes the concrete source blocks and generation seed,
next seed, attempts, backtracks and typed warning notices under
`map.recipe.generation`. World output also includes `map.cells_sha256`, the hash
of all initialized 12-byte cell records, including cells outside the viewport.
The service has no QWidget dependency; the preview widget presents its image.

```sh
xvfb-run -a python3 tools/test-terrain-generated-scenes.py \
  --source-root SOURCE_SNAPSHOT --preview NORMAL --sanitized SANITIZED
```

This checks original selected assignments, independent whole-map copy/rotation,
original geometry and queue/visibility calls, complete RGB565/RGBA scene frames
and failure paths. Entities, lighting, water and live replacement remain separate.
See [generated scene evidence](../../research/runtime/native-generated-terrain-scenes.md).

For broader offline coverage across all shipped recipes at zero and wrapping
seeds, with corner cameras and explicit refusal accounting, run:

```sh
xvfb-run -a python3 tools/test-terrain-generated-coverage.py \
  --source-root SOURCE_SNAPSHOT --preview NORMAL --sanitized SANITIZED \
  --preview-report research/runtime/native-generated-terrain-scenes.json
```

The previews must match the preceding report's binary and source hashes. The
wrapper validates native cameras before invoking original traversal, records
unavailable/refused/exhausted cases separately and compares full initialized
maps and accepted scene frames. See
[expanded coverage](../../research/runtime/terrain-generated-coverage.md).

Baseline palette shading is now available explicitly:

```bash
mnm-terrain-preview --root working/game-clean --palette-shading --light -17 \
  --output working/shaded-terrain
```

This uses the recovered mode-0 palette builder with count 16, intensity and
saturation levels 1, and powers 2. `--light` accepts -127..127 and supplies a
controlled uniform fixture light to every submitted tile, including world and
generated scenes. Spatial light generation and effect-chain selection remain outstanding. The upload cache distinguishes
frames at different selected tables and retains owned textures. See
[palette shading evidence](../../research/runtime/terrain-palette-shading.md)
for original-binary comparisons and reproduction.

To use the installed global palette controls, add
`--lighting-config 'CFG\Encrypted\chaos.cfg'` to a `--palette-shading` request.
The native packed CFG loader reads `LightCurve`, `ColourFactor`, `LightPower`
and `ColourPower` from `GLOBAL_OPTIONS`, then applies the original bounds.
Output JSON records the effective controls. Missing fields preserve image
settings; malformed numeric fields or unavailable files are refused. This
uses count 16 unless `--preferences` is supplied, and controlled uniform
`--light`; spatial light production remains outstanding.
See [configured lighting evidence](../../research/runtime/terrain-lighting-config.md).

To use the installed terrain palette count, also add `--preferences 'CFG\prefs.cfg'`.
This plain CFG supplies `[VIDEO] TerrainLightLevels` (installed value 256).
Missing keys retain the fixture count; present values receive the original
2..256 clamp. The preview supports powers of two after admission and refuses
other counts. JSON records both the effective count and preference path.
See [terrain preference evidence](../../research/runtime/terrain-palette-preferences.md).
