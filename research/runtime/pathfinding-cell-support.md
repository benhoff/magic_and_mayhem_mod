# Cell support predicate: 0x004f3320

## Evidence and confidence

Build: no-CD PE32 SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses are preferred-image VAs, not verified live pointers. Read-only evidence:
`working/decompiled/search-support-bsqtq5it/cell_support.asm` and its hash-bearing
manifest, exported by `tools/export-search-support.py`. The function occupies
`0x004f3320..0x004f3439`, excluding padding, and calls no deeper functions.

Confidence is high for indexing, flags, classification comparisons, below-cell
fallback and return values: full assembly and isolated original i386 execution
agree. “Support” describes its role under `0x004f4330`; specific terrain, creature
and flag gameplay meanings remain hypotheses. Actual map data is still required;
fixture comparisons do not establish live terrain or search equivalence.

## New terrain field

The cell and terrain layout is shared with
[cell validity](pathfinding-cell-validity.md): 12-byte cells, unsigned terrain
WORD at cell `+0`, flags WORD at `+0x0a`, terrain array global `0x0065660c`, record
stride `0x164`, and classification DWORD at record `+0x94`.

At `0x004f33a6`, the engine tests terrain record byte `+0xb0` with mask `0x08`.
`TerrainValidityRecord` now exposes `flags_b0`, with an offset assertion. The
remaining opaque range is split around that byte without changing record size
or the classification offset. This is a confirmed byte test, not a gameplay
label for the flag.

## Ordered rules

Four stack DWORDs: X, Y, Z and the existing 22-byte parameter pointer. Incoming
ECX is the caller's map receiver. The starting address uses its row/layer offset
tables and cell base, with the same DWORD cell indexing as the other map helpers.

Cache parameter byte `+0x10` (`unknown_10`) once. In either tested cell, a zero
cached byte makes flag `0x20` with flag `0x10` clear reject the query. Having both
bits set passes this restriction. A nonzero cached byte bypasses that restriction
only, not the other rules below.

For the current cell:

1. Apply the cached-byte restriction, then reject flag `0x4000`.
2. If its terrain index is nonzero and flag `0x80` is clear, read the terrain
   record. Classification must be **unsigned** below 16; negative DWORD values
   fail this comparison too.
3. A qualifying record supports only when `flags_b0 & 0x08` is nonzero or the
   parameter DWORD at `+0x11` (`type_index`) equals 22. Return exactly one or zero.
   Failure on this path never falls back to a supporting cell below.
4. Only a zero terrain index or flag `0x80` enters the below-cell fallback.

For the fallback:

1. If starting Z is at most zero, return zero without reading below.
2. Subtract `12 * receiver.plane_stride` from the current pointer. This is one
   plane below, not a fresh lookup of the Z-1 offset table.
3. Apply the cached-byte restriction. Then flag `0x4000` immediately supports,
   returning one regardless of terrain index/classification. This is the inverse
   of its effect on the current cell.
4. Otherwise require a nonzero terrain index and flag `0x80` clear.
5. Require classification **exactly** 16, followed by the same terrain flag
   `0x08` or parameter type-index 22 eligibility test. Any failure returns zero.

No parameter extent, layer count, boundary band, XY normalization or category
output is used here. The full result is always integer 0/1. Current terrain can
support at Z=0; only the below fallback is disabled there. No cells, records or
parameters are modified. Negative-Z memory indexing is not validated by this
milestone; the host view requires a valid nonnegative initial table index.

## Implementation and integration

`route_cell_support.*` provides `test_cell_support` and `with_cell_support`,
reusing the borrowed `CellValidityMapView` for cell and terrain storage. Tables,
cells and records must remain alive and stable for all captured callbacks.
Incomplete storage throws as a host guard, not a recovered engine branch.
Equivalence claims exclude corrupt pointers and arbitrary wrapped addresses.

```cpp
support = with_cell_support(support, cell_map);
movement = with_record_query(movement, support);
validity = with_cell_validity_test(validity, cell_map);
movement = with_validity_test(movement, validity);
acceptance = with_movement_test(acceptance, movement);
neighbors = with_standard_movement_test(neighbors, movement);
neighbors = with_creature_acceptance(neighbors, acceptance);
```

The `0x004f4330 -> 0x004f3320` support chain now has reconstructed decisions
and explicit map data instead of a lower decision callback. The supplied query
boundary callback still represents the same receiver context used by movement.
Generator setup records and live map extraction remain separate dependencies.
There is no installed hook or gameplay modification. Follow-up: [the boundary model](pathfinding-boundary-test.md) now supplies
`0x004f41a0`. Only `0x004f44e0` and `0x004f45b0` remain movement decision helpers.

## Reproduce validation

```bash
cmake -S reconstruction/pathfinding -B working/build/pathfinding -DCMAKE_BUILD_TYPE=Release
cmake --build working/build/pathfinding --parallel 2
ctest --test-dir working/build/pathfinding --output-on-failure
./tests/test-reconstruction.sh
python3 tests/test-native-movement.py --component cell-support
python3 tests/test-search-support.py
```

Validation on 2026-10-02: eleven CMake suites and ASan/UBSan reconstruction
checks pass. Native comparisons agree in 246,288 direct cases, 4,096 aggregation
cases and 3,072 movement cases: 253,456 total.

Direct cases cover all low cell-flag bytes and selected high bytes spanning
`0x4000`, current/below differences, zero/nonzero terrain indices, classifications
0/15/16/17/-1, terrain-flag eligibility, type-index exemptions, and zero/nonzero
parameter bytes. Additional checks cover every terrain flag byte, ignored
parameters, Z=0, null terrain tables for lookup-bypass paths, failed-current
terrain without fallback, cached restriction precedence, nonstandard offset
tables/stride, and unsigned terrain indices 32768/65535. Direct and aggregation
checks verify unchanged cell, terrain, context and parameter bytes.

Aggregation tests cover all combinations of four current and four below-cell
support placements, one-cell/two-by-two modes, boundary fallbacks, diagonal
balance and XY seams. Movement comparisons exercise source overrides, capability
flags, categories, current/below support and intermediate validity. They compare
results and categories while verifying unchanged cells, parameters and receiver
context; the three remaining movement checks are fixtures.

The optional driver requires Linux x86 and `g++ -m32`. It checks the working
executable hash before making a disposable snapshot and maps it privately.
All five target routines remain original: `0x004f3320`, `0x004f4330`,
`0x004f3440`, `0x004f46b0`, `0x004f3990`. Direct/aggregation calls use a separate
receiver context with supplied map/terrain data. Movement uses the initialized
private global map context. Only `0x004f41a0`, `0x004f44e0`, `0x004f45b0` are
redirected after prologue checks, with integer EAX returns. Original coordinate
constructors remain intact. No running game is attached or on-disk executable
patched; the source hash is checked after success. The immutable decompilation
baseline also passes all 14 artifact and 1,686 byte-chunk integrity checks.
