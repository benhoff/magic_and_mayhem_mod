# Native movement scene

A bounded Qt/OpenGL inspection app for the NS06 terrain-motion simulation.
It restores a native checkpoint and frozen navigation, reads the checkpoint's
owned ANI controller display, and interleaves terrain and creature draws using
the recovered signed sprite queue. Step advances the simulation once; rendering
never advances animation. Save checkpoint publishes a new native checkpoint.

The default mode is an explicit **diagnostic terrain fixture**. The installed-map
crop mode below presents matched ordinary geometry/navigation with declared
projection and creature-profile limits. Neither loads a complete game world.
A selected TTD body is repeated over 1..64 frozen-grid coordinates. Each visual
surface uses the ordinary frozen terrain height at a caller-selected standing
layer. Slopes and vertical travel can therefore be inspected against a selected
layer; the app does not infer neighboring floors, walls or runtime entities.
Cell-centred isometric projection and the fixed viewport are preview policy.
Four views rotate that projection; ANI sequence/facing remains the checkpoint's
numeric selection, without inferring camera-relative action groups. SPR origins
and ANI body offsets use existing recovered contracts. Terrain uses only the
selected body, its embedded palette, and no visibility or lighting pass.

Build and inspect a synthetic terrace with installed sprite assets:

```bash
cmake -S apps/world-scene -B working/build/world-scene -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/world-scene -j4
./tools/original-manifest.sh verify
python3 tools/create-movement-fixture.py working/scene-terrace.bin --terrain-profile terrace
working/build/world-scene/world/mnm-world-sandbox move-terrain-ani \
  working/scene-terrace.bin working/game-clean/Creatures/redcap.ani 0 \
  working/scene-start.mnms 1 1 1 5 1 1 2
working/build/world-scene/mnm-world-scene-preview \
  --checkpoint working/scene-start.mnms --root working/game-clean \
  --sprite Creatures/RedCap.spr --realm Realms/Celtic/Plains \
  --definition 1 --region 0,0,1,8,4
./tools/original-manifest.sh verify
```

ANI base 0 and paired SPR are explicit caller selections, not recovered action
configuration. The movement adapter rejects unsupported programs. Before an ANI
controller displays a record, the actor has no draw; completed segment displays
remain visible at boundaries and arrival. The viewport clips sprites safely.
The scene accepts one ordinary creature. The default diagnostic mode has at
most 64 visual tiles; matched map crops retain at most 4,096 cells.

Use `--view 0..3`, `--ticks 0..4096`, or
`--output working/scene-frame --frames 12` for export. Export produces numbered
PNG and little-endian RGB565 frames, a JSON queue/tick/fine-position report, and
the last frame's `.mnms` checkpoint. All outputs require new filenames.
Subsequent exports can restore that checkpoint; the first resumed frame is at
the saved tick unless `--ticks 1` is specified.

Validation:

```bash
ctest --test-dir working/build/world-scene --output-on-failure
python3 tools/test-world-scene.py \
  working/build/world-scene/mnm-world-scene-preview \
  working/build/world-scene/world/mnm-world-sandbox
```

Tests require a local Xvfb/OpenGL display. The installed-asset validation script
verifies the original manifest before and after, exports three controlled
terrain profiles in four views, compares full images with an independent SPR
pixel decoder, and checks save/restart frame and checkpoint equality. It uses
a synthetic ANI program with explicit frame IDs, separate from installed ANI
behavior validation. See [NS07 evidence](../../research/runtime/native-world-scene.md).

## Installed-map crops

`mnm-map-navigation-export` creates ordinary terrain geometry and matching frozen
navigation from an installed MAP/TTD pair. It derives geometry on the full source,
then crops all layers and seals the outer XY ring. Its creature profile is an
explicit synthetic one-cell test profile. Runtime objects/references are omitted.
The JSON report lists supported coordinates; not every pair is necessarily
reachable. Read [NS08 scope](../../research/runtime/native-map-navigation.md).

```bash
./tools/original-manifest.sh verify
working/build/world-scene/mnm-map-navigation-export \
  working/game-clean Realms/Celtic/Plains/CPsec01.map Realms/Celtic/Plains \
  0 0 8 8 working/plains-crop
working/build/world-scene/world/mnm-world-sandbox move-terrain-ani \
  working/plains-crop.frozen working/game-clean/Creatures/redcap.ani 0 \
  working/plains-start.mnms 1 1 3 4 6 3 2
working/build/world-scene/mnm-world-scene-preview \
  --checkpoint working/plains-start.mnms --root working/game-clean \
  --realm Realms/Celtic/Plains --sprite Creatures/RedCap.spr \
  --terrain-map working/plains-crop.geometry
./tools/original-manifest.sh verify
```

The `.geometry` file is an owned decoded MAP payload. The scene requires exact
cell, terrain-catalog and dimension agreement with the checkpoint's frozen input;
wrong realm/catalog or changed geometry is refused before output or movement.
`--terrain-map` excludes the synthetic `--region` and `--definition` options.
Native diagnostic projection and unshaded palettes remain; lighting is a separate
workstream. Crop XY bounds are 4..16, include at least two layers, and retain at
most 4,096 cells. These limits do not change the diagnostic fixture's 64 tiles.

Run the installed-map validation separately from synthetic CTests:

```bash
python3 tools/test-map-navigation.py working/build/world-scene
```

It uses local Xvfb and isolated hash-checked original predicates, verifies the
immutable-input manifest before/after, and retains new evidence under
`working/tests/map-navigation/`. It compares geometry bytes, predicate decisions,
complete pixels, route arrival, mismatch refusal and fresh-process continuation.
