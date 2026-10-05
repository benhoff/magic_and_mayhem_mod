# Native movement scene

A bounded Qt/OpenGL inspection app for the NS06 terrain-motion simulation.
It restores a native checkpoint and frozen navigation, reads the checkpoint's
owned ANI controller display, and interleaves terrain and creature draws using
the recovered signed sprite queue. Step advances the simulation once; rendering
never advances animation. Save checkpoint publishes a new native checkpoint.

This is an explicit **diagnostic terrain fixture**, not an installed MAP world.
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
  working/scene-terrace.bin working/game-clean/Creatures/redcap.ani 8 \
  working/scene-start.mnms 1 1 1 5 1 1 2
working/build/world-scene/mnm-world-scene-preview \
  --checkpoint working/scene-start.mnms --root working/game-clean \
  --sprite Creatures/RedCap.spr --realm Realms/Celtic/Plains \
  --definition 1 --region 0,0,1,8,4
./tools/original-manifest.sh verify
```

ANI base 8 and paired SPR are explicit caller selections, not recovered action
configuration. The movement adapter rejects unsupported programs. Before an ANI
controller displays a record, the actor has no draw; completed segment displays
remain visible at boundaries and arrival. The viewport clips sprites safely.
The scene accepts one ordinary creature, with at most 64 visual tiles.

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
