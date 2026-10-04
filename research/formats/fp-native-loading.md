# Native FP Realm Viewer flag paths

Reviewed 2026-10-04. Owned offline version-2 input reader and JSON inspector.
Evidence combines every installed FP file with the hash-pinned No-CD reader,
constructor, point/flag-slot helpers and Realm Viewer loading caller.
Native flag allocation, movement, rendering and live UI integration are separate.

## Confirmed layout

All 41 files under `Interface/RealmViewer/Generic` total 132,356 bytes,
79 paths and 15,950 points. All use `.FP\0`, version two, a 116-byte header
and eight-byte x/y point records. Installed files have one to four active paths,
with 74–957 total points per file; the header has capacity for eight paths.
Confidence is high for byte layout, active range use and the flag-path role.
The precise purpose of the header pair at offsets 8/12 remains unresolved.

| File offset | Bytes | Native representation |
| ---: | ---: | --- |
| 0 | 4 | `.FP\0` signature |
| 4 | 4 | Version, two |
| 8, 12 | 4 each | Signed header x/y pair, `headerPoint`, without inferred purpose |
| 16 | 4 | Active path count, maximum eight |
| 20–51 | 32 | Eight DWORD point counts |
| 52–83 | 32 | Eight DWORD starting offsets, measured in points |
| 84–115 | 32 | Four signed x/y flag-placement positions |
| 116 onward | 8 per point | Signed x/y point payload |

All integers are little-endian. Total point count is the sum of counts for
active paths only; actual extent is `116 + totalPoints * 8`. Each active path
references its stored offset and count in the flat payload. Installed offsets
are canonical prefix sums, and inactive count/offset slots are zero. The reader
preserves those slots and does not require the installed convention.
See [original reader and caller evidence](../runtime/fp-flag-path-loading.md)
for executable hash, addresses and reproducible exports.

## Native API and policies

[assets/fp.hpp](../../assets/fp.hpp) exports `FpPoint`, `FpAsset`, `FpLimits`,
`FpResult`, `decodeFp(bytes)` and `loadFp(file)` through `mnm-fp-loader`.
Public types use the standard library and native asset file access; no widget
or build-specific reconstruction dependency is introduced. Results own the
header metadata, complete eight-slot arrays, four flag positions and flat
point payload. No source pointers survive.

Native policies, separate from original admission/error handling:

- Validate exact signature and version two; reject path counts above eight.
- Sum active counts in 64 bits and require the exact payload extent.
  Truncation and trailing bytes fail; inactive counts do not affect allocation.
- Validate every active `offset + count` in 64 bits against total points.
  Retain valid noncanonical, overlapping and reordered ranges rather than
  recomputing offsets. Preserve inactive words even when nonzero or extreme.
- Accept zero active paths and zero-length active paths with in-bounds offsets.
  No empty installed example exists; this is a native policy. Consumers must
  check for an empty path before requesting its last point.
- Defaults: 4 MiB input, 65,536 points and 4 MiB decoded point storage.
  Charge `sizeof(FpPoint)` per payload point before allocation; fixed header
  fields and vector overhead are outside that point-storage budget.
- Preserve signed 32-bit coordinates without screen bounds, scaling, clamping
  or interpolation. Point/count/offset metadata is not converted into UI state.
- Errors carry code, byte offset and detail, with backend input errors retained.
  Signed read-limit overflow is invalidArgument; allocation/length failures
  are limitExceeded. Parse failures return no partial asset.

The original reads the header as 84 bytes plus four eight-byte positions, then
reads the sum of active counts in eight-byte elements. It does not check exact
file extent or safely bound the eight path slots/ranges. Native checks are
therefore added safety policies, not claims of identical malformed-input behavior.

## Build, inspect and compare

```bash
cmake -S assets -B working/build/fp -DBUILD_TESTING=ON
cmake --build working/build/fp --parallel 4
ctest --test-dir working/build/fp --output-on-failure
working/build/fp/mnm-fp-inspect working/game-clean Interface/RealmViewer/Generic/Celtic_FlagPath_00.FP
python3 tests/test-fp-loader.py working/build/fp/mnm-fp-inspect \
    --installation working/game-clean \
    --report working/tests/fp-loader/installed-comparison.json
python3 tools/export-fp-support.py --decompile
```

The inspector closes its AssetFile before printing metadata, every count/offset,
all flag/header positions and every payload point. AssetStore supplies case
folding and containment; failures exit 2. No display or original process is
needed for parsing.

Native fixtures cover ownership, signed extremes, inactive metadata, valid
noncanonical/overlapping ranges, empty paths/lists, every truncated prefix,
extra bytes, invalid active ranges, signature/version/path capacity and input/
point/allocation limits. Huge point counts under permissive budgets still fail
extent validation before allocation. Five independent Python `struct` fixtures
compare every field, including all eight active paths, noncanonical offsets,
arbitrary unused words, case folding and missing/traversal failure paths.
The installed run records source hashes, rehashes inputs and verifies immutable
originals before/after, including failure.

All 26 asset CTests and both FP ASan/UBSan tests pass. Leak detection is disabled
because tracing prevents LeakSanitizer; prebuilt Qt is uninstrumented.
All 41 files/79 paths/15,950 points match independent decoding. All 2,927
immutable originals verify. Comparison report:
`working/tests/fp-loader/installed-comparison.json`; ordered canonical records
SHA-256 `2420926719b9b5cc36366655483957a8c8fdb585ef48f3cf21c9cc5f8f0cdd48`.
Tested inspector SHA-256
`d2afbc7cc7a0f5b3fa4be03b0efc66a7dd02227075197a3839b9d483bb44f4bf`
(GCC 16.2.1, C++17, Qt 6.11.2).

Remaining work: complete header-point semantics, flag slot allocation/release,
path selection and movement, coordinate scaling and drawing, then native Realm
Viewer consumption and bounded live comparison. No native writer, UI behavior
or live reader replacement is supplied. See
[coverage ledger](../runtime/coverage-ledger.md).
