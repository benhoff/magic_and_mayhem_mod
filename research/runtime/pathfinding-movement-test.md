# Lower movement test: 0x004f3990

2026-10-02. Exact no-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses are preferred VAs, not fixed live addresses. The installed game was
not launched or patched. Original media was not consumed.

## Evidence and confidence

`./tools/export-search-support.py` now exports `0x4f3990..0x4f419f` as
`movement_test.asm`. Evidence: `working/decompiled/search-support-82vv_6fx/`,
with artifact SHA-256 and `input_unchanged: true` in its manifest. The export
checks the executable hash before reading/writing evidence. The range ends
at the next function entry, excluding that function's switch-table data.

Static findings below have **high confidence** for the exact build. In
addition, an isolated native i386 reference test compares the reconstructed
function to the original machine code, substituting identical lower helpers.
It checks return values, category stores, and exact ordered helper queries for
more than 25,000 boundary/mode/clearance cases. This validates this routine
under those supplied inputs, not the real terrain helpers or gameplay.

## Interface

The receiver is map context ECX. Nine DWORD arguments are source XYZ,
destination XYZ, the 22-byte movement member, category output pointer, and
source record override; `ret 0x24` confirms the size. After locals/saved
registers, source is ESP `+0x28/+0x2c/+0x30`, destination
`+0x34/+0x38/+0x3c`, member `+0x40` (later reused), category `+0x44`, and
override `+0x48` (also reused).

The member layout is documented in
[creature acceptance](pathfinding-creature-acceptance.md). Names remain source
offsets: `type_0c` is member +0, `type_08` is +4, `object_104` is +8,
`type_44` is +0xc, type index is +0x11, and special-state byte is +0x15.
Gameplay labels such as flying, swimming, water level, or body height remain
hypotheses and are not used as recovered facts.

## Blocks and recovered behavior

| Address block | Behavior |
| --- | --- |
| 0x4f399e | Initialize output category to 5 |
| 0x4f39a5..0x4f39bb | Reject type index 22 when both special state and object +104 are zero |
| 0x4f39c1..0x4f3a4f | Test destination with 0x4f46b0, resolve its record with 0x4f4330, apply missing-record boundary restrictions |
| 0x4f3a55..0x4f3ae0 | Compute wrapped DWORD dz. Override -1 tests/resolves source; any other DWORD is used directly as its token, including zero |
| 0x4f3ae4..0x4f3b64 | Derive a special transition flag for member +0xc == 2 |
| 0x4f3b66..0x4f3bcc | Unless transition flag is set, reject when source 0x4f41a0 is false and destination 0x4f41a0 is true |
| 0x4f3bd2..0x4f3c16 | Apply mode-specific signed Z/dz restrictions |
| 0x4f3c18..0x4f3d03 | Select category 0/1/2/3/4 using source/destination record presence, boundary, transition, and member fields |
| 0x4f3d09..0x4f3ffa | Member +0 == 1 enables additional clearance queries |
| 0x4f3fff..0x4f418b | Member +4 == 1 enables two alternative intermediate-cell groups when signed squared distance exceeds 1 |

Let B be map receiver DWORD +0x7f0, L the DWORD-wrapped `B - member[+0]`, and
dz the wrapped destination Z minus source Z. For mode member +0xc == 2, the
transition flag can be set when either endpoint is in the signed band `(L,B]`:
an upward move with no source record sets it; a downward move with no
destination record sets it unless destination is exactly L and its cell at
Z+1 fails 0x4f46b0. Preserve the extra check and its order.

Mode zero rejects descending from the open band `(L,B)`. Mode two rejects
nonascending moves when source Z < L. The early missing-destination condition
rejects destination Z >= B when mode != 2, object +104 is zero, and special
state is zero. Record absence is not synonymous with a blocked cell.

Category assignment (after those preconditions):

| Source record | Destination record | Conditions | Category/result |
| --- | --- | --- | --- |
| present | present | — | 0 |
| present | absent | destination Z >= B and transition false | require object +104 or special state; also require dz == 1 or special state; then 2 |
| present | absent | other, mode == 2 | 0 |
| present | absent | other mode | object +104 required, then 1 |
| absent | either | source Z >= B and transition false | object +104 required; destination record requires dz == -1, then 3; absent destination gives 1 |
| absent | present | other | 0 |
| absent | absent | other, mode == 2 | 4 |
| absent | absent | other mode | object +104 required, then 1 |

Early rejection leaves the initial 5; late clearance or intermediate rejection
explicitly restores 5 after category assignment.

## Clearance details

Callbacks are deliberately named by address: A = 0x4f44e0, T = 0x4f45b0.
Their implementations/terrain semantics remain unresolved.

For equal Z below global `0x5e1780 - 1`, reject if
`T(source) && A(destination at Z+1)` OR
`A(source at Z+1) && T(destination)`, preserving short circuits.

For unequal Z, identify the lower/higher endpoints. With identical XY,
reject if A(higher endpoint). Otherwise reject when any ordered group passes:

1. A(source XY at higher Z) AND A(destination XY at higher Z).
2. A(lower endpoint at Z+1) AND **T(higher XY at lower Z)**.
3. Higher Z < global layer count - 1 AND A(lower XY at higher Z+1)
   AND T(higher endpoint).

The mixed-Z query in group 2 is confirmed by the native reference comparison;
using T(higher endpoint) there gave the same result in some tests but a different
helper query. A permanent regression distinguishes the two cases.

For member +4 == 1, the final groups use 0x4f46b0 and the same mixed coordinate
triples as `0x514360`: the three source-plane cells OR three destination-plane
cells, each group stopping at first failure. Wrapped XY squared distances and
dz squared use DWORD arithmetic and a signed <= 1 exemption. Repeated cells
are not deduplicated.

## Implementation and integration

`reconstruction/pathfinding/route_movement.hpp/.cpp` supplies `test_movement`.
`MovementHelpers` supplies map boundary/layer count, record resolver 0x4f4330,
and checks 0x4f46b0/0x4f41a0/0x4f44e0/0x4f45b0. The function implements all its
own branches; these remaining helpers are explicit engine dependencies.

`with_movement_test(acceptance, movement)` installs it under creature acceptance.
`with_standard_movement_test(neighbors, movement)` supplies the mode-nonzero
generator path, copying the exact descriptor +0x18 member. `NeighborRecord`
now carries `engine_token` for that path's source override. Supply the real
token, or zero for an absent setup record; never cast a host address to it.
Install the standard adapter before `with_creature_acceptance` to wire both
paths without replacing either with guessed legality.

## Reproduce validation

```bash
./tests/test-reconstruction.sh
python3 tests/test-search-support.py
cmake --build working/build/pathfinding --parallel 2
ctest --test-dir working/build/pathfinding --output-on-failure
python3 tests/test-native-movement.py
```

The normal five tests run on the host with ASan/UBSan. The last command is an
optional Linux x86 test requiring a working `g++ -m32` toolchain and permission
to execute 32-bit system calls (the Codex sandbox blocks them with SIGSYS).
It verifies the exact executable hash, creates a disposable verified snapshot,
maps its PE sections into a standalone i386 process, and redirects five helper
entrypoints to test callbacks. Helper prologues are checked before redirection;
the fixed mapping uses MAP_FIXED_NOREPLACE. The original movement code and
coordinate helpers execute unchanged. No loader/GUI/game process is started.
Temporary files are removed and the working executable hash is checked afterward.

The native comparison covers categories and early exits, 64 intermediate-cell
combinations, a boundary/mode/record-override matrix, 20,000 reproducible varied
lower-helper scenarios, and the mixed-Z regression. Adapter tests cover both
standard paths. Native fixtures provide pure lower-helper results and fixed
global snapshots; arbitrary live mutations, real terrain semantics, and
pointer lifetime validity are outside this validation. Host provider checks
and exceptions are not reconstructed engine error handling.

Follow-up: [the validity footprint model](pathfinding-validity-test.md) supplies
`0x004f46b0` through `with_validity_test`, while preserving the other movement
providers. Its lower `0x004f3440` rules remain explicit. An additional 3,072
native comparisons run the original movement and validity wrappers together.

Further follow-up: [cell validity and overhead clearance](pathfinding-cell-validity.md)
now supply `0x004f3440` under the recovered validity wrapper. An additional
1,536 native cases exercise all three original routines together. Record and
other boundary/clearance queries remain explicit movement dependencies.

Further follow-up: [support aggregation](pathfinding-record-query.md) supplies
`0x004f4330` under `MovementHelpers::record`. The result is a support DWORD,
not a record pointer; lower `0x004f3320` remains explicit. The native fixture
check bridge now returns integer EAX rather than C++ bool AL, fixing stale
upper-register bits observed during the new integration comparison.

Further follow-up: [cell support](pathfinding-cell-support.md) completes the
`0x004f4330 -> 0x004f3320` support chain and identifies terrain byte `+0xb0` mask
`0x08`. The new native comparison retains support, aggregation, validity and
movement routines, with only `0x004f41a0`, `0x004f44e0`, `0x004f45b0` as fixtures.

Further follow-up: [boundary/mode predicate](pathfinding-boundary-test.md) now
supplies `0x004f41a0`, including exact-boundary upper validity and post-query
parameter rereads. Install `with_boundary_test` after support/validity adapters.
The integrated native comparison now retains six original routines and uses
fixtures only for `0x004f44e0` and `0x004f45b0`.

Further follow-up: [clearance predicates](pathfinding-clearance-tests.md) supply
`0x004f44e0` and `0x004f45b0` from the fixed global map. All movement decision
helpers are reconstructed, and the new comparison runs all eight original
routines without instruction redirects. Their oversized wrapped scans can
cycle; the host model detects repetition while preserving terminating behavior.
Real world data and creature scalar/cost computation still require integration.
