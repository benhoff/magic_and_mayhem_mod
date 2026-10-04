# Native PCX loading

Reviewed 2026-10-04. Scope: an offline native indexed-image loader and inspector.
No original executable was run and no live image replacement is claimed.

## Installed evidence and structure

All 124 PCX files under `working/game-clean` use version 5, RLE encoding,
8 bits per pixel and one plane, with a terminal 256-entry RGB palette.
Total encoded size: 3,459,543 bytes. Dimensions are 800x600 (82 files),
640x480 (41 files), and 436x108 (one file), totaling 52,002,288 pixels.
PaletteInfo is zero in 122 files and one in two; the reader preserves it.
All installed row strides equal width. Every encoded image ends exactly at
its palette marker. Confidence: high for this installed corpus and native
policies; original caller semantics remain unverified.

The [ZSoft Technical Reference Manual, revision 5](https://techheap.packetizer.com/compression/graphics/pcxfmt.html)
defines the following little-endian layout:

| Offset | Field |
| ---: | --- |
| 0 | Manufacturer 10 |
| 1–3 | Version, encoding, bits per plane |
| 4–11 | Inclusive xmin/ymin/xmax/ymax bounds, four WORDs |
| 12–15 | Horizontal/vertical DPI, two WORDs |
| 16–63 | Older 16-color header palette |
| 64–65 | Reserved byte, number of planes |
| 66–69 | Bytes per row per plane, palette information, two WORDs |
| 70–127 | Screen dimensions and filler |
| 128 onward | RLE image rows |
| Last 769 bytes | Marker 12 followed by 256 RGB triples |

Width/height are maximum minus minimum plus one. Stride includes padding
and is even. Bytes below 192 are literals; other bytes encode a run length
in the low six bits followed by its value. Runs end within scanlines.
The native reader removes row padding and returns top-down indices.

## API and policies

[assets/pcx.hpp](../../assets/pcx.hpp) exports `PcxImage`, `PcxLimits`,
`PcxResult`, `decodePcx(bytes)` and `loadPcx(file)`. Link `mnm-pcx-loader`.
The decoder and public types use the standard library; read-only file access
uses the existing native asset backend. No widgets, renderer or runtime hooks
are involved. Each result owns its indices and all 256 RGB palette entries,
dimensions, origin, DPI, row stride, palette information and source-byte count.
Palette entries have no inferred alpha or transparent index. Origin metadata
is preserved without assigning game placement semantics.

Native validation policies are explicit, rather than recovered original errors:

- Require version 5, encoding 1, 8 bits and one plane. Other PCX modes, including
  planar low-bit-depth and 24-bit RGB, return unsupportedEncoding.
- Require ordered bounds and an even stride at least as large as width.
- Require the complete terminal palette and all declared image rows.
- Reject zero-length runs, runs crossing a scanline, truncated values and
  extra bytes between the final row and palette. Unused reserved/header fields
  are ignored; PaletteInfo is retained without rejecting observed zero values.
- Defaults: 64 MiB input, dimensions at most 8192x8192 and 16,777,216 output
  pixels. Padding is decoded without an additional row buffer. Output is
  one byte per pixel plus a fixed 768-byte palette; source input is separately
  bounded. Allocation/length failures return limit errors.
- Errors include a code, file offset and detail; input errors retain the
  backend Error. A read limit exceeding the signed file API range fails as
  invalidArgument. No partial decoded image is returned.

## Reproduction and validation

```bash
cmake -S assets -B working/build/pcx -DBUILD_TESTING=ON
cmake --build working/build/pcx --parallel 4
ctest --test-dir working/build/pcx --output-on-failure
python3 tests/test-pcx-loader.py working/build/pcx/mnm-pcx-inspect \
    --installation working/game-clean \
    --report working/tests/pcx-loader/installed-comparison.json
```

`mnm-pcx-inspect ROOT PATH.pcx` uses case-insensitive AssetStore resolution,
closes the file before emitting JSON, and reports metadata plus SHA-256 of
indices and palette. Invalid input exits with status 2.

The Python reference expands the encoded stream independently before removing
padding and compares every metadata field and the complete palette/indices
through hashes. Four synthetic fixtures exercise odd widths, escaped literals,
nonzero origins and path case folding. Native fixtures cover repeated runs,
padding, palette ordering, unsupported modes, malformed bounds/stride/runs,
every truncation of a valid fixture and limits. Installed reports record
per-file source hashes. Optional installed runs verify the immutable original
manifest before/after, including on failure; extracted inputs and immutable
source preservation remain separate evidence.

All 13 asset CTests pass. Both PCX tests pass under AddressSanitizer and
UndefinedBehaviorSanitizer with leak detection disabled because environment
tracing prevents LeakSanitizer. All 124 installed files match the independent decoder, totaling 52,002,288
pixels. Original-manifest verification passes for all 2,927 files. The report is
`working/tests/pcx-loader/installed-comparison.json`; its ordered records, encoded
as JSON with sorted keys and compact separators, have SHA-256
`6d1d70d0c0dd52fda21abf07bbaff6a6b975d5546f9b074cf0442d70b417aabb`.

Remaining work: original loader/caller contracts, game-specific transparency
and placement, renderer upload, UI consumption and live comparison. These
remain separate milestones in the [coverage ledger](../runtime/coverage-ledger.md).
