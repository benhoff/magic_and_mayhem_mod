# Native TAG sprite-name tables

Reviewed 2026-10-04. Owned offline input reader and JSON inspector.
The schema is confirmed against every installed TAG and its companion SPR
frame-name table. No original TAG file-reading function or live use is established.

## Confirmed structure

All 85 installed files total 1,570,692 bytes and 130,891 records. Each file
contains between 847 and 2,418 records. There is no header, magic, version,
count word or checksum: file length divided by 12 determines the record count.
Confidence is high for the installed byte layout and SPR correlation.

| Record offset | Bytes | Meaning |
| ---: | ---: | --- |
| 0–7 | 8 | Raw sprite frame name, matching SPR frame header offset 20 |
| 8–11 | 4 | Little-endian occurrence index for that name |

Record ordinal equals frame ordinal in the companion SPR. The index is zero
for the first occurrence of a name and counts earlier frames with exactly the
same eight name bytes. This correlation holds for every installed record.
It is not an absolute frame index: indices restart for each new name.
The native API stores the full DWORD as unsigned, preserving unknown high bits.

There are 320 distinct raw names across the corpus. All installed names are
seven ASCII bytes followed by NUL, and occurrence indices range from zero to
59. Installed examples include `UA000S1` and `UC000S1`. Those are first frame
names, not two container signatures or format versions; the earlier inventory
classification is corrected. No fixed-name prefix is required for admission.

17 TAGs have a same-stem SPR. The other 68 section TAGs match `Terrain.spr`
in their own directory. This association is confirmed for the installed files,
but the parser does not resolve companions automatically or depend on SPR
loading. The corpus comparison uses that explicit association.

## Native API and policies

[assets/tag.hpp](../../assets/tag.hpp) exports `TagEntry`, `TagAsset`,
`TagLimits`, `TagResult`, `decodeTag(bytes)`, `loadTag(file)` and
`tagEntryName(entry)` through `mnm-tag-loader`. Public types and parsing use the
standard library and native read-only asset access. No widget, renderer, SPR
loader or build-specific reconstruction dependency is introduced.

Results own the ordered entries and source byte count. Entry ordinal remains
available through vector position. Each entry retains all eight name bytes
and the DWORD occurrence; no source pointers survive.

Native policies, not claims about original malformed-input behavior:

- Require file length divisible by 12; reject an incomplete final record.
- Accept an empty table. No empty installed example exists; this is a policy.
- Defaults: 4 MiB input, 262,144 records and 4 MiB decoded record storage.
  Charge `sizeof(TagEntry)` per record before allocation; container overhead
  is outside that record-storage budget.
- Preserve arbitrary name bytes, terminators/padding, duplicate names and
  arbitrary occurrence words. Do not validate the installed index sequence,
  normalize names, sort entries or discard duplicates during loading.
- `tagEntryName` returns bytes before the first NUL, bounded to eight bytes.
  Empty names and full unterminated fields are accepted without text decoding.
- Errors include code, offset and detail, with backend input errors retained.
  Input limits outside the signed file API range return invalidArgument;
  allocation/length failures return limitExceeded. No partial asset is returned.

Because TAG has no signature/count/checksum, removing or appending whole
records cannot be detected from TAG alone, and arbitrary data whose length
is divisible by 12 may parse. Companion SPR agreement is a separate validation
step, not an input admission rule or a guarantee supplied by `decodeTag`.

## Build, inspect and validate

```bash
cmake -S assets -B working/build/tag -DBUILD_TESTING=ON
cmake --build working/build/tag --parallel 4
ctest --test-dir working/build/tag --output-on-failure
working/build/tag/mnm-tag-inspect working/game-clean Realms/Celtic/Forest/Terrain.tag
python3 tests/test-tag-loader.py working/build/tag/mnm-tag-inspect \
    --installation working/game-clean \
    --report working/tests/tag-loader/installed-comparison.json
python3 tools/export-tag-support.py
```

The inspector closes its AssetFile before printing source/count metadata and
all name/occurrence fields. Display names use a reversible Latin-1 mapping;
`name_bytes_hex` retains the complete eight bytes. This display policy does not
establish the game's encoding. AssetStore provides containment and case folding.
Failure exits 2; no display server or original game process is needed.

Native fixtures cover ownership, unsigned extreme indices, empty/full/NUL
names, nonzero bytes after NUL, every truncated prefix, partial trailing records,
whole-record append acceptance and input/count/allocation limits.
Five independent Python `struct` fixtures compare every field, including both
previously misclassified first names, repeated/nonsequential entries and high
name bytes. Missing/traversal paths fail. Installed comparison checks every
field, SPR name/ordinal agreement and occurrence sequence, then rehashes all
TAG/SPR inputs. Immutable originals verify before/after, including on failure.

All 24 asset CTests pass in the tested tree, including the terrain catalog test
from the concurrent terrain milestone. Both TAG tests pass under ASan/UBSan;
leak detection is disabled because tracing prevents LeakSanitizer, and prebuilt
Qt is uninstrumented. All 85 TAGs/130,891 records match independent decoding
and the 17 companion SPR name tables. All 2,927 immutable originals verify.
Ordered comparison SHA-256:
`729ac3b04b69588588b4abd53c3d6504d1a67e13dba57f42a14e42c012e824e0`.
Tested inspector SHA-256:
`fac511e6385019fcb80aa04dbfbdb42c4bfed7683e30a4581e601508ced41581`
(GCC 16.2.1, C++17, Qt 6.11.2).

Remaining work: determine original/editor TAG consumption and lookup contracts,
introduce consumers only where needed, and validate bounded live use before
claiming replacement. This reader does not implement name lookup, world
construction, sprite rendering or a writer. See
[correlation evidence and runtime boundary](../runtime/tag-sprite-tables.md)
and [coverage ledger](../runtime/coverage-ledger.md).
