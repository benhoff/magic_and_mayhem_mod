# Terrain section placement and rotation

Selected No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The native terrain preview can assemble an explicit complete grid of square MAP
selections, rotate them, initialize ordinary terrain geometry across section
boundaries, and produce the existing four-orientation scene. This is offline
terrain assembly; random region selection and runtime entity placement remain
unrecovered.

## Executed copy contract

`004ef6b0..004efb7b` is a five-argument thiscall: source cell pointer, layer
count, destination X, destination Y, rotation. Receiver +50 supplies the square
side; +4/+8/+c are map dimensions, +4c points to 12-byte cells, and +6432/+64b2
contain layer/row cell offsets. Source is contiguous, X fastest, then Y, then Z.
All copied layers start at destination layer zero. This routine does not wrap
out-of-bounds coordinates or infer source dimensions.

For source `(x,y)` within side `s`, relative destination coordinates are:

| Rotation | X | Y | Definition lookup |
|---|---|---|---|
| 0 | x | y | None |
| 1 | s-1-y | x | TTD WORD +90 |
| 2 | s-1-x | s-1-y | TTD WORD +8c |
| 3 | y | s-1-x | TTD WORD +88 |

Rotation zero copies every field unchanged, even when object flag 8 is present.
For rotations 1..3, ordinary cells use a **single** lookup from the original
definition's 356-byte TTD record. These WORD fields overlap the previously
observed body-frame DWORD array; their use as rotated definition IDs is
confirmed here, without assigning that meaning to every bit of each DWORD.
The original changes the source definition WORD before copying the cell.
Source references and both source flag words remain unchanged.

After copying, destination flags8 direction bits cycle
`0400 -> 0800 -> 1000 -> 2000 -> 0400`, repeated rotation times. Every other
flags8 bit, all flags10 bits and all three references are preserved.
Resulting definition IDs are not checked by this original function.

If source flags10 bit 8 is present during rotation, the original calls
`004efb90`, which uses global object lists and additional helpers. Native copy
rejects that branch; it is not equivalent to the ordinary lookup.

Confidence: high for the selected ordinary branch, including source mutations,
with unchanged original execution and complete byte comparisons. Object-linked
rotation is identified statically and remains outside the executed scope.

## Caller and native boundary

Original caller sites `004ee116` and `004ee2e3` use placement indices multiplied
by receiver section side. The latter caller packs selected square rows from a
larger MAP into contiguous scratch before copying. Its surrounding selection
and matching loops, multi-block descriptors, allocations, entity/NOD placement
and random configuration choices are not reproduced by explicit preview inputs.
Static allocation helper `004ed8f0` has a minimum dimension check of 43 and
further global/grid/camera initialization; native grid allocation is bounded
policy rather than a reconstruction of that complete function.

`reconstruction/rendering/terrain_sections.hpp/.cpp` owns no Qt or file I/O.
`copyTerrainSection` reads a const MAP selection into owned scratch, validates
all selected definitions/objects and extents before destination mutation, then
copies rotated cells. This deliberately eliminates the original mutation of
loaded asset storage, while matching destination bytes. Source and destination
may alias safely because all source cells are staged before writes.
Rotation zero does not need a TTD lookup. Rotated target IDs are validated by
subsequent geometry initialization before rendering.

`assembleTerrainRegion` returns independent owned storage for one complete
explicit grid: each slot appears once, every source has the same layer count,
and square source selections are in bounds. Dimensions are 1..128 and layers
1..32. Holes, duplicate slots, overlap, invalid rotations and object rotation
are rejected. Opaque MAP metadata is not synthesized from unrelated source
headers; assembled metadata is zero and is not interpreted by this pipeline.

The preview requires `--world --initialize-terrain --grid columns,rows,side`
and a repeated `--section path,sourceX,sourceY,column,row,rotation`. MAP files
come through the existing Windows-path/case-insensitive native asset store.
`--map` and explicit grid are mutually exclusive. Preview sources are separately
owned and projected to ordinary terrain: clear object bit 8 and all references,
with projection counts recorded as **whole source assets**, including cells
outside a selected crop. Aggregate source storage is limited to 2,097,152 cells.
Geometry initialization runs once after assembly; this includes cross-section
neighbors and the recovered toroidal map-edge surface predicates. Ordinary
entity/reference projection, explicit selection, allocation and layout are
preview policies. There are no native entities, lights or gameplay changes.

## Validation

Run the reproducible wrapper with normal and ASan/UBSan preview binaries:

```sh
xvfb-run -a python3 tools/test-terrain-sections.py \
  --preview working/build/terrain-sections-frozen/mnm-terrain-preview \
  --sanitized working/build/terrain-sections-sanitized/mnm-terrain-preview
```

The wrapper pins executable bytes, guards the original manifest before/after,
hashes inputs, decodes all 683 installed MAPs independently, and selects their
matching TTD catalogs from local realm configuration, including shared-catalog
aliases. All four rotations compare complete destination bytes (including the
untouched border and extra layer) and all original source side effects. Random
fixtures exercise cropped rectangular sources, multiple layers, arbitrary
reference/flag words, and full 16-bit rotation targets. Native callers retain
unchanged source assets. Installed object flags are explicitly projected for
this ordinary branch; references and other fields remain arbitrary in the copy
oracle.

The image checks assemble three installed Forest MAP selections into 2x2 grids
of sides 10 and 20, assigning rotations 0..3. Independent Python section
placement and geometry produce the input for unchanged original
traversal/producer/sort/visibility execution. Native queues, physical ownership,
visited cell fields, full RGB565 images, RGBA hashes and surface release are
compared in all four views, with visibility on/off, in both builds. Pixels use
independent Python SPR composition with embedded unshaded palettes, rather than
original whole-scene rendering. There is no live Wine/game replacement test.

Validation passed: 8,192 synthetic cases/410,500 cells and 683 installed MAP
selections/30,084,800 copied cells across four rotations; 96 complete assembled
images; six normal and six ASan/UBSan unit tests; 16 CLI rejection checks.
The existing single-MAP pipeline also passed 18,428 original traversal cases
and 192 image regressions. Original manifest guards and
input hashes passed. Validation counts and artifact provenance are recorded in
[the companion report](terrain-section-assembly.json).
