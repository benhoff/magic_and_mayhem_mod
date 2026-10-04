# MAP grid reader and selected original cell checks

No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Preferred addresses refer to this pinned build, not live pointer stability.
No Wine or original game process was launched and no instructions were patched.

## Reader and cell layout: confirmed static evidence

`0x004eeaf0..0x004eefb9` opens an encrypted stream via `0x004a1360`, decodes it
via `0x004a1590`, reads 76 header bytes via `0x004a1820`, and computes plane and
cell counts from header dimensions. Version dispatch supports 4/5/6. The
version-6 branch at `0x004eec1e` seeks to zero and allocates/reads exactly
`76+12*width*height*layers` bytes. It publishes the header pointer and a cell
pointer at header+76 (`0x004eef88..0x004eef93`). Full stream/allocator/reader
execution remains unvalidated; native bounds are a separate safety policy.

Selected checks at `0x004f13f0` form the cell pointer from receiver +0x4c plus
12*(X + row[Y] + layer[Z]); row and layer tables live at receiver +0x64b2 and
+0x6432. The grid fields correspond to width/height/layers and plane stride.
The selected mode-1, extent-1 path returns one precisely when flag byte +10
lacks 0x80, has either low bit set, and cell WORD +4 differs from the supplied
WORD token. It returns zero otherwise. Extent one exits before terrain or
additional-layer helpers are reached. This is an indexing/field validation,
not a reconstruction of the full occupancy/terrain predicate.

The rendering producer `0x004f8960` reads the same terrain WORD +0 and flag
WORDs +8/+10. Its recovered world height is vertical level*16. TTD arrays
supply view-specific frames; see [terrain submission](terrain-submission.md).
Object/creature references and runtime post-load normalization remain separate.

## Executed evidence and confidence

`python3 tools/test-map-loader.py NATIVE --sanitized SANITIZED` validates
original manifests before and after; pins the PE hash; records sources, native
binaries and every installed MAP hash; exports the selected assembly ranges.
It independently decodes all installed files with Python, compares complete
native typed-payload SHA-256 and dimensions/opaque header fields in both builds,
then runs the unchanged original helper on independently decoded cells in a
private PE32 mapping. Native coordinate access and flags/reference values supply
the expected result. Both the mismatching sentinel token and each cell's own
token are checked for every coordinate. No Win32/import/allocator paths run.

[Comparison report](map-grid-loading.json): 683 files, 7,595,200 cells,
15,190,400 matching selected original checks. Artifacts:
`working/tests/map-loader/run-ryw1h285/`. The native ownership/malformed-grid
CTest also passes normally and under ASan/UBSan (leak detection disabled).
Confidence is high for installed payload bytes, layer-major indexing and the
selected original field check. Reader/conversion control flow is static evidence;
full original MAP decode/initialization equivalence and live loading are pending.

A bounded native map slice is a separate application milestone; camera,
world traversal, lighting, topmost-surface selection and entity creation are
not implied by having an owned grid.

Selected post-load cell fields and the complete original geometry/surface pass
now have a separate [native initialization milestone](terrain-map-initialization.md).
It prepares an owned ordinary-terrain projection for scene rendering, with
explicit reference/object exclusions; the raw MAP reader remains unchanged.
Full original map lifecycle and entity creation remain unverified.
