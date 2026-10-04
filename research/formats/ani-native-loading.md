# Native ANI version-5 loading

Reviewed 2026-10-04. The native loader in `assets/animation.hpp` reads owned
version-5 animation tables through `AssetFile`. Selected original No-CD loader
and controller instructions confirm the layout; installed-byte comparison and
controller execution are separate evidence. No live loader is replaced.

## Layout

| Position | Supported interpretation |
| --- | --- |
| +0 | `ANI\0` signature |
| +4 / +8 / +12 | Declared file bytes, record count, version |
| +16 | Opaque header word |
| +20 | Offset count, including the final sentinel |
| +24 | Exactly 20 associated SPR filename bytes |
| +44 | Offset-count DWORDs, containing record indices |
| After offsets | Record-count records, 44 bytes each |
| Record +0 / +4 | Opcode and signed argument |
| Record +8..+43 | Nine preserved raw words, including eight name bytes at +16 |

Offset entries increase from zero to the record count. A sequence is the range
between adjacent entries; the final offset is an extent, not another sequence.
Every supported installed sequence ends in opcode 6. An empty sequence is a
single stop record, rather than a zero-length range. Other fields include
coordinate-like values; their full placement/attachment semantics are not
interpreted by this parser or the bounded scene.

Evidence: original loader `0x004644d0..0x00464ab4` reads a 44-byte header and a
four-byte offset table. Version 5 reads remaining records directly; sequence
count `0x00464b20` returns header offset-count minus one. Sequence selection at
`0x00464b30` computes `records + offsets[sequence] * 44`. Original version-3/4
paths expand older 28/36-byte records into the 44-byte in-memory representation;
those conversions are outside the native loader's current scope.

The pinned MMSprite notes were a useful starting point, but their speculative
action labels are not promoted to confirmed game contracts. Numeric sequence
selection is supported; compass orientation and action-to-sequence mapping
require caller/configuration recovery.

## API, ownership and safety

`decodeAnimation(bytes, limits)` and `loadAnimation(AssetFile&, limits)` return
an owned `Animation` or structured `AnimationError`. Headers, offsets, records,
signed arguments and all raw metadata survive file closure/input destruction.
There are no Qt types or original pointers in this public API. The caller owns
the `AssetFile`; loading uses the existing bounded `readWhole` helper from zero.
Input errors retain the structured asset error.

Defaults cap input at 8 MiB, records at 65,536 and sequences at 4,096. Signature,
declared size, count/record/table extents, increasing offset ranges and terminal
stops are validated before unsafe indexing. Allocation/length failures return
limit errors. Versions other than 5 return `unsupportedVersion`; signatures
other than ANI return `invalidFormat`. These strict native malformed-input
policies are not claims of original failure equivalence. Unsupported older
ANI files are known original-supported formats, not presumed invalid assets.

The stored filename remains raw data. Automatic sibling basename resolution
is an application policy, not performed by the parser. Unknown opcodes and
metadata are preserved. Playback validates control transfers separately.

## Reproduce and recorded evidence

```bash
cmake -S assets -B working/build/assets
cmake --build working/build/assets --parallel 4
ctest --test-dir working/build/assets --output-on-failure
python3 tools/export-animation-support.py
python3 tools/test-animation-contract.py
```

The exporter and comparison runner verify the original manifest before/after,
also on failures, and pin the No-CD executable to SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The comparison runner needs `g++ -m32` and ELF i386 execution. The native loader
and fixture-only tests need neither original executable execution nor Wine.

`mnm-animation-inspect ROOT MANIFEST` accepts a JSON array of asset-path strings,
bounded to 1 MiB/4,096 entries. It loads through AssetFile, closes each input,
and reports raw filename bytes, offsets and reserialized canonical little-endian
record hashes. It preserves per-file unsupported errors in JSON. Exit 0 means
the bounded inspection completed; invocation/I/O errors return 2. Installed
experiments should use the wrapper rather than invoke this CLI directly.

The [inventory report](ani-inventory.json) records **136 ANI files**: **133
version 5**, two version 4 (`Sprites/LordKing.ani`, `Sprites/cursors.ani`) and
one version 3 (`Sprites/objects.ani`). Supported files contain **121,476
records**, **27,940 sequences** and **14,581 empty sequences**. All supported
headers' record counts agree with the independently measured payload.

The [native comparison](../runtime/animation-forward-contract.json) confirms
all 133 supported files' offsets, filenames, opaque header words and record
bytes through mixed-case relative/aliased Windows requests. All three older
files return `unsupportedVersion`. The unit fixture covers ownership, raw
metadata/signed values, malformed/truncated headers, table/record extents,
terminal stops and input/count budgets. It passes ASan/UBSan with leak checking
disabled for this environment.

Confidence is high for this layout, installed decode and native ownership/
validation policies. Timing/control evidence is recorded separately in the
[forward controller contract](../runtime/animation-forward-contract.md).
