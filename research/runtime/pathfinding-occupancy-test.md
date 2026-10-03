# Vertical occupancy exclusion: 0x004f3550

## Evidence and confidence

Build: no-CD PE32 SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
VAs refer to preferred image base `0x00400000`, not verified live addresses.
Read-only evidence: `working/decompiled/search-support-1z1d43r7/occupancy_test.asm`
and its hash-bearing `manifest.json`, exported by `tools/export-search-support.py`.
The function ends at `0x004f35d0`; the much longer routine starting there is a
separate function and is not part of this reconstruction.

Confidence is high for the byte fields, indexing, comparisons and control flow:
complete assembly and isolated original i386 execution agree. “Occupant ID” and
“vertical extent” describe the observed exclusion operation; specific gameplay
classes represented by the two low flag bits remain unresolved. Terrain is not
checked here. This function calls no deeper helpers.

## Map inputs and record layout

| Input | Recovered use |
| --- | --- |
| Receiver `+0x4c` | Base pointer to 12-byte cell records |
| Receiver `+0x64b2 + 4*Y` | Row offset, in cells |
| Receiver `+0x6432 + 4*Z` | Layer offset, in cells |
| Receiver `+0x10` | Plane stride, in cells, for upward stepping |
| Global `0x005e1780` | Signed layer limit, captured once when the extent is nonempty |
| Parameter member `+0` | Signed scan extent, sourced from creature type `+0x0c` |
| Cell `+0x04` | Unsigned 16-bit occupant token; `0xffff` is exempt |
| Cell `+0x0a` | Byte flags; only mask `0x03` matters |

The initial cell address is formed unconditionally as
`base + 12 * (row_offset[Y] + layer_offset[Z] + X)` using DWORD arithmetic.
The scan end is signed DWORD `Z + extent`. Starting at Z, while the current
layer is less than both that end and the captured global layer limit:

1. If neither low flag bit is set, pass this cell.
2. Otherwise read the occupant WORD. Token `0xffff` passes.
3. Zero-extend any other token and compare it with the **whole** creature DWORD
   `+0`. Equal tokens pass; a mismatch immediately returns zero.
4. Increment the layer and advance the cell pointer by `12 * plane_stride`.

Exhausting the loop returns one. Zero/negative extents and a start at or above
the global limit succeed without reading cell flags. Signed DWORD overflow in
the end calculation can also produce an empty scan. There is no XY wrapping,
negative-Z guard, bounds check, terrain lookup, category store, or parameter
write in this routine. The caller `0x004f47d0` supplies footprint wrapping.
Only parameter `+0` is used; its other 18 bytes do not affect this function.

## Implementation and connection

`route_occupancy.*` defines the packed 12-byte `OccupancyCell`, a borrowed
`OccupancyMapView`, `test_occupancy`, and `with_occupancy_test`. Offset tables and
stride are explicit, so storage need not be inferred from width and height.
The backing cells and tables must remain alive and stable for the view and all
callbacks that capture it. Pointer/count checks throw for incomplete host
storage; these are host guards, not discovered engine branches. Equivalence
claims cover valid table/storage inputs, not arbitrary wrapped memory addresses
or corrupt map pointers. The native fixture uses positive/zero Z with valid
offset-table entries; negative Z memory access is not validated.

Connect the recovered occupancy and footprint rules to creature acceptance:

```cpp
cells = with_occupancy_test(cells, map_view);
acceptance = with_cell_test(acceptance, cells);
neighbors = with_creature_acceptance(neighbors, acceptance);
```

Object-state and lower movement/terrain providers still need to be supplied.
No live hook or installed executable replacement is included.

## Reproduce validation

```bash
cmake -S reconstruction/pathfinding -B working/build/pathfinding -DCMAKE_BUILD_TYPE=Release
cmake --build working/build/pathfinding --parallel 2
ctest --test-dir working/build/pathfinding --output-on-failure
./tests/test-reconstruction.sh
python3 tests/test-native-movement.py --component occupancy
python3 tests/test-search-support.py
```

Validation on 2026-10-02: seven CMake suites and ASan/UBSan checks pass.
The native comparison agrees in 9,509 direct occupancy cases and 576 integrated
footprint cases. Tests cover all 256 flag bytes, exempt/self/foreign occupants,
full-DWORD rather than truncated owner comparison, negative/zero/positive scan
extents, clipping and empty scans, signed end overflow, nonstandard offset
tables and plane stride, early rejection, ignored parameters and opaque cell
bytes. Integration tests exercise one-cell and 2x2 footprints across both map
seams, upward blockers, category preservation on acceptance, and category reset
on rejection. These are fixture-based comparisons, not live map captures.

The optional native driver requires Linux x86 and a working `g++ -m32` compiler.
It hash-checks the working executable before creating a disposable snapshot,
maps that copy privately, and runs the original `0x004f3550` and `0x004f47d0`
bytes without redirecting either routine. The map receiver, offset tables and
cell storage are supplied as native fixture memory; the global layer limit and
footprint map receiver are initialized in the private mapping. Results and
unchanged parameter/map/context bytes are checked. Shared loader setup redirects
five unrelated lower movement helpers, none of which these routines call.
No game process is attached and no on-disk executable is patched. The source
hash is checked again afterward; raw decompilation artifacts remain unchanged.

The cell-footprint chain now has no unresolved decision helpers. Follow-up: [the validity wrapper](pathfinding-validity-test.md) now supplies
`0x004f46b0`, the initial check used by `0x004f3990`. Its lower `0x004f3440`
cell rules remain to be reconstructed.
