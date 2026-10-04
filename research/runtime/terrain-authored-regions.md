# Authored region recipes to native terrain scenes

Selected No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The native preview now loads an installed region recipe, selects its authored
MAP files and matching TTD/SPR pair, expands multi-block sections, assembles a
mixed-height terrain grid, initializes geometry once, and renders it.
Supported installed recipes in this milestone are Celtic REGION6 (The Misty
Marshes) and REGION9 (Tutorial Island). Both specify complete fixed placement
without Random candidates or wildcard fields.

## Recovered placement contract

`0052d240` invokes configuration loading, then runs up to ten generation
attempts. Each attempt clears descriptor placement counts and 25 grid slots,
runs Specific placement through `0052f3f0`, and calls `00532030` for constraint
selection of empty slots. The seed advances by 500 after each attempt,
including the successful attempt. This full generation policy is not implemented.

The selected original placement data uses 50 descriptors at receiver +2c with
63-byte stride, and a 5x5 grid at +39b4 with 37-byte stride. Grid addressing is
`column*5 + row`. A populated grid record contains a presence byte and DWORDs
for section ID, descriptor index, rotation, source block X/Y and four edge
labels. These layouts are fixture contracts; native runtime objects do not
share these addresses or packed host structures.

`0052edf0` constructs a descriptor for one source block and updates receiver
+20 to the maximum source layer count. Header block counts determine the
section side by source width/block-columns and height/block-rows in the
configuration caller. The unchanged descriptor builder, selected edge
admission `005300a0`, and placement `00531b20` are executed on installed authored
fixtures. Every requested placement in both supported regions passes original
edge admission without fallback. Descriptor reads and placement helpers make
no Win32 calls on these valid selected paths.

Rotation is around **source block (0,0)**, not a rotated bounding box origin.
For source block `(x,y)`, placement relative to the configured anchor is:

| Rotation | Relative column | Relative row |
|---|---|---|
| 0 | x | y |
| 1 | -y | x |
| 2 | -x | -y |
| 3 | y | -x |

Columns and rows wrap by region block dimensions. Original neighbor helper
`005318e0` supplies this wrapping; `00531980` records the selected block and
increments its placement count. `00531b20` walks internal connectors for
multi-block sections. Synthetic tests exercise its unchanged implementation
and all four rotations, with native geometric placement producing the same
section/descriptor/rotation/source-block assignments.

The configuration caller allocates terrain with the maximum selected source
layer count. A shorter section supplies only its source layers; untouched upper
cells retain definition zero, references ffff and clear flags. Selected
allocation field setup is visible in `004ed8f0`; complete allocation/grid/camera
initialization remains separate. This milestone models the owned terrain
storage fields needed for copy and geometry, rather than that full function.

## Native implementation and policy boundaries

`assets/region_recipe` owns format decoding with no reconstruction or widget
dependency. `reconstruction/rendering/terrain_region` reads owned recipes and
MAP storage, validates block metadata/side agreement, expands fixed placements,
requires complete non-overlapping coverage, and returns an owned plan/map.
Source selections and returned terrain use ordinary C++ storage.
`apps/terrain-preview/region_loader` handles file orchestration and provenance;
Qt presentation still receives the resulting scene image.

The fixed planner accepts source blocks of 1x1, 2x1, 1x2 or 2x2. Region dimensions
are 1..5 blocks, tile dimensions at most 128, layers 1..32. Source storage is
bounded to 2,097,152 cells. Block placement may wrap; individual square copies
remain in bounds. Sources correspond in order to Specific entries, and assets
remain unchanged. MAP metadata for generated grids is zero; it does not invent
serialized region metadata or entities.

The original explicit-grid preview still requires equal source layer counts.
The authored-region service handles differing layer counts through a separate
owned map initialized to empty cells, then calls the already validated section
copy operation for each source block. It clears source object bit 8 and all
references as the explicit ordinary-terrain projection, recording whole-source
counts. Selected classification and geometry then run once over the completed
grid, including cross-section neighbors and toroidal map-edge admission.

Strict refusal of Random/wildcard recipes, overlap, holes and malformed
metadata is native preview policy. Fixed rotations are used as authored;
general edge admission, alternative rotation correction and original error
recovery are not implemented in the native planner. Original admission is
confirmed separately for the two shipped supported recipes. Custom complete
fixed recipes are accepted as explicit preview inputs without claiming original
generator equivalence.

```sh
working/build/terrain-region-frozen/mnm-terrain-preview \
  --root working/game-clean --region-config 'Realms\Celtic\Celtic.cfg' \
  --region-id 6 --world --initialize-terrain --view 0 --visibility \
  --output working/tests/authored-example
```

`--region-config` requires numeric `--region-id`, world mode and initialization;
it excludes explicit map/grid/section/realm options. SpritePath selects TTD/SPR
separately from the MAP Path. JSON records config/id/name, sprite directory,
selected block requests and source projection counts under `map.recipe`.
Both installed examples produce an 80x80 grid; layer counts follow their source
headers. Camera controls and four view orientations retain their existing
contracts. Water, lighting, MPS/EVT/NOD placement, object rotation and live
simulation are not created by this ordinary terrain pipeline.

## Evidence and validation

The wrapper guards original files before/after, pins the executable hash,
records input/source hashes and compares all 40 owned recipes to independently
parsed installed CFG text in normal and ASan/UBSan builds. Original CFG loading
is not executed. Synthetic original placement cases cover grid sizes 2..5,
every anchor, block shapes 1x1/2x1/1x2/2x2 and all four rotations.

Installed descriptor/admission/placement fixtures use real MAP headers and
Specific tuples for both supported authored recipes. Unique internal connector
labels start at a fixture threshold above the installed outer labels; their
numeric identities are fixture configuration, not a reconstruction of the
original caller's edge-threshold calculation. Source-block grid assignments,
source layer maxima and admission success are checked against the original.

For scene checks, independent Python assembles selected MAP cells and geometry;
unchanged original traversal/producer/sort/visibility supplies queues/owners.
Native selected block provenance, visited cell fields, complete RGB565 images,
RGBA hashes and surface release are compared across four views, visibility
on/off and both builds. Pixels use independent unshaded SPR composition;
original whole-scene rendering is not claimed.

```sh
xvfb-run -a python3 tools/test-terrain-region.py \
  --preview NORMAL --sanitized SANITIZED \
  --inspect NORMAL_RECIPE_INSPECT --inspect-sanitized SANITIZED_RECIPE_INSPECT
```

Validation passed: 40 installed recipes in both builds; 3,136 synthetic
placement cases/46,656 blocks and two installed descriptor/admission/placement
fixtures/32 blocks; 32 complete authored-region scene comparisons; 12 CLI
rejection checks; and eight unit tests in each normal and ASan/UBSan build.
An additional 32 existing single-MAP/explicit-grid frames retained identical
JSON and RGB565 output across four views, visibility on/off and both builds.
Original manifest verification passed before and after each experiment.

Counts and provenance are recorded in
[the companion report](terrain-authored-regions.json). Random selection,
its RNG stream and retries, complete descriptor/catalog initialization,
rotation fallback and entity/pathfinding placement remain separate milestones.
