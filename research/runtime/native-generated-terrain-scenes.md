# Native scenes from generated terrain recipes

Selected No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This milestone connects the [owned generated plans](terrain-generated-plans.md)
to complete installed MAP loading, ordinary assembly, geometry initialization
and bounded Qt/OpenGL terrain scene production. It is offline validation;
original whole-scene rendering and live replacement remain separate.

## Integration and ownership

`loadGeneratedTerrainRegion` in `apps/terrain-preview/region_loader.hpp/.cpp`
uses the native asset interface to read complete MAP payloads in Specific then
Random recipe order. Repeated section IDs retain their source-entry identities.
The source files are read-only and decoded buffers exist only for the call.
Successful generation feeds the existing mixed-height section-copy backend;
returned catalog, state, diagnostics, concrete plan, source paths and assembled
MAP own their storage. Failure retains the generation result and paths with no
assembled MAP. Missing/malformed inputs throw the existing strict native errors.
Aggregate source storage remains bounded to 2,097,152 cells.

The same private source-loading helper serves the authored path. Omitting the
new `--generation-seed` preview option preserves authored-only behavior. Supplying
an unsigned 32-bit seed, including zero, selects generated recipes; it requires
`--region-config`, region ID, world mode and ordinary terrain initialization.
The existing mode exclusions, camera configuration, bounded canvas and frame
cache apply. Geometry runs after assembly so section neighbors participate in
surface admission. Exhaustion reports the attempt count and next seed and exits
before geometry, rendering or output creation.

The widget still only presents a completed image. File loading and generation
are application services; reconstructed selection and placement remain outside
widgets and native asset services. This introduces no Wine dependency.

Output records the actual source blocks and seed/next-seed, completion, attempt
seeds, backtracks and typed warning notices under `map.recipe.generation`.
World-mode output also records `map.cells_sha256`, hashing every initialized
12-byte MAP cell in little-endian order, including cells outside the viewport.
This is diagnostic output; it does not change the MAP format.

## Reproduction and comparison boundaries

Build normal and ASan/UBSan versions of `apps/terrain-preview`, then run:

```sh
xvfb-run -a python3 tools/test-terrain-generated-scenes.py \
  --source-root SOURCE_SNAPSHOT --preview NORMAL --sanitized SANITIZED
```

The wrapper pins the executable hash, guards immutable originals before and
after, records source/input/helper hashes and supports a frozen source baseline.
Native requests deliberately use Windows separators and mixed-case CFG names.

`tests/terrain-generated-scene-reference.cpp` reuses the private PE/FS adapter
from the preceding generator corpus. It runs the unchanged original post-CFG
loop at `0052d269`, compares final state, and emits the original grid's selected
source entries, crop offsets and rotations. Source ownership is independently
expanded from source-header counts. The original CFG/file reader is bypassed;
its catalog inputs use the separately validated native catalog construction.
No executable instructions or original files are patched. A checked private
`MessageBoxA` import collects original notices.

Python independently assembles those original-selected crops, rotates definition
IDs/flags/cells, projects out object references and fills empty upper layers.
The independent geometry transcription supplies expected initialized cells.
The unchanged original geometry pass at `004ecf20` checks complete cell bytes via
`tests/terrain-map-reference.cpp`. The native preview's whole-map hash then checks
all cells, rather than only visible tiles. Source-entry provenance and whole-source
projection counts are also compared.

`tests/world-terrain-reference.cpp` supplies selected original traversal,
producer, depth-sort and visibility results for every view. Independent Python
SPR decoding/composition supplies expected RGB565 and RGBA pixels. Comparisons
cover complete queues, tile inputs, ownership flags, full frames and zero retained
OpenGL surfaces. This does not execute the original whole-scene renderer or its
lighting palette chain: pixels use the existing unshaded embedded-palette policy.

The installed corpus selects Celtic Forest REGION1, Greek Mountains REGION1 and
Medieval Dinas REGION16 with seeds 0 and 123. The first two exercise Random
selection; the latter exercises wildcard Specific placement without Random
entries. Celtic Swamp REGION6 without a generation seed checks the authored
regression path. All four views and visibility off/on run in both normal and
ASan/UBSan builds. Exact layout, cell, frame and rejection counts and frozen
provenance are recorded in [the companion report](native-generated-terrain-scenes.json).

Seven layouts / **130 concrete block assignments** matched, including six
generated layouts and one authored regression. Complete initialized cells for
all seven maps matched: **1,223,200 cells**. The original geometry helper also
passed its 2,048 synthetic fixtures / 387,962 cells. All **112 native frames**
(56 per build) matched queue, ownership and independent pixels, including 3,692
hidden draw records across both builds. Ten negative cases (five per build)
passed. Four focused recipe/MAP loading and generated/authored assembly CTests
passed in both normal and ASan/UBSan builds.

Celtic Forest uses source heights 23/24; Medieval Dinas and the authored Swamp
regression use heights 20/25. Greek Mountains and both Medieval layouts exercise
multi-block sources. Thus full-cell checks cover empty upper layers as well as
rotation and section-boundary geometry.

Negative cases cover absent config, negative/overflow seed, missing Medieval
REGION0 Test assets and an independently created complete one-source recipe
whose occurrence limit cannot fill its grid. Exhaustion retains the selected
next seed internally, reports ten failed attempts and creates no output files.
The eleven missing Test sections remain unavailable; no other realm files are
substituted. Address/undefined-behavior checks remain enabled; leak detection is
disabled under the execution sandbox.

Confidence is high for this bounded generated ordinary-terrain pipeline. It
extends prior component evidence and is not a claim that every installed recipe,
seed, complete scene producer or original file-error recovery path was exercised.
Full map lifecycle, entities, water, lighting, named animation producers and live
integration remain separate. Further offline work can broaden available recipes,
seeds and camera locations before selecting the next original scene producer.
