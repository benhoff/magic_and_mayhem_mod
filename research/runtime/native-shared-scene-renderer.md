# Shared native scene rendering

Step 2 connects the sprite and world previews to the native resource manager
and one persistent GPU scene service. Recovered animation, projection, terrain
submission, exact depth sorting and visibility remain in application/reconstruction
adapters. The service accepts the resulting ordered resource/frame/anchor display
data, optional coverage clipping and owned colour tables; it runs no simulation.

The service is a native policy, with high confidence within the tested bounded
scope. It retains background/canvas/cache ownership, validates the entire visible
list before clearing a completed frame, and refuses partial-frame presentation
after GPU execution failures. Draw records own colour tables and use semantic
resource IDs; they contain no legacy pointers or C++ objects shared across IPC.
See [the service contract](../../renderer/scenes/README.md).

The world adapter binds immutable checkpoint ANI bytes with separate 8 MiB
encoded recipe accounting. Both adapters keep their referenced decoded resources
resident for their lifetime. The resource manager header now names its asset
includes explicitly, preventing the reconstruction terrain catalog header from
shadowing the native asset catalog when these libraries are composed.

## Reproduction and evidence

```sh
python3 tools/test-native-scene-service.py
python3 tools/test-sprite-scene.py
python3 tools/test-animation-layers.py
python3 tools/test-world-scene.py \
  working/build/world-scene/mnm-world-scene-preview \
  working/build/world-scene/world/mnm-world-sandbox
python3 tools/test-native-scene-window.py \
  working/build/world-scene working/tests/native-scene-window/NEW_DIRECTORY
```

The first runner configures/builds the affected targets and records source/binary
hashes with seven synthetic CTests. Independent complete pixels cover ordering,
origins, transparency, clipping, palette values, resident upload reuse,
admission preservation, execution failure/retry and constructor/destructor
ownership. Migrated adapter tests retain the existing ANI/queue, 72 world-frame
and picking expectations. Resource tests cover copied ANI input, reload,
mutually exclusive sources and exact/aggregate encoded capacity budgets.

Installed validation retains 195 sprite frames with 390 isolated-original
selection/event matches, 198 layered frames, and 144 world pixel comparisons
with 12 fresh-process continuations. Window tests drive actual Qt input, timer
playback, modal Save and fresh-window continuation in four views. Original
manifest verification surrounds each installed-artifact experiment.

The sprite/layer CPU presentation tables now explicitly use RGB565 bit
replication, independently of the renderer. Their former floor-scaled tables
predated the confirmed Surface2 conversion in
[the surface contract](surface-dib-ddraw.md); this corrects the oracle, without
changing rendering or overwriting historical evidence.

New immutable results are registered as `NR.scene-service.synthetic-20261006`
and `NR.scene-service.installed-20261006`. Historical resource and preview
results remain unchanged and can be stale after this migration. The new results
assert only the listed bounded native behavior, not renewed equivalence for
every contract sharing these adapter sources.

## Remaining boundaries

Original live scene snapshots and live rendering replacement remain outside
this milestone. The next step needs an observed, versioned display snapshot and
explicit original-identity-to-resource mapping; this service can consume the
mapped ordered display records. Complete original camera/world admission,
lighting, animated palettes, text/cursor policies, effect/action production,
context recovery and broader installed scene coverage remain independent gaps.
