# Native MAP terrain grid loading

All 683 installed `.MAP` files use the seed/encrypted/checksummed container
already decoded by `decodePackedContainer` with `cfgBytes` transformation.
Their decoded payloads have version 6, a 76-byte header, and packed 12-byte cells.
There is no `MAP` signature inside this payload.

| Payload offset | Field |
| --- | --- |
| +0 | DWORD version (6) |
| +4/+8/+12 | DWORD X width, Y height, vertical layer count |
| +16/+20 | DWORD cells per plane and total cells |
| +24..+75 | Thirteen retained opaque DWORDs |
| +76 | Cell array, X fastest, then Y, then vertical layer |

A cell is six little-endian WORDs: terrain definition ID +0, retained references
+2/+4/+6, and flag words +8/+10. Selected runtime evidence identifies +4 as an
occupant token; the reader does not resolve any reference or impose flags.
Terrain definition IDs address the companion TTD catalog. Height/elevation for
a submitted cell is its vertical layer index, rather than a separate cell field.
Index is `(z*height+y)*width+x`. Empty/body-less definitions remain exact values;
no surface selection, topmost-cell policy or terrain substitution is applied.

The original reader `0x004eeaf0..0x004eefb9` reads 0x4c header bytes, recomputes
width*height*layers and seeks to offset zero for version 6 before reading
`76+12*count` bytes. Versions 4/5 have conversion branches and remain unsupported
natively. [Runtime evidence](../runtime/map-grid-loading.md) distinguishes this
static reader evidence from executed selected cell-helper comparisons.

Native `assets/map` validates both outer checksums/decompression, nonzero
bounded dimensions, plane/total agreement and exact payload extent before cell
allocation. Defaults cap input/decoded bytes at 16 MiB, each XY dimension at
128, layers at 32 and total cells at 524,288. These are native safety policies;
malformed-input behavior is not claimed equivalent to the original reader.
Returned metadata and typed cells own all values. Coordinate access checks
bounds; file errors retain their structured AssetFile cause. Runtime reference
initialization and post-load flag mutation are deliberately absent.

`mnm-map-inspect ROOT PATH [PATH ...]` serializes every typed field back to its
canonical little-endian payload and reports its SHA-256. Independent Python
container decoding matches all 683 full payloads / 7,595,200 cells, in normal and
ASan/UBSan builds. Observed maximum dimensions are 40x40x30. Ownership, bounds,
unsupported versions, inconsistent counts, truncation and limits are checked by
`map-loader-owned-grid` in both builds. LeakSanitizer is disabled in managed
execution; prebuilt Qt dependencies are uninstrumented.
