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

### Configured ground Redcap (NS09)

Append type `10` to the map exporter to use installed creature CFG dimensions,
acceleration, swimming admission and ANI-derived movement samples. Append start
XYZ, goal XYZ and a bounded tick count to create an owned checkpoint directly:

```bash
cmake -S apps/world-scene -B working/build/world-creature-navigation
cmake --build working/build/world-creature-navigation -j4
./tools/original-manifest.sh verify
working/build/world-creature-navigation/mnm-map-navigation-export \
  working/game-clean Realms/Celtic/Plains/CPsec01.map Realms/Celtic/Plains \
  0 0 8 8 working/redcap-navigation 10 1 1 3 4 6 3 0
working/build/world-creature-navigation/mnm-world-scene-preview \
  --checkpoint working/redcap-navigation.mnms --root working/game-clean \
  --realm Realms/Celtic/Plains --terrain-map working/redcap-navigation.geometry \
  --ticks 133 --frames 1 --output working/redcap-arrival
./tools/original-manifest.sh verify
```

All output paths must be unused. Configured creation selects the normal installed
Redcap ANI and base-0 ground programs internally; checkpoint ANI bytes are owned.
The report includes profile fields, samples, generator maximum and exact CFG,
ANI and SPR hashes. Without type 10 the prior synthetic exporter is unchanged.

This path currently admits one ordinary ground Redcap, one cell wide and three
layers tall for the installed CFG. Other types and flying/swimming action
profiles reject. Camera convention, dynamic occupancy, runtime objects, the
remaining original creature fields and live scheduling remain separate.
`GroundSpeed` is recorded; actual pacing uses the recovered ANI samples and
scalar/setup contracts. The real ANI event boundary has a zero displacement
sample, now compared against the original controller/action.

Run `python3 tools/test-creature-navigation.py working/build/world-creature-navigation`
for isolated-original profile/motion comparisons and configured installed-map
pixels/continuation. [Scope and evidence](../../research/runtime/native-creature-profile.md).

## Multiple moving creatures (NS12)

The preview accepts NS11 multi-movement checkpoints and composes up to 32
same-profile terrain-motion bodies into the shared terrain depth queue. Each body
reads its own saved ANI display and fine position; exported draws and actor rows
carry slot/generation identity. Stationary and cleaned entities are not presented.

Create a two-actor checkpoint with `mnm-world-sandbox move-pair-terrain-ani MAP
ANI BASE OUTPUT SX1 SY1 SZ1 TX1 TY1 TZ1 SX2 SY2 SZ2 TX2 TY2 TZ2 TICKS`, then
use the existing preview options. The Step and Save controls continue from the
owned multi-creature session.

`python3 tools/test-multi-world-scene.py SCENE SANDBOX NEW_OUTPUT` checks two
actors across four views, CPU/OpenGL pixels, per-actor positions/identity and
exact split/fresh-process presentation/checkpoints. Installed sprite reads are
bracketed by original-manifest verification. [Scope and accepted evidence](../../research/runtime/native-multi-world-scene.md).

## Select and order creatures (NS13)

Choose a creature, enter a target X/Y/Z cell, then Queue move. The order applies
when Step runs. The selector shows each creature's logical cell and movement
action; the view outlines its displayed body. Save checkpoint preserves pending
orders even before Step. Selection is transient and starts empty when reopening
a checkpoint. Cleanup or slot reuse clears selection.

Target fields survive ordinary refreshes and reset to the new creature's cell when
selection changes. Navigation still decides reachability and occupancy outcomes.
These are explicit diagnostic controls; mouse picking and automatic playback remain
open. [Contract and evidence](../../research/runtime/native-scene-orders.md).
