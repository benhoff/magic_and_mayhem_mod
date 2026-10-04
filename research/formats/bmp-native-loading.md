# Native BMP loading

Reviewed 2026-10-04. This is a native offline decoder and inspector for the
installed BMP subset. No original executable, renderer upload or live image
replacement was exercised.

## Installed evidence

All 48 BMP files in `working/game-clean` use the `BM` signature, a 40-byte
BITMAPINFOHEADER, one plane, 24-bit pixels and BI_RGB compression (zero).
They total 13,814,536 source bytes. All have positive heights (bottom-up),
pixel offset 54, zero color counts and no optional color table. Widths range
from 49 to 800; heights from 6 to 600. Five 97x48 images and one 49x23 image
have row padding. Forty-six files set the image-size field to zero and contain
two extra bytes after the pixel rows, within their declared file size. Two
files specify the exact encoded row extent and have no trailing bytes.
All declared file sizes equal their actual sizes.

Confidence is high for these installed bytes and native policies. The original
loader's error handling, transparency and placement contracts are unverified.

## Structure and references

The little-endian file layout begins with a 14-byte
[BITMAPFILEHEADER](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader):
`BM`, DWORD file size, two reserved WORDs and DWORD pixel offset.
It is followed by
[BITMAPINFOHEADER](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader):

| File offset | Field |
| ---: | --- |
| 14 | Header size, DWORD |
| 18, 22 | Width and signed height, LONGs |
| 26, 28 | Plane count and bits per pixel, WORDs |
| 30, 34 | Compression and image size, DWORDs |
| 38, 42 | Horizontal and vertical pixels per meter, LONGs |
| 46, 50 | Used and important colors, DWORDs |

Positive height means bottom-up; negative means top-down. Rows are rounded up
to four-byte boundaries: `stride = (width * 3 + 3) & ~3` for this subset.
Uncompressed image size may be zero. Pixel triples are stored blue, green,
red; see the [RGBTRIPLE byte order](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbtriple).
Native output is tightly packed RGB in top-down order.

## API and native policies

[assets/bmp.hpp](../../assets/bmp.hpp) exports `BmpImage`, `BmpLimits`,
`BmpResult`, `decodeBmp(bytes)` and `loadBmp(file)`. Link `mnm-bmp-loader`.
The standard-library public types and decoder depend on native asset file
access, without widgets, runtime hooks or renderer dependencies. Images own
all RGB bytes and preserve source orientation, pixel offset, stride, image-size
field, resolution and color-count metadata. No source pointers survive.
No alpha or color-key transparency is inferred.

These policies describe the native implementation, not recovered original errors:

- Require `BM`, zero reserved file fields and a 40-byte information header.
  Other DIB header sizes return unsupportedEncoding.
- Require one plane, 24-bit BI_RGB. Indexed, bitfield, compressed and alpha
  formats return explicit unsupported errors.
- Require positive width and nonzero height; reject INT32_MIN height.
  Both orientations are accepted, with magnitude/sign conversion checked.
- Validate pixel offset against the complete header and any declared optional
  optimization color table (`54 + colorsUsed * 4`). Such entries are skipped;
  they do not replace direct RGB pixels. Resolution/count metadata is retained.
- Require declared file size no greater than actual input and large enough
  for all pixel rows. Nonzero image-size fields must equal the padded row
  extent. Gap bytes and trailing bytes inside or after the declared file
  extent are allowed; they are bounded by input limits and not decoded.
- Defaults: 64 MiB input, 8192x8192 maximum dimensions, 16,777,216 pixels
  and 48 MiB decoded RGB. Compute strides/extents in 64 bits; reject strides
  outside the public 32-bit range. Validate all extents before allocation.
  Decode directly into one owned output buffer, without row scratch storage.
- Structured errors carry code, file-byte offset and detail. Backend read
  errors retain their original Error. Unrepresentable signed read limits
  fail as invalidArgument; allocation/length failures become limit errors.
  Failed decoding never returns a partial image.

## Build, inspection and validation

```bash
cmake -S assets -B working/build/bmp -DBUILD_TESTING=ON
cmake --build working/build/bmp --parallel 4
ctest --test-dir working/build/bmp --output-on-failure
python3 tests/test-bmp-loader.py working/build/bmp/mnm-bmp-inspect \
    --installation working/game-clean \
    --report working/tests/bmp-loader/installed-comparison.json
```

`mnm-bmp-inspect ROOT PATH.bmp` resolves assets through AssetStore, closes its
file before emitting JSON, and reports metadata plus RGB SHA-256. Failure exits
with status 2. It requires no application widgets or display server.

Native fixtures verify known RGB pixels, both orientations, nonzero padding,
output ownership, zero/exact image size, optional color-table extents, gaps and
trailing bytes, malformed fields, every truncation of a valid file, input/output
limits, extreme signed dimensions and oversized caller budgets. Ten independent
Python fixtures cover all four padding sizes, both orientations and case-folded
paths. Missing files, malformed input and parent traversal must fail.
The Python reference unpacks BGR triples row-wise, reverses row order when
needed, and compares complete RGB hashes and every inspector metadata field.
Optional installed runs record per-file source hashes and verify the immutable
original manifest before/after, including on failure. Extracted inputs and
immutable source preservation are separate evidence.

All 15 asset CTests pass. Both BMP tests pass with AddressSanitizer and
UndefinedBehaviorSanitizer; leak detection is disabled because tracing prevents
LeakSanitizer in this environment. All 48 installed files match the independent reference (4,603,863 pixels).
All 2,927 immutable originals verify before/after. The installed report is
`working/tests/bmp-loader/installed-comparison.json`; its ordered records,
encoded as JSON with sorted keys and compact separators, have SHA-256
`081ad36f2392d92882669aed766060e0ac1022cbc42cbe7753a87dad6ea1455c`. The tested native inspector SHA-256 is
`63bc5aae6c80e277315535f1fc4b3ad6816642c11c4867c95a108f8e4b58dc1e` (GCC 16.2.1, C++17, Qt 6.8 Core).

Remaining milestones: original caller contracts, game-specific transparency
and placement, renderer/UI consumption and live visual comparison. Existing
Qt menu image loading remains a separate path. See the
[coverage ledger](../runtime/coverage-ledger.md).
