# Native MPS map placement loading

Reviewed 2026-10-04. Scope: an owned native input reader and inspector for
version-1 map placement files. Schema evidence combines all installed inputs
and the hash-pinned No-CD original reader. This is offline loading, without
native placement application or live scene integration.

## Confirmed on-disk structure

All 683 installed files under `working/game-clean` total 389,488 bytes and
9,464 placement records. All use `MPS\0` and version one. Counts range from
zero to 79. All integers are little-endian; record words are retained as signed
32-bit values, including their original two's-complement bit patterns.
Confidence is high for boundaries, coordinates and named kinds; the remaining
per-kind semantics are incomplete.

| File offset | Bytes | Field |
| ---: | ---: | --- |
| 0 | 4 | `MPS\0` signature |
| 4 | 4 | Size-like header word; preserve as metadata |
| 8 | 4 | Version, one in every installed file |
| 12 | 4 | Record count |
| 16 onward | count × 40 | Ten-word placement records |

Every actual file length is `16 + 40 * count`. Every installed size-like word
instead equals `16 + 16 * count`, ranging from 16 to 1,280. The original reader
ignores that word for extent checks and uses the actual length and 40-byte
records. Its historical meaning is unknown; do not treat it as file size.
See [original reader/caller evidence](../runtime/mps-placement-loading.md)
for executable hash, addresses, recovered behavior and reproducible exports.

| Record offset | Bytes | Confirmed meaning/native representation |
| ---: | ---: | --- |
| 0, 4, 8 | 4 each | Section-local x/y/z; `position[0..2]` |
| 12 | 4 | Kind; named values from the original debug switch |
| 16–39 | 24 | Six kind-dependent words; `parameters[0..5]`, preserved without inferred semantics |

| Kind | Original debug label | Installed records |
| ---: | --- | ---: |
| 0 | Undefined | 0 |
| 1 | Friendly Wizard | 408 |
| 2 | Enemy Wizard | 261 |
| 3 | Multiplayer Wizard | 821 |
| 4 | Creature | 1,572 |
| 5 | Artifact | 3,101 |
| 6 | No named case; meaning unknown | 3,301 |

Unknown kinds and all parameters remain available to callers. A generic reader
must not discard kind 6 just because the original debug labels stop at five.
Coordinates are not validated against a particular map or transformed into
world coordinates. Empty lists are valid. No defaults/clamping, spawning,
rotation, duplicate removal or gameplay changes are performed.

## Native API and policies

[assets/mps.hpp](../../assets/mps.hpp) exports `MpsAsset`, `MpsPlacement`,
`MpsKind`, `MpsLimits`, `MpsResult`, `decodeMps(bytes)`, `loadMps(file)` and
`mpsKindName(kind)`. Link `mnm-mps-loader`. Public types and parsing use the
standard library and native asset file access; no widget, renderer or
build-specific reconstruction dependencies are introduced.

Each result owns the ordered placements, header size word, version and source
byte count. Unknown numeric kinds are retained using a signed enum underlying
type; their name is `unknown`. No source pointers survive. Any parse failure
returns an error instead of a partial asset.

Native policies, distinct from original error handling:

- Validate signature and version one. Other versions return unsupportedVersion.
- Treat header size word as opaque metadata; preserve arbitrary values rather
  than forcing the installed correlation as an unnecessary admission rule.
- Require exactly `16 + count * 40` bytes; reject short records and trailing data.
- Defaults: 4 MiB input, 65,536 records and 4 MiB decoded placement storage.
  Charge actual `sizeof(MpsPlacement)` per record against the allocation limit;
  vector/container overhead is outside that record-byte accounting.
- Compute extents in 64 bits and validate before allocation. Even permissive
  caller budgets do not bypass extent checks. Preserve unknown kinds, negative
  words and extreme signed values without guessed gameplay constraints.
- Errors include code, byte offset and detail; input errors retain the backend
  Error. Signed-read-limit overflow is invalidArgument; allocation/length
  failures are limit errors.

## Build, inspect and compare

```bash
cmake -S assets -B working/build/mps -DBUILD_TESTING=ON
cmake --build working/build/mps --parallel 4
ctest --test-dir working/build/mps --output-on-failure
working/build/mps/mnm-mps-inspect working/game-clean Realms/Celtic/Forest/CFsec01.mps
python3 tests/test-mps-loader.py working/build/mps/mnm-mps-inspect \
    --installation working/game-clean \
    --report working/tests/mps-loader/installed-comparison.json
python3 tools/export-mps-support.py --decompile
```

The JSON inspector closes its AssetFile before reporting metadata and every
coordinate, numeric/named kind and parameter word. It exits with status 2 on
failure and needs no display server or original executable. AssetStore supplies
case-insensitive path resolution and containment.

Native fixtures cover known/unknown kinds, extreme signed parameters, owned
storage, opaque size metadata, empty lists, every truncation of a valid file,
extra records/bytes, versions/signatures and input/record/allocation limits.
Four independent Python struct fixtures compare every field and exercise
case folding, negative coordinates and arbitrary size words. Invalid/missing
files and parent traversal must fail. The optional installed run records
source hashes and verifies original media before/after, including on failure.

All 19 asset CTests pass. Both MPS tests pass under AddressSanitizer and
UndefinedBehaviorSanitizer with leak detection disabled because tracing prevents
LeakSanitizer. The installed report is
`working/tests/mps-loader/installed-comparison.json`. All 683 installed files
and 9,464 complete records match the independent decoder, including kind 6.
All 2,927 immutable originals verify before/after. The ordered report records,
encoded as JSON with sorted keys and compact separators, have SHA-256
`f1a654b861a15244b8f817ec07c39657758a827f1df1d7ade5ec964a34f0e121`. The tested native inspector SHA-256 is
`efb420b4e73513d6cfe7151bf2cd9b4f0d92a4cb96cc231453d0ff39b09352a1` (GCC 16.2.1, C++17, Qt 6.11.2 Core backend).

Remaining work: identify kind 6 and all per-kind parameters, bind catalogs,
recover/validate placement transformations and entity creation, and exercise
native world consumers and live loading. Offline parsing does not establish
original runtime equivalence. See the [coverage ledger](../runtime/coverage-ledger.md).
