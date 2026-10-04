# Native SFT font loading

Reviewed 2026-10-04. Owned offline version-3 font input, byte-to-glyph lookup
and diagnostic glyph-atlas export. Native text layout, application font use
and live loader replacement are separate milestones.

## Confirmed layout

All six installed fonts total 624,508 bytes, 1,205 glyphs and 26,459 metric
row pairs. Five fonts contain 223 glyphs; `yellowtext.sft` contains 90.
All have one RGB palette and version three. Confidence is high for this layout:
complete installed decoding agrees with independent Python decoding, and the
[selected original reader and consumers](../runtime/sft-font-loading.md)
confirm the pointer arithmetic and contour use.

| Offset | Bytes | Field |
| ---: | ---: | --- |
| 0 | 4 | `SFT\0` magic |
| 4 | 4 | Complete file size |
| 8 | 4 | Version, three |
| 12 | 4 | Glyph count |
| 16 | 4 | Rows per glyph spacing profile |
| 20, 24 | 4 each | Ascent/descent words; preserved as signed values |
| 28 | 4 | Opaque header word, preserved |
| 32 | 4 | Palette presence; any nonzero value means one palette |
| 36 | 4 | Number of glyph spacing profiles |
| 40 | 768 if present | 256 RGB triples |
| After palette | rows × profile count × 8 | Glyph-major signed leading/trailing row pairs |
| After profiles | glyph count × 4 | Glyph offsets relative to end of this table |
| After offsets | Variable | Table-selected compressed glyph frames |

Integers are little-endian. Profile and glyph counts are distinct fields;
installed counts agree, but the native API preserves them independently.
Consumers must check profile availability before indexing. The contour pairs
support stateful spacing and are not fixed glyph widths.

Glyphs use the [SPR frame header, row RLE and auxiliary planes](spr-native-loading.md).
The native implementation shares that decoder without constructing a synthetic
SPR file: source offsets and errors refer to the actual SFT input. Indexed
frames preserve palette indices and raw RGB; no-palette RGB565 input is covered
by synthetic tests only. Frame origins, raw eight-byte names, opacity masks,
zero-valued opaque pixels, padding and auxiliary bytes are preserved.

`sftGlyphIndex` maps raw byte 33 to glyph zero and rejects unavailable ordinals.
The installed 223-glyph fonts cover bytes 33–255; yellow covers 33–122.
This is a raw asset mapping. Original space/underscore handling, tabs, newlines,
tracking and stateful contour spacing require a separate text-layout service;
Unicode conversion/code-page policy remains unresolved.

## Native boundaries and limits

`assets/sft.hpp` exposes `decodeSft`, `loadSft`, `SftFont` and `SftLimits`.
Output owns profiles, palettes, pixels, masks and auxiliary planes independently
of the input file. The API depends on native asset storage, not Qt presentation
or recovered runtime contracts. The optional inspector uses Qt for JSON/PNG.

Native validation requires exact magic, version three and declared file size;
all header/table/frame/row extents are checked before access. The original
reader accepts versions two/three and does not check magic; version two is
explicitly unsupported here. The native policies reject partial frame overlaps,
invalid palette references and malformed runs, while permitting identical
aliases and in-bounds noncanonical table selections. Empty fonts are supported
as a synthetic native policy, without installed evidence.

Defaults inherit SPR limits (including 4,096 glyphs, 2,048-pixel dimensions,
input/pixel/scan budgets) and add an 8 MiB profile budget. Profile storage also
counts toward the aggregate decoded-byte budget. File errors retain backend
information; decoder errors retain source offset and glyph ordinal where known.

## Reproduction and validation

```bash
cmake -S assets -B working/build/sft -DBUILD_TESTING=ON
cmake --build working/build/sft --parallel 4
ctest --test-dir working/build/sft --output-on-failure
python3 tests/test-sft-loader.py working/build/sft/mnm-sft-inspect \
    --installation working/game-clean \
    --report working/tests/sft-loader/comparison.json \
    --atlas-dir working/tests/sft-loader/atlases
```

`mnm-sft-inspect ROOT PATH.sft [ATLAS.png]` reports every profile, header word,
raw palette, glyph header, pixel, mask and auxiliary byte. The atlas places
glyph bitmaps in a transparent 16-column grid using source RGB or expanded
RGB565, with a 16M-pixel output cap. It is a diagnostic image; it does not apply
baseline placement or reproduce original text spacing/display palette conversion.

The comparison runner performs original-manifest checks before and after
installed experiments, rechecks source hashes, and independently unpacks all
fields and row runs. Four synthetic comparisons exercise indexed/direct input,
noncanonical palette flags, aliases, empty fonts, transparency and auxiliary
planes. C++ tests exercise ownership, byte lookup, malformed headers/runs,
truncation and aggregate limits. All 29 asset CTests pass; SFT unit/comparison
and shared SPR unit tests also pass ASan/UBSan with leak detection disabled.
[Retained evidence](sft-native-loading.json) records inputs, tool hashes and
comparison results. The shared decoder regression reloaded all 174 supported SPRs (59,407 frames),
rejected seven legacy versions, and matched 91 draw/77 conversion samples against
retained original artifacts. A fresh isolated i386 original harness attempt
terminated with SIGSYS in this environment; this regression used existing
original outputs, with their report hash retained. The standard SPR runner in
[SPR loading](spr-native-loading.md) reproduces the full comparison where that
harness can execute. Original SFT code was inspected statically, not executed as
an SFT differential oracle; live text rendering remains unvalidated.
