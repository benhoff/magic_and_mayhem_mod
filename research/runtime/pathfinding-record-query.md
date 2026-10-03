# Support aggregation: 0x004f4330

## Evidence and confidence

Build: no-CD PE32 SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses refer to the preferred image base. Read-only evidence:
`working/decompiled/search-support-z_jy08m7/record_query.asm` and its hash-bearing
manifest, exported by `tools/export-search-support.py`. Function range:
`0x004f4330..0x004f44db`, excluding padding.

Confidence is high for branching, query order, wrapping, fallback and aggregation:
complete assembly and original i386 execution agree. This is a support-result
query, not a returned record pointer. The `MovementHelpers::record` name remains
for API compatibility; movement tests its result for nonzero. Specific gameplay
meanings of parameter modes and the boundary band remain unconfirmed. Follow-up: the [cell-support model](pathfinding-cell-support.md) now supplies
`0x004f3320` using map and terrain data.

## Recovered behavior

Four stack DWORDs: X, Y, Z and a pointer to the existing 22-byte parameter member.
ECX is preserved as the receiver for all lower `0x004f3320` calls. Parameters are
forwarded by the same pointer, with no local member construction or category write.

When parameter `+4` (`type_08`) is exactly 1, `0x004f4375` performs one lower query
at the original coordinates and returns its entire EAX unchanged. No coordinate
normalization or boundary access occurs in this branch.

Every other mode uses four footprint columns, ordered `(0,0)`, `(1,0)`, `(0,1)`,
`(1,1)`: Y outer, X inner. For each column:

1. Query at the requested Z, then at most Z-1 if the earlier query did not count
   as support. Negative Z is never queried in this branch.
2. X/Y plus the footprint offsets use DWORD addition and the original coordinate
   constructors on **every** query, including offsets zero. Global dimensions
   `0x006c5494/98` provide wrapping; the caller receiver still supplies map data.
3. Any nonzero lower result counts this column once, accumulates its local X/Y
   offsets and stops descending. A hit at the original Z also sets a shared
   “requested-level hit” flag.
4. After a zero lower result, parameter `+0x0c` (`type_44`) equal to 2 enables
   a fallback: the current Z must be at least receiver `+0x7f0` and less than
   signed DWORD `boundary + 2`. The fallback counts the column and stops descent,
   but does **not** set the requested-level-hit flag.

After all columns, with count N and offset sums X/Y, success requires:

```text
abs(2 * (2*X - N)) < N
abs(2 * (2*Y - N)) < N
at least one actual lower-query hit at the original Z
```

The counts and sums are at most 4, so these aggregate calculations cannot
overflow. Geometrically, balanced support means at least three columns, or
exactly the two diagonal columns. Adjacent pairs and isolated columns fail.
All support supplied only below the requested Z, or entirely by boundary-band
fallbacks, fails the last condition. The aggregate result is exactly 0 or 1.
A one-cell map dimension still retains all four logical column contributions
and repeated lower calls; columns are not deduplicated.

The branch mode is read once. The boundary-mode parameter and boundary value
are read after each zero lower result; the model preserves these rereads.
DWORD wrapping in Z-1 and boundary+2 is preserved. No map/layer-limit clipping
is present in this wrapper. Corrupt pointers and missing providers are outside
the model's domain; host setup guards are not recovered engine branches.

## Implementation and connection

`route_record_query.*` defines `RecordQueryHelpers`, `query_record`, and
`with_record_query`. The lower `query` callback supplies `0x004f3320`; the boundary
callback represents the caller's map `+0x7f0`, and dimensions represent the
engine's stable global XY dimensions. The movement and query providers must
represent the same map context and boundary. Their dimensions are checked.

```cpp
movement = with_record_query(movement, support);
acceptance = with_movement_test(acceptance, movement);
neighbors = with_standard_movement_test(neighbors, movement);
neighbors = with_creature_acceptance(neighbors, acceptance);
```

This supplies source/destination queries made by `0x004f3990` itself. A supplied
source override still skips the source query; the generator's existing setup
record callback remains a separate provider. The mode-zero validity and occupancy
chains can continue to use the previously reconstructed adapters. No installed
hook or gameplay modification is included. The [follow-up comparison](pathfinding-cell-support.md) executes original
`0x004f3320` and both validity routines together with this query and movement.

## Reproduce validation

```bash
cmake -S reconstruction/pathfinding -B working/build/pathfinding -DCMAKE_BUILD_TYPE=Release
cmake --build working/build/pathfinding --parallel 2
ctest --test-dir working/build/pathfinding --output-on-failure
./tests/test-reconstruction.sh
python3 tests/test-native-movement.py --component record-query
python3 tests/test-search-support.py
```

Validation on 2026-10-02: ten CMake suites and ASan/UBSan reconstruction checks
pass. Native comparison passes 55,306 direct cases and 9,216 movement cases:
64,522 total, plus a separate parameter-reread regression.

Direct tests use all 256 top/below support combinations across four logical
columns, three aggregate modes, both fallback branches, four boundary values,
three starting Z values and three dimension combinations. An independent corner
mask oracle asserts diagonal/three/four-column acceptance and the exact lower
query sequence. Tests also check raw EAX forwarding, unchanged coordinates in
the single-query branch, negative Z, INT_MIN/MAX and coordinate/boundary wrap.
The regression changes mode and boundary-mode parameters inside the lower
callback to verify their different read timing. Movement integration compares
result, category and complete lower-query/check order for different source
records, destination support, capabilities and parameter modes.

The optional native driver requires Linux x86 and `g++ -m32`. It verifies the
working executable hash before creating a disposable snapshot and maps that
copy privately. The target `0x004f4330` and calling `0x004f3990` remain unmodified.
Only `0x004f3320` and the other movement checks are redirected to fixtures after
prologue checks; original coordinate helpers remain intact. Direct calls use a
receiver distinct from fixed occupancy context, asserted by the lower stub.
The source hash is checked again after success. No running game is attached and
no on-disk executable is patched; immutable raw decompilation bytes are unchanged.

This comparison exposed an ABI defect in the shared test harness: a C++ `bool`
stub only guarantees AL, while the engine tests full EAX. False results could
leave nonzero upper bits and skip a query. The stub now returns integer 0/1 in
EAX. This changes the fixture bridge, not the reconstructed movement rules.
All five earlier native components pass again with the corrected bridge:
movement (25,841 cases), cell (35,201), occupancy (10,085), validity (38,273),
and cell-validity (101,180). Fixture support
rules still do not establish live terrain or search equivalence.
