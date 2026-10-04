# Ordinary terrain geometry initialization and surface admission

Pinned No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This milestone reconstructs selected cell initialization and the complete
geometry pass `004ecf20..004ed43b`, then uses their results in native terrain
scene production. It does not replace the complete original map load or create
runtime entities.

## Recovered load sequence

Static assembly `004ee684..004eea3c` shows list cleanup, a cell loop and then
additional grid/entity initialization. The selected cell loop:

- Resets cell WORD +2 to `ffff`.
- Sets flags10 bit 0 exactly when definition is nonzero and its TTD DWORD +94
  equals `10`; otherwise clears bit 0.
- Takes an allocation/list branch when flags10 bit `08` is set. That branch invokes
  object helpers and can change cell references. Its behavior is not replaced.
- Clears flags10 bit `2000` and resets cell WORD +4 to `ffff`.

It then calls `004ecdd0` (five packed grid buffers and `004f1890`), `00545570`,
`004ecf20`, `004ecda0`, one of `004efc80`/`004eff00`, and `004f06b0`.
The latter functions include configured entity admission, random choices and
allocation; these remain separate. Header/global binding, full map allocation,
packed grid buffers and these later calls are not implied by this milestone.

Confidence: high for these static field operations and call order; the complete
cell loop and its object branch are **not executed** by the test harness.
Independent Python checks validate the native selected field operations and
explicit projection against all installed MAPs, without claiming original
whole-load equivalence.

## Geometry pass: executed original contract

The first pass visits z=0..layers-2, rows then columns. X/Y neighbors wrap by
map width/height, including a one-cell extent. All TTD lookups see the pre-cull
terrain definitions. The top layer is untouched by this function.
TTD byte +a8 supplies these selected connection predicates:

| Cell flags8 bit | Conditions |
|---|---|
| `08` | east +a8 bit `08`, south bit `01`, above bit `20` |
| `10` | south bit `01`, west bit `02`, above bit `20` |
| `20` | north bit `04`, west bit `02`, above bit `20` |
| `40` | north bit `04`, east bit `08`, above bit `20` |

The pass first clears flags8 bits `78`, then sets the above decisions.
For `boundary=200 << (flags8 & 3)`, disagreement with the above cell's flags8
low two bits sets flags10 `boundary`. Agreement clears that bit **and bit
4000**. Other ownership-boundary bits remain unchanged; do not replace this
with a blanket mask clear.

A cell becomes flags8 bit `80` (empty) exactly when its definition is zero,
all three reference WORDs are `ffff`, and flags10 bit `2000` is absent.
Otherwise bit `80` is cleared. This test precedes culling: a newly culled cell
need not acquire bit `80` in this same invocation.

A second complete pass visits the same lower layers. When all flags8 bits `78`
are set and the selected flags10 boundary bit is absent, it sets definition
zero, clears flags10 bits 0/1 and sets `4000`. This is the flag already tested
by all four recovered terrain traversals to suppress a visit. The complete
pass ordering matters; interleaving culls with neighbor lookup changes results.
Repeated invocation is not assumed to be idempotent.

Confidence: high within valid native dimensions/definition bounds, supported by
unchanged original execution and comparison of every cell field, including
preserved references and top-layer fields, on synthetic fixtures.

## Native ownership and integration

`reconstruction/rendering/terrain_map.hpp/.cpp` consumes owned native MAP/TTD
storage. Native asset readers continue to retain raw decoded bytes; they do not
call recovered algorithms. The geometry pass validates all dimensions, cell
extents and definition IDs before mutating any cell. Native limits are
1..128 in X/Y and 1..32 layers; malformed inputs are rejected as native policy.

`deriveTerrainSurfaces` implements the selected complete geometry function,
including arbitrary runtime references in its admission decisions.
`prepareTerrainGeometry` creates an independent map copy for ordinary-terrain
scene production. Its projection sets **all three** references to `ffff` and
clears flags10 bits `08` and `2000`, omitting object/creature branches. Clearing
reference +6 and object bit `08` is a deliberate preview projection, not recovered
original initialization. It then applies the selected initial classification
bit and executes the geometry pass.

The application requests this through `--world --initialize-terrain`, optionally
with `--recovered-camera`. Qt widgets only present the resulting image.
JSON records projected object-flag/reference counts, changed cells and removed
nonzero definitions. Raw MAP mode and bounded slice mode retain their previous
behavior. The existing original world-reference fixture accepts optional origin
correction fields so recovered-camera geometry can also be compared.

```sh
working/build/terrain-map/mnm-terrain-preview --root working/game-clean \
  --map Realms/Celtic/Forest/CFsec01.map --world --initialize-terrain \
  --recovered-camera --view 1 --visibility --output working/tests/geometry-example
```

## Validation and remaining boundaries

`tools/test-terrain-map.py` independently decodes installed MAPs, projects and
initializes them in Python, compiles a native/reference harness, and executes
the unchanged `004ecf20` in a private PE32 mapping. Selected code has no imports,
allocator calls or Wine dependency. It compares all 12 bytes of every cell,
including definitions, flags and references. Base setup has independent byte
checks and static evidence; geometry has executed original evidence.

**Executed results:** 2,048 synthetic maps/387,962 complete cells and 683
installed maps/7,595,200 complete cells match the original geometry function.
All installed geometry also matches under ASan/UBSan. There are 96 initialized
scene image comparisons, plus 192 passing raw-scene regression images and
18,428 safe traversal cases/3,089,824 queue records. All original guards pass.
See [comparison report](terrain-map-initialization.json); artifacts are in
`working/tests/terrain-map/run-esfz7avy/` and
`working/tests/world-terrain/run-rkq05n_m/`.

The native CTest covers source ownership, 1x1 wrapping, owner disagreement,
empty-cell references, top-layer preservation, culling and invalid definition
rejection before mutation. Camera, traversal, queue and visibility CTests also
pass in normal and ASan/UBSan builds.

The scene oracle uses independently initialized cells, unchanged original
traversal/terrain producer/sort/visibility and independent SPR pixel composition.
Installed MAP/catalog pairing uses the region CFG SpritePath where a section
directory lacks its own Terrain.ttd (for example Delphi → Greek Village).
The application still accepts an explicit realm/catalog directory; automatic
region orchestration remains separate.

Installed checks cover CFsec01/02/38, all four views, explicit/recovered camera
configurations, and visibility off/on in both normal/sanitized builds. Complete
RGB565 bytes, presented RGBA hashes, queues, physical cell ownership and resource
release are checked. This is not a capture of original whole-scene pixels.

Original manifests are guarded before/after; executable, source, asset and
native build hashes and assembly exports accompany the report.

Remaining: region section assembly/rotation, full original map lifecycle,
object/creature admission, packed grid
buffer initialization, lights/palettes, water and other terrain producers,
configured entities/effects, visibility activation, original full camera
lifecycle, input clock and live integration. No gameplay balance changes.
