# Validity footprint wrapper: 0x004f46b0

## Evidence and confidence

Build: no-CD PE32 SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses are preferred-image VAs, not verified live pointers.
Read-only evidence: `working/decompiled/search-support-cllv4afs/validity_test.asm`
and its hash-bearing manifest, reproduced by `tools/export-search-support.py`.
The exact function range is `0x004f46b0..0x004f47c4`, excluding alignment padding.

Confidence is high for control flow, wrapping, parameter forwarding and receiver
preservation: full assembly and isolated original i386 execution agree.
“Validity” follows its role in `0x004f3990`, not a confirmed gameplay label.
Follow-up: the [cell-validity model](pathfinding-cell-validity.md) now supplies
the rules at `0x004f3440` through `with_cell_validity_test`. This milestone
reconstructs their footprint wrapper, not those rules or live terrain behavior.

## Recovered behavior

The entry takes X, Y, Z and the existing 22-byte parameter-member pointer as
four stack DWORDs (`ret 0x10`). It saves incoming ECX and uses that receiver for
**every** call to `0x004f3440`. This differs from `0x004f47d0`, which calls the
occupancy routine with fixed receiver `0x006c5490` and an additional object DWORD.

| Call site | Coordinates passed to 0x004f3440 |
| --- | --- |
| `0x004f46e2` | Original X, Y, Z |
| `0x004f472a` | Original X, wrapped Y+1, original Z |
| `0x004f4768` | Wrapped X+1, original Y, original Z |
| `0x004f47ac` | Wrapped X+1, wrapped Y+1, original Z |

The base call always runs. A failure immediately returns zero. After success,
`0x004f46f0` reads parameter `+4` (`type_08`). Any value other than 2 returns
one; value 2 enables the remaining three calls in the listed order, stopping at
the first failure. The same parameter pointer is passed every time.

XY increments are DWORD operations followed by the original coordinate
constructors, which read the global dimensions `0x006c5494/98`. The dimensions
are global even when the receiver is a different map context. Initial XY and
all Z values are copied without normalization or local bounds checks. All four
calls remain present for one-cell dimensions, including duplicates. The routine
does not construct parameters, take an object ID, or write a category.

## Implementation and connection

`route_validity.*` exposes `ValidityHelpers`, `test_validity`, and
`with_validity_test`. The traversal is shared with the already verified
`test_cell` model because both wrappers have identical order, wrapping and
post-base-call parameter reread. A host adapter discards the synthetic object
argument used by that shared implementation; it is never sent to `0x004f3440`.
The lower validity callback (now supplied by `with_cell_validity_test`) represents the caller's map context, separately
from the occupancy callback. Positive, stable global XY dimensions are required;
missing providers and dimension mismatches throw as host setup guards, not
recovered engine branches. No host callback is cast to a game calling convention.

```cpp
movement = with_validity_test(movement, validity);
acceptance = with_movement_test(acceptance, movement);
neighbors = with_standard_movement_test(neighbors, movement);
neighbors = with_creature_acceptance(neighbors, acceptance);
```

The adapter replaces only `MovementCheck::check_46b0`; existing `0x004f41a0`,
`0x004f44e0`, `0x004f45b0` callbacks and the record provider remain installed.
The occupancy chain continues to use `with_occupancy_test` and `with_cell_test`.
The [follow-up comparison](pathfinding-cell-validity.md) runs the lower cell
validity rules together with both original wrappers.
No installed hook or gameplay modification is included.

## Reproduce validation

```bash
cmake -S reconstruction/pathfinding -B working/build/pathfinding -DCMAKE_BUILD_TYPE=Release
cmake --build working/build/pathfinding --parallel 2
ctest --test-dir working/build/pathfinding --output-on-failure
./tests/test-reconstruction.sh
python3 tests/test-native-movement.py --component validity
python3 tests/test-search-support.py
```

Validation on 2026-10-02: all eight CMake suites and ASan/UBSan reconstruction
checks pass. Native comparisons pass 35,201 footprint cases covering all 16
provider-result masks, five parameter values, nine dimension combinations,
canonical XY positions, seams, negative XY normalization and unbounded Z values.
They compare results, exact query order, all parameter bytes and pointer identity.
A separate regression verifies that parameter `+4` is read after the base call.
Native direct calls use receiver `0x006c4bd0`, distinct from the fixed occupancy
context; the lower stub asserts that every call preserves it.

Another 3,072 comparisons execute original `0x004f3990` with its original
`0x004f46b0` calls, across all 512 blocked-cell patterns in a 3x3 fixture, three
parameter modes and two source-record override choices. They compare result,
category and ordered lower-helper/record queries, testing destination, source
and intermediate validity checks and retaining the other movement providers.
These establish equivalence with fixture dependencies, not live-map equivalence.

The optional native driver requires Linux x86 and `g++ -m32`. It verifies the
working executable hash before copying it to a disposable directory, maps the
snapshot privately and checks helper prologues before redirects. This test
leaves `0x004f46b0` and `0x004f3990` unmodified, redirects `0x004f3440` to an
ABI-correct fixture, and redirects the remaining lower movement helpers as in
the existing movement comparison. Original coordinate constructors remain intact.
The driver checks the source hash again after success. No running game is
attached and no on-disk executable is patched; raw baseline artifacts are unchanged.
