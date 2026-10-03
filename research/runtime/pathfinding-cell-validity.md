# Cell validity and overhead clearance: 0x004f3440

## Evidence and confidence

Build: no-CD PE32 SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses are preferred-image VAs. Read-only support evidence is
`working/decompiled/search-support-w2_1t3k1/cell_validity_test.asm` with its
hash-bearing manifest, exported by `tools/export-search-support.py`.
The function range is `0x004f3440..0x004f354c`, excluding alignment padding.

Confidence is high for fields, comparisons, indexing, short-circuit behavior and
scan limits: complete assembly and isolated native i386 execution agree.
Terrain/classification and clearance describe their observed role; the gameplay
meanings of individual flags and classification numbers remain unresolved.
This routine calls no deeper functions. It still requires actual map and terrain
data; fixture equivalence does not establish live-game behavior.

## Inputs and recovered structures

Four stack DWORDs: X, Y, Z and a pointer to the existing 22-byte parameter member.
ECX is the caller's map context. Initial addressing uses the same row/layer
cell-offset tables, cell base at receiver `+0x4c`, and plane stride at receiver
`+0x10` documented in [occupancy findings](pathfinding-occupancy-test.md).

| Field | Observed use |
| --- | --- |
| Cell `+0x00`, WORD | Unsigned terrain record index |
| Cell `+0x0a`, WORD | Full 16-bit flags, including byte `+0x0b` |
| Global `0x0065660c` | Terrain record array base |
| Terrain record stride | `0x164` bytes (356) |
| Terrain record `+0x94`, DWORD | Classification used only for the base cell |
| Parameter `+0x00`, DWORD | Requested vertical extent (`type_0c`) |
| Parameter `+0x10`, byte | Nonzero bypasses the `0x20` without `0x10` restriction |
| Global `0x005e1780` | Signed upper layer limit |

`TerrainValidityRecord` is a packed minimum layout with classification at `+0x94`
and the full observed array stride. The [cell-support follow-up](pathfinding-cell-support.md)
also exposes `flags_b0` at `+0xb0`; remaining bytes retain opaque names.
`OccupancyCell` remains compatible: the first WORD is read from `unknown_00`,
and `flags_0a` plus `unknown_0b` form the flags WORD. The occupant at `+0x04`
is not examined by this function.

## Ordered rules

1. Read the starting cell's flags. Flag `0x4000` immediately rejects it.
2. Default classification to zero. Only if the first WORD is nonzero and flag
   `0x80` is clear, read that terrain record's classification. Exactly 16 rejects
   the base cell. Zero terrain index and flag `0x80` bypass the lookup entirely.
3. Cache parameter byte `+0x10`. If it is zero, flag `0x20` with flag `0x10`
   clear rejects the base cell. Having both bits set passes this restriction.
4. If classification is **unsigned** at least 8 and requested extent is exactly
   1, use extent 2. Otherwise use the requested extent unchanged. Signed-negative
   classifications therefore also promote extent 1, unless rejected earlier.
5. Form the signed DWORD end layer `Z + extent`. If it exceeds the global layer
   limit, reject. This differs from occupancy's scan clipping.
6. Starting one plane above the base, scan layers `Z+1 .. end-1`. Each rejects
   on flag `0x4000`, a nonzero terrain WORD with flag `0x80` clear, or the cached
   byte's `0x20` without `0x10` restriction. Upper terrain classifications are
   never loaded: a nonzero terrain index is enough unless `0x80` bypasses it.
7. Return one when all checks pass; any rejection returns zero.

Initial XY/Z are not normalized or bounds-checked here. Above-cell addresses
advance by the receiver's plane stride, not by re-reading layer offsets.
Signed DWORD overflow is preserved in the end calculation. Zero and negative
extents still run all base-cell rules and the end-limit comparison, but can
produce an empty overhead scan. No category, cell or parameter bytes are written.

## Implementation and integration

`route_cell_validity.*` implements `test_cell_validity` and
`with_cell_validity_test`, using a borrowed `CellValidityMapView`. Cell storage,
offset tables and terrain records must remain alive and stable for all captured
callbacks. Missing/incomplete storage throws as a host guard, not an engine
branch; the model does not claim equivalence for corrupt pointers or arbitrary
wrapped addresses. Tests use valid offset tables and nonnegative initial Z.

```cpp
validity = with_cell_validity_test(validity, cell_map);
movement = with_validity_test(movement, validity);
acceptance = with_movement_test(acceptance, movement);
neighbors = with_standard_movement_test(neighbors, movement);
neighbors = with_creature_acceptance(neighbors, acceptance);
```

This completes the decision rules in the `0x004f46b0 -> 0x004f3440` validity
chain. The record query `0x004f4330` and checks `0x004f41a0`, `0x004f44e0`,
`0x004f45b0` remain movement dependencies. Follow-up: [support aggregation](pathfinding-record-query.md) now supplies
`0x004f4330` under `MovementHelpers::record`. Its lower `0x004f3320` remains
the next dependency. No installed hook or gameplay changes are included.

## Reproduce validation

```bash
cmake -S reconstruction/pathfinding -B working/build/pathfinding -DCMAKE_BUILD_TYPE=Release
cmake --build working/build/pathfinding --parallel 2
ctest --test-dir working/build/pathfinding --output-on-failure
./tests/test-reconstruction.sh
python3 tests/test-native-movement.py --component cell-validity
python3 tests/test-search-support.py
```

Validation on 2026-10-02: all nine CMake suites and ASan/UBSan reconstruction
checks pass. The native comparison matches 99,068 direct cases, 576 cases
through the original validity footprint and 1,536 through the original movement
and footprint chain: 101,180 total.

Direct tests cover all low flag bytes and selected high flag bytes spanning
`0x4000` and ignored high bits, base/upper differences, classification 0/7/8/16/-1,
zero/nonzero parameter bytes, extent promotion, negative/zero/positive extents,
top limits, DWORD overflow and nonstandard row/layer offsets and stride.
Lookup-bypass tests use a null terrain table, and separate tests exercise
unsigned terrain indices 32768 and 65535. Integration covers map seams, one-cell
and 2x2 footprints, overhead blockers, source overrides, category resets and
intermediate movement validity. Fixture cell, terrain, context and parameter
bytes are checked for writes. Movement comparisons check results and categories;
remaining lower movement dependencies are supplied as fixtures.

The optional driver requires Linux x86 and `g++ -m32`. It hash-checks the working
executable before creating a disposable snapshot and maps it privately. This
comparison leaves all three target routines (`0x004f3440`, `0x004f46b0`,
`0x004f3990`) unmodified, supplies receiver/global map and terrain storage, and
redirects only the remaining lower movement dependencies after checking their
prologues. Direct/footprint tests use a separate receiver context, preserving
the distinction from fixed occupancy context. Original coordinate helpers
remain intact. No game process is attached, no on-disk executable is patched,
and the source hash is checked again after success. The 14 immutable raw
baseline artifacts also pass their byte-integrity checks.
