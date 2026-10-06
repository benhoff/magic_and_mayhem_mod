# Bounded native Forest/Redcap scene specification v1

`tools/load-bounded-scene.py` consumes a strict JSON object (at most 64 KiB).
The checked-in example is `tests/fixtures/forest-redcap-scene.json`. Paths and
SHA-256 values identify installed inputs, not an original saved game. This is an
intentional native ordinary-terrain projection; it does not claim world-loader
equivalence. Unknown/duplicate keys, missing fields, unsupported schema/policy,
noninteger or out-of-range coordinates and resource substitutions refuse.

| Field | Accepted contract |
| --- | --- |
| `schema` | Integer 1 |
| `policy` | `ordinary-terrain-only`; explicit consent to the omissions below |
| `resources` | Exactly six installed request-to-lowercase-SHA-256 bindings in the example |
| `crop` | Source-local `[x,y,width,height]`; XY origin 0..127, extents 4..16 |
| `creature` | Exactly type 10, crop-local `start` and `goal` XYZ; interior XY and layer 1..31 |
| `presentation` | Exactly view 0..3, initial ticks 0..4096 and frames 1..64 |

The native exporter additionally validates actual map bounds, 2..32 source layers,
the 4,096-cell budget, CFG profile and ANI program/binding shape. Both endpoints
must be in its configured support/validity list. This is not a reachability proof;
the loader does not assert every admitted order arrives. Unsupported animation or
movement encountered during stepping refuses through the native driver.

Version 1 admits only `Realms/Celtic/Forest/CFsec01.map`, its `Terrain.ttd` and
`Terrain.spr`, `CFG/Encrypted/creature.cfg`, `Creatures/redcap.ani` and the ANI's
`Creatures/redcap.spr`. The installed resolver accepts a unique case-insensitive
match, including `RedCap.spr`, but refuses ambiguous names and symlink components.
Each resource is limited to 24 MiB. Other maps, realms, types, low-memory resource
variants, additional actors, scripts and bindings require a separately reviewed
schema extension. There is no automatic fallback or hash regeneration.

## Preservation and deliberate omissions

| Input | Preserved or derived | Omitted or substituted |
| --- | --- | --- |
| MAP terrain | Ordinary definitions, orientations and admitted flags across all source layers; full-source neighbor context before crop; derived surface/culling bits | Three reference WORDs replaced by `0xffff`; flags10 object/path bits `0x0008`/`0x2000` removed; some occluded definitions culled; source header metadata zeroed |
| Crop topology | Crop-local dimensions and canonical row/layer tables | Native sealed outer XY ring (`flags10 0x4000`) on every layer; ring omitted from drawing; full-map wraparound unavailable |
| TTD | Complete 356-byte records shared by navigation and presentation | No complete runtime object/entity loading or object collision reconstruction |
| Creature CFG | Normalized type 10 height, width, acceleration, swimming admission, CanFly, GroundSpeed and FlyingSpeed | Remaining creature/HTH stats, attacks, spells, faction and balance; no flying or swimming action implementation. Recorded speed fields do not replace ANI-driven pacing |
| ANI | Normal-memory Redcap selection, embedded sprite name, base-0 ground programs, derived samples/maximum; owned checkpoint ANI | Named idle/action selection, camera-relative directions, low-memory variants and unsupported event/action categories |
| SPR resources | Exact terrain and creature bytes; native decoded frames, origins and embedded palettes | Lighting/visibility integration, runtime attachments and other resource families |
| Runtime world | One explicitly specified native actor and queued move | Installed `.mps`, `.nod`, `.evt`, campaign state, triggers, scenery entities, pickups, resource nodes/economy, dynamic source occupancy and original scheduler |

The frozen navigation also retains the exporter's native scalar 720, slope 0.969
and boundary policy. Inert pointer tokens and zeroed unused runtime state are
adapter inputs, not recovered source-world state. The presentation uses a fixed
512x256 viewport, diagnostic isometric camera and unshaded palettes. Detailed
baseline evidence remains in [NS08](../runtime/native-map-navigation.md) and
[NS09](../runtime/native-creature-profile.md). Confidence is high within the
documented native scope; original whole-world/resource binding remains unproven.

## Loading and reproducibility

```bash
cmake -S apps/world-scene -B working/build/world-creature-navigation
cmake --build working/build/world-creature-navigation -j4
xvfb-run -a env QT_QPA_PLATFORM=xcb LIBGL_ALWAYS_SOFTWARE=1 \
  python3 tools/load-bounded-scene.py tests/fixtures/forest-redcap-scene.json \
  --root working/game-clean --build working/build/world-creature-navigation \
  --output working/forest-redcap-scene
```

The loader verifies the original manifest before and after consuming artifacts,
including native rejection paths. It admits all six hashes before creating its
new output directory, copies the verified owned bytes into `resources/`, and
uses that same staged root for export and presentation. It selects both sprite
requests explicitly. The existing presentation validator requires complete
geometry/cell/TTD agreement with frozen navigation before movement or output.
Staged resource hashes are checked before and after presentation, and executable
hashes are recorded and checked after execution. The local staging directory is
not an adversarial concurrent-writer security boundary.

`world.geometry`, `world.frozen`, `world.mnms`, numbered PNG/RGB565 frames and
`frame.mnms` retain the result. `result.json` is the success marker, published only
after both consumers and the final manifest check succeed. It records the exact
specification, resource/executable hashes, commands and output hashes. A failure
can leave diagnostic intermediate files without that marker; consumers must not
treat such a directory as a successfully loaded scene. Original output paths and
existing output directories refuse. Native checkpoints reference the retained
absolute staged frozen path, so they are resumable there but are not portable
bundles. Pixel/geometry reproducibility is independent of output directory;
checkpoint bytes can differ because that absolute path differs.

Run `python3 -B tests/test-bounded-scene.py` for synthetic refusal tests, and
`xvfb-run -a python3 -B tests/test-bounded-scene.py --installed
working/build/world-creature-navigation working/tests/bounded-scene-new` for
manifest-guarded installed integration. The latter checks repeated pixels and
geometry, independent SPR composition, the example's route arrival and refusal
of a changed terrain sprite hash before any output directory is created.
