# Native CUR loading

Reviewed 2026-10-04. This is an offline native asset reader and inspector, not
live cursor selection or presentation. Data evidence is the 20 installed
`working/game-clean/Cursors/*.cur` files, totaling 10,984 bytes. Confidence is
high for their directory layout, DIB fields, palettes, decoded planes and
hotspots; no original game/GDI cursor execution was performed.

Representative installed input SHA-256:

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `attack.cur` | 326 | `39cd16ba27b363d6507375c4b571677d919a4bca98a55e4ad2c0a9636527d9e4` |
| `chest.cur` | 2,558 | `8f7810fdad30f9992bba8660ed5f72f613e0f99360aa26edbed2c70946561505` |
| `default.cur` | 2,558 | `0273ed1d239b12d90f855d658a63f2f2ca3b76d9bb2893b136e0fb091e772e98` |
| `scrollright.cur` | 326 | `015c4b31cfbbbeeec37d05b3b5c07665b28c8e819b8faf129e7755931b0451e3` |

## Installed structure

All integers are little-endian. The six-byte header is reserved WORD 0, type
WORD 2 (cursor) and image-count WORD. A directory follows, with 16 bytes per
image. The installed files have one image each except `default.cur` and
`chest.cur`, which each have two: 22 images total.

| Relative directory offset | Bytes | Field |
| --- | ---: | --- |
| 0 | 1 | Width; zero encodes 256 |
| 1 | 1 | Height; zero encodes 256 |
| 2 | 1 | Colour count metadata |
| 3 | 1 | Reserved |
| 4 | 2 | Hotspot X |
| 6 | 2 | Hotspot Y |
| 8 | 4 | Encoded image byte extent |
| 12 | 4 | Absolute file offset to DIB |

Every installed image is 32x32. Twenty images are 1-bpp and two are 8-bpp.
The DIB header is 40-byte BITMAPINFOHEADER with width 32, height 64 (XOR and AND
planes), one plane, BI_RGB compression 0, and coloursUsed 0. The XOR image-size
field is 128 for 1-bpp and 1,024 for 8-bpp. The palette has 2 or 256 four-byte
BGR/reserved entries. It is followed by XOR rows and then 1-bpp AND rows.
Rows are bottom-up, bits are MSB-first and each row is padded to four bytes:
`xorStride = ((width * bitDepth + 31) / 32) * 4`,
`andStride = ((width + 31) / 32) * 4` (integer division).

DIB palettes use coloursUsed when nonzero; zero selects the full bit-depth
palette. Directory colour counts are all zero, even for the 1-bpp images, so
that byte cannot determine palette length. The 1-bpp palettes are black/white.
All installed palette reserved bytes are zero; the reader retains that byte
without interpreting it as alpha.

The common hotspot is (1,1). Scroll images use edge/corner hotspots such as
(31,15), (17,31) and (31,31). Both additional 8-bpp images have hotspot (0,0),
which must remain separate from their first image's (1,1). The loader retains
directory order and all alternatives; it does not choose an image automatically.
Names alone do not establish the game's cursor-selection state machine.

## AND/XOR preservation

[Microsoft's cursor-mask example](https://learn.microsoft.com/en-us/windows/win32/menurc/using-cursors#creating-a-cursor)
identifies monochrome mask pairs for black, white, unchanged background and
inverted background. Consequently, a generic cursor asset must retain both
planes rather than reduce every AND=1 pixel to transparent alpha. The native
fixture checks all four operations against a background byte.

The installed monochrome images have no nonzero XOR colour where AND=1.
The 8-bpp `chest.cur` and `default.cur` alternatives have 721 and 474 such pixels,
respectively, with palette RGB (4,4,4). The native reader preserves those bytes;
it neither clears the XOR colour nor converts them into an ordinary alpha mask.
Their actual original display behavior has not been compared. A future presenter
must make any background-dependent composition or approximation policy explicit.
A reusable native renderer and a Qt/OS cursor adapter are separate milestones.

For standard DIB field semantics see
[Microsoft BITMAPINFOHEADER](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader).
For the hotspot's input-coordinate role see
[Microsoft's cursor overview](https://learn.microsoft.com/en-us/windows/win32/menurc/about-cursors#the-hot-spot).
These references support the format interpretation; installed bytes and
synthetic/native tests supply this repository's evidence.

## Native API and input policies

[assets/cursor.hpp](../../assets/cursor.hpp) exports `CursorAsset`, `CursorImage`,
`CursorLimits`, `CursorResult`, `decodeCursor(bytes)` and `loadCursor(file)`.
Link `mnm-cursor-loader`, which depends only on the native asset backend.
Public types and decoding use the standard library, without Qt types, widgets,
renderer dependencies, Wine handles or original addresses.

Each image owns its palette (RGB plus original reserved byte), top-down
`xorIndices` and unpacked `andMask` (one 0/1 value per pixel), dimensions,
hotspot and source extent. All palette indices are checked. A result outlives
its source buffer, AssetFile and AssetStore. No partial asset is returned after
any image fails. Caller selection is explicit through `asset.images[index]`.

Supported subset: uncompressed indexed 1/8-bpp, bottom-up, 40-byte
BITMAPINFOHEADER images. This covers every installed image. PNG, compressed DIB,
4/16/24/32-bpp, alpha and other DIB headers are explicitly unsupported; an ICO
(type 1) is not treated as a CUR. There is no ANI animated-cursor decoder here.

Native validation policies, not recovered original error behavior:

- Require reserved header/directory fields zero and a nonempty directory.
- Bound every declared image to the input and reject offsets into the directory.
  Aliased/overlapping payload extents are independently parsed and charged to
  aggregate budgets; aliases do not share mutable decoded storage.
- Require matching directory/DIB dimensions, positive doubled DIB height,
  one plane, hotspots within the image and supported bit depth/compression.
- Validate complete palette, XOR and AND extents before pixel allocation;
  reject out-of-range palette entries. Accept unused trailing image/file bytes.
- DIB image-size may be zero, the XOR-plane size or the combined plane size.
  Other nonzero values fail; this is a deliberately strict native policy.
- Defaults: 4 MiB input, 64 images, dimensions at most 256x256, 1,048,576 total
  decoded pixels and 4 MiB aggregate decoded palette/plane bytes. Repeated
  directory aliases count toward both aggregate limits. Metadata/container
  overhead is bounded by image count, outside the decoded-byte accounting.
- Structured errors include code, file-byte offset, optional image index and
  detail. Asset input errors retain the backend Error. Unrepresentable signed
  file-read limits fail as invalidArgument; allocation/length failures become
  limit errors. Whole-file reads follow AssetFile's existing readWhole policy.

## Build and inspect

```bash
cmake -S assets -B working/build/cursor -DBUILD_TESTING=ON
cmake --build working/build/cursor --parallel 4
ctest --test-dir working/build/cursor --output-on-failure
working/build/cursor/mnm-cursor-inspect working/game-clean Cursors/default.cur
python3 tests/test-cursor-loader.py working/build/cursor/mnm-cursor-inspect \
    --installation working/game-clean \
    --report working/tests/cursor-loader/installed-comparison.json
```

The inspector uses AssetStore path resolution and closes its handle before
emitting JSON. It reports every image's dimensions, hotspot, bit depth, extent,
palette/plane SHA-256 and AND=1/nonzero-XOR count. It returns 2 on failure.
No display server, application widgets or original executable are needed.

The installed report inventory SHA-256 is
`0d3b622ad74ceb86b300e58e29486aacbe89eebef9f757becce7201e88e77a8b`
(canonical JSON of the ordered per-file records).

The Python reference uses independent row/bit unpacking and compares every
palette byte, pixel plane and metadata field through hashes. The optional
installed run verifies originals before/after, including on failure, and records
per-file source hashes plus all image results. These are installed extracted
inputs; original-manifest preservation is a separate check. Omit
`--installation` to run only independent synthetic comparisons.

Validation: all 11 asset CTests pass. Native fixtures cover both bit depths,
partial-byte/DWORD row padding, bottom-up orientation, reserved palette bytes,
short palettes, dimensions encoded as zero, multiple/aliased image entries,
hotspots, malformed/truncated inputs, aggregate limits and file/input ownership.
Four independent Python fixtures include 256x256 dimensions and padded/gapped
payloads. All 20 installed files/22 images match the independent decoder.
AddressSanitizer/UndefinedBehaviorSanitizer pass for both cursor tests; leak
detection is disabled because LeakSanitizer cannot run under environment
tracing. Original-manifest verification passes for all 2,927 immutable files.

No live cursor loading/replacement, original GDI pixel comparison, native
presentation policy or game-driven image selection is claimed. See the
[coverage ledger](../runtime/coverage-ledger.md).
