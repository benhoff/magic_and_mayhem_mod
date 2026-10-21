# Native scene session

`SceneSession` owns presentation state, installed resources, linked terrain,
lighting, animation bindings, visibility/depth ordering and a persistent GPU
canvas. Its source is an immutable semantic snapshot. It never steps a game
World or a movement controller.

Use `worldSceneSnapshot` with typed CombatState/CreatureEncounter health,
`movementSceneSnapshot` with its authoritative fine-position provider, or
`nativeSceneSnapshot` with an identified native command profile/asset registry.
Pass the result to `prepare`, then `render`. `recompose` redraws the accepted
snapshot after camera changes without a simulation/visual/light tick.
`presentGpu` retains native GPU output; `read` and `present` are explicit export
synchronization points. Frame plans are bound to their owning session, accepted
source epoch/serial, map revision and resource revisions.

Source revisions increase within an epoch. Source ticks are either unchanged or
advance by one, including unsigned wrap. A skipped tick requires a restored/new
epoch: inventing intermediate positions or lighting transitions is forbidden.
Imported ANI continuations remain authoritative. `SceneFrameOptions.visualTicks`
explicitly selects actors whose scene-owned channels may advance on an admitted
source tick. Duplicate actors, repeat clocks and ticks of exclusively imported
channels are refused. A new epoch restarts presentation ownership and the
immutable source map; `retire` also releases decoded resources and GPU cache
entries. Limits bound entities, channels, draw entries and CPU/GPU storage.

Native special, secondary, linked-effect, effect-camera and action-object
identities are explicit producer inputs. Maintenance auxiliary clocks are never
silently interpreted as overlays. Branch-specific effect camera metadata binds
to the current effect generation. Missing identified producers refuse at the
boundary. This supports the normalized contracts without claiming absent fields
exist in an older native continuation format.

`decodeSceneManifest` accepts version 1 JSON, at most 1 MiB, known fields and
bounded typed values. `assets` binds semantic `{kind,name}` identities to read-only
paths; `terrain` selects the SPR/TTD pair. `terrainAnimation` is a separate ANI
path because its output selects TTD records rather than sprite frames. `map` and
optional `mps` load installed geometry/placements. MPS activation remains an
explicit simulation decision; loading does not roll chances or spawn entities.
`types` supplies family/type/optional-owner profiles and ordered layers. Each
layer names a channel and resource, then an ANI `sequence` or static `frame`.
Optional attachment is `first` or `second`; priority, kind, lit and livingOnly
are explicit presentation inputs. Creature/object body channel is 0; effect body
channel is 3. Camera span/cut default to the bound map; view and an explicit cut
are preserved. `viewport` is `[width,height]`.

`lighting` accepts enabled/player/creatures/objects, packed `effectConfig` and
`terrainConfig`, and explicit objectTypes mappings. `palette` accepts modes,
ordinal, packed globalConfig, plain effectProfile and plain preferences paths.
These use the existing owned asset/config loaders. Missing lighting enablement
leaves lighting disabled; missing preference keys preserve native defaults.

Build this directory with CMake. The `native-scene-session` executable renders
one actual native checkpoint through the same session owner:

```sh
native-scene-session ASSET_ROOT MANIFEST.json WORLD.mnw OUTPUT.png
```

The manifest is a host configuration file; its resource paths resolve inside
ASSET_ROOT. WORLD.mnw is the native world snapshot with typed combat health in
its subsystem section. Original SAV files require their separate reconstruction
adapter. Existing terrain-motion snapshots need an authoritative fine-position
provider and use the C++ adapter instead of guessing height from cell coordinates.

Run `tools/test-native-scene-session.py --output NEW_OUTPUT --build-root NEW_BUILD`
for source/animation/light assertions and software OpenGL pixel comparisons.
Add `--installed-root PATH` for immutable installed MAP/MPS/ANI/TTD/config loading,
checkpoint creation and executable image agreement. `--sanitized` enables Clang
ASan/UBSan. `tools/test-native-scene-sources.py` runs only the CPU source services.
Wrap storage-producing runs with the repository retention guard. Installed runs
verify the original manifest before and after consumption. These native tests do
not establish original whole-scene equivalence, physical NVIDIA acceptance or
live replacement.
