# Clearance predicates: 0x004f44e0 and 0x004f45b0

## Evidence and confidence

Build: no-CD PE32 SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses are preferred-image VAs, not assumed live pointers. Read-only evidence:
`working/decompiled/search-support-vlwhfu2m/any_terrain.asm`, `tall_terrain.asm`,
`normalize_y_return.asm` and their hash-bearing manifest, reproduced by
`tools/export-search-support.py`.

| Routine | Exact code range |
| --- | --- |
| Any terrain | `0x004f44e0..0x004f45ae` |
| Tall-class terrain | `0x004f45b0..0x004f46a9` |
| Y normalization returning value | `0x004132e0..0x00413302` |

Confidence is high for fields, loop traversal, wrapping, global-map addressing
and return values: full assembly and isolated native i386 execution agree.
“Tall” describes classification threshold 8 in this clearance context, not a
confirmed gameplay category. Both predicates return true when a blocker is
found; true does not mean movement is accepted.

## Recovered rules and map source

Both take X, Y, Z and a pointer to the existing 22-byte parameters. Incoming ECX
is not used for map data. Unlike support and validity, these routines read the
fixed global map and global terrain records:

| Input | Use |
| --- | --- |
| `0x006c54dc` | Global 12-byte cell array base |
| `0x006cb942 + 4*Y` | Global row offset in cells |
| `0x006cb8c2 + 4*Z` | Global layer offset in cells |
| `0x006c5494/98` | Global XY dimensions for differences and normalization |
| `0x0065660c` | Terrain array, stride `0x164` |
| Parameters `+4` (`type_08`) | Signed footprint bound for both axes |

The starting XY are used as supplied, and Z never changes. Iterate Y outer,
X inner, beginning at the original XY. Each loop continues while signed wrapped
coordinate difference from its starting axis is less than parameter `+4`.
After an unsuccessful cell, increment X with DWORD arithmetic and normalize it
by the global X dimension. After finishing a row, increment Y and normalize it
using `0x004132e0`. The difference helpers are `0x0040e4e0` and `0x0040eb70`:
positive half-dimension ties remain positive. Starting XY is not normalized
before the first lookup.

`0x004f44e0` returns one at the first nonzero cell terrain WORD at `+0`; it does
not look up a terrain record. `0x004f45b0` reads a nonzero index's record
classification at `+0x94` and returns one if it is **unsigned** at least 8.
Negative DWORD classifications therefore qualify. A zero terrain index is
ignored by both predicates. An exhausted scan returns zero. Cell flags,
occupant IDs, terrain `+0xb0` eligibility and all other parameter fields are
unused. Neither routine clips Z, tests global layer count or writes categories.

For positive bounds not exceeding both half-dimensions, this visits the expected
wrapped E-by-E square in row order. Zero/negative bounds return zero without
cell/table access. Larger bounds must not be simplified to an ordinary finite
square: the signed wrapped difference can never reach the bound, causing the
original empty scan to repeat indefinitely. For example, a 2x2 map and bound 2
never escapes the X loop when no qualifying cell is found. Early qualifying
hits still return normally, including oversized bounds.

## Implementation and host cycle guard

`route_clearance.*` exposes `test_any_terrain`, `test_tall_terrain` and
`with_clearance_tests`. It uses the existing borrowed `CellValidityMapView`,
whose data here represents the **global** map, which may differ from movement's
incoming receiver map. Global dimensions must be positive and stable. Cells,
tables and terrain records must remain alive for captured callbacks; incomplete
storage throws as a host guard, not an engine branch. Initial Z is nonnegative
and tables/storage must cover every queried cell in the equivalence domain.

The traversal follows wrapped differences rather than assuming a bounded
square. If an axis completes a full period without reaching its bound or
finding a blocker, the host model throws `runtime_error`. This deliberately
terminates a condition where the original would keep cycling; it is not an
installed engine fix and is excluded from return-value equivalence. Tests
preserve original early-hit behavior for oversized bounds. A separate Y-cycle
host regression covers a valid X bound with an unreachable Y bound.

A complete movement helper setup can start without placeholder decision checks:

```cpp
support = with_cell_support(support, receiver_map);
validity = with_cell_validity_test(validity, receiver_map);
movement = with_clearance_tests(movement, global_map);
movement = with_record_query(movement, support);
movement = with_validity_test(movement, validity);
movement = with_boundary_test(movement); // captures installed support/validity
```

The receiver and global map views share engine XY dimensions and terrain table
in the native comparison, while their cell buffers differ. This tests the
addressing distinction. Boundary, layer-count and actual map/terrain storage
remain explicit data inputs. Connect the resulting helpers to creature acceptance
and standard neighbor generation with the previously documented adapters.

All decision helpers directly used by `0x004f3990` are now reconstructed.
This does not complete live search integration: extracting real map/object
snapshots, replacing generator setup data, and creature scalar/cost computation
at `0x005205b0` still remain. No live hook, executable patch or gameplay change
is installed. The next integration milestone is an end-to-end route replay with
captured world data and explicitly validated costs.

## Reproduce validation

```bash
cmake -S reconstruction/pathfinding -B working/build/pathfinding -DCMAKE_BUILD_TYPE=Release
cmake --build working/build/pathfinding --parallel 2
ctest --test-dir working/build/pathfinding --output-on-failure
./tests/test-reconstruction.sh
python3 tests/test-native-movement.py --component clearance
python3 tests/test-search-support.py
```

Validation on 2026-10-02: all thirteen CMake suites and ASan/UBSan checks pass.
Native comparisons match 51,204 any-terrain cases, 51,206 tall-terrain cases and
18,432 complete movement cases: 120,842 total.

Direct cases cover all 512 terrain-placement masks in a 3x3 area, two dimension
combinations, seams, negative/zero/one/two/three footprint bounds and classifications
0/7/8/16/-1. An independent finite-square oracle supplies expected results in
the terminating domain. Additional tests cover flag independence, any-terrain
with no terrain table, nonstandard global offsets, empty extent with absent
host tables/null cell base, oversized early hits, and unsigned indices
32768/65535. Native setup supplies inert tables for the empty scan, keeping the
cell base null so an unexpected dereference would fail.

Complete movement cases use all 64 support/blocker patterns with distinct receiver
and global cell buffers, four modes, capabilities, both footprint bounds, source
overrides, three vertical transitions and both ordinary/type-index-22 parameter
sets. Results and categories agree. Parameter, cell, terrain and receiver/global
context bytes are checked for writes.

The optional driver requires Linux x86 and `g++ -m32`. It verifies the executable
hash before making a disposable snapshot, maps it privately and initializes
fixture data. **No instructions are redirected in this component.** The original
movement routine and all seven helpers run unchanged: `0x004f3990`,
`0x004f4330`, `0x004f3320`, `0x004f41a0`, `0x004f46b0`, `0x004f3440`,
`0x004f44e0`, `0x004f45b0`, plus original coordinate routines. Direct clearance
calls use incoming receiver address 1, demonstrating that data comes from the
initialized global map rather than that receiver.

Two additional disposable native processes probe empty oversized scans, one for
each predicate. Both remain in the original scan until their one-second timeout;
the driver kills only those child processes and checks that this expected timeout
occurred. Static periodic-loop analysis plus these observations establish the
cyclic fixture behavior. They do not claim an observed hang in the live game.
The working executable hash is verified afterward; no on-disk executable is
patched or running game attached. All 14 raw decompilation artifacts and 1,686
instruction-byte chunks also pass integrity checks.
