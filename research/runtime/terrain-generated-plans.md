# Generated terrain layouts and owned assembly plans

Selected No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This milestone connects complete available recipe/header catalogs to the recovered
ten-attempt generator and converts successful assignments into ordinary terrain
section-copy plans. It is offline evidence; installed generated scene pixels and
live replacement are subsequent milestones.

## Interface and ownership

`generateTerrainRegionPlan` in `reconstruction/rendering/terrain_generated.hpp/.cpp`
accepts an owned recipe, MAP inputs in Specific-then-Random source-entry order and
an explicit seed. Header-only inputs suffice for this stage. The returned value
owns its catalog, final generation state, mutated requests, attempt diagnostics,
next seed and optional concrete plan; it retains no source pointers or cells.
After ten unsuccessful attempts it returns the partial generation result with
no assembly plan. Missing or malformed inputs use the existing strict native
refusals, separate from original file/dialog recovery.

`planGeneratedTerrainRegion` also exposes conversion for an already generated
result. It verifies completion, placement-state integrity, dimensions, immutable
descriptor identity, source-entry spans and full destination coverage. Blocks
are emitted in destination row-first order. Source block coordinates are
multiplied by the common section side to become MAP cell offsets, while actual
selected rotation and source-entry identity are preserved. Repeated section
numbers therefore remain distinct source entries.

The result uses the existing `FixedRegionPlan` storage and
`assembleFixedTerrainRegion` copy backend; the authored-only planner's admission
rules do not apply to generated plans. With complete MAP payloads and the TTD
catalog, that backend produces an owned ordinary terrain MAP, including mixed
source heights. Existing reference/object projection remains an explicit native
policy. Geometry initialization must follow assembly. File orchestration,
scene creation and presentation remain outside this reconstruction service.

## Evidence and confidence

Reproduce with:

```sh
python3 tools/test-terrain-generated.py
```

The wrapper pins the executable, verifies all 2,927 immutable originals before
and after, independently parses three installed realm CFGs, decodes 564 unique
available referenced MAP headers and records source/input/helper hashes. It
supports `--source-root` for frozen source snapshots.

The 32-bit harness privately maps the PE and enters the unchanged post-CFG
attempt loop at `0052d269`, using the previously validated stack adapter and
private FS exception head. The checked `MessageBoxA` import is bound privately
to a warning collector; executable instructions and files remain unchanged.
It compares completion, the complete descriptor/request bank, grid assignments,
source coordinates, candidate and location tables, carry, final seed and ordered
warning notices. This builds on the separate validated
[catalog/threshold](terrain-region-catalog.md) and
[attempt orchestration](terrain-region-generation.md) milestones; original CFG
and file loading are bypassed.

For every successful plan, independent source-header block counts identify the
original grid descriptor's owning source entry. The comparison checks destination
order, source identity, MAP cell offsets, rotation, common side and maximum
layers. It does not use the catalog provenance table as its expected owner.

The corpus contains 2,048 deterministic synthetic multi-source fixtures with
repeated section IDs, varied block shapes, layer counts, grid sizes and occurrence
limits. It also tries all 40 installed recipes with seeds 0, 1, 123 and
`0xdeadbeef`. The 39 available recipes contribute 156 comparable cases, including
155 successful plans and one exhausted generation. Overall, **2,204 complete
original-loop state comparisons matched**, with **1,194 successful plans /
15,849 concrete block assignments** and **1,010 failed generations**.

Four unavailable-input refusals are the four seeds for Medieval REGION0:
`Realms/Medieval/Test` sections 0 and 20..29 are absent. Its eleven missing inputs
are listed individually in the report. Those original missing-file recovery
paths are not executed or claimed equivalent; no replacement assets are inferred.

The complete native corpus also passes ASan/UBSan with identical outcome counts.
Unit checks exercise actual assembly from a generated plan, immutable source
cells, repeated-ID source identity, selected Specific rotation, incomplete and
mutated-plan refusal and ten-attempt failure without an assembly plan. All
fourteen rendering reconstruction CTests pass in normal and ASan/UBSan builds.
Leak detection is disabled under the execution sandbox. The frozen baseline,
source hashes, helper hashes and outcomes are recorded in
[the companion report](terrain-generated-plans.json).

Confidence is high for selected generation state and successful-assignment
conversion within this bounded domain. Installed full MAP payload assembly,
generated scene frame comparisons, original CFG/file-error continuation,
entities, water, lighting and live integration are separate. The next chunk is
to connect available generated recipes to complete MAP loading, ordinary assembly,
geometry initialization and bounded scene comparisons through the native preview.

Complete installed MAP loading, owned ordinary assembly, post-assembly geometry
and bounded generated scene comparisons are now connected in the subsequent
[native generated scene milestone](native-generated-terrain-scenes.md).
