# Cell footprint test: 0x004f47d0

## Evidence and confidence

Build: no-CD PE32 SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses are preferred-image VAs, not verified live pointers.
Read-only support export: `working/decompiled/search-support-z01phlc9/cell_test.asm`
and its hash-bearing `manifest.json`. Export via `tools/export-search-support.py`.
The immutable raw decompilation baseline remains unchanged.

Confidence is high for the control flow, arguments, short-circuit order and XY
wrapping: complete assembly plus an isolated native i386 comparison agree.
Calling this a footprint describes the geometry. A specific gameplay meaning
for parameter `+4 == 2` (such as creature size) remains a hypothesis.
Follow-up: `0x004f3550` is now reconstructed as a vertical occupancy scan; see
[occupancy findings](pathfinding-occupancy-test.md). Terrain checks are elsewhere.

## Recovered behavior

The entry receives five stack DWORDs: X, Y, Z, creature DWORD `+0`, and a pointer
to the existing 22-byte movement parameter member. Each lower call receives
those arguments with receiver `0x006c5490`; the incoming receiver is unused.

| Call site | Coordinates passed to 0x004f3550 |
| --- | --- |
| `0x004f4808` | Original X, Y, Z |
| `0x004f4854` | Original X, wrapped Y+1, original Z |
| `0x004f4896` | Wrapped X+1, original Y, original Z |
| `0x004f48de` | Wrapped X+1, wrapped Y+1, original Z |

The first check always runs. If it fails, return zero immediately. Only after
it succeeds does `0x004f4816` read parameter `+4` (`type_08`). Values other than
2 return one. Value 2 enables the remaining three checks, stopping at the first
failure. All four must pass. The same object DWORD and parameter pointer are
forwarded every time; this routine does not construct a new member.

XY increments are DWORD `INC` operations followed by coordinate constructors
`0x0040e290` and `0x0040e8a0`. Z is copied unchanged, with no local Z bounds
check. Initial XY are copied unchanged too. Even a one-cell dimension retains
all four calls, including duplicates. No category output is written.

## Implementation and validation

`reconstruction/pathfinding/route_cell.*` implements `test_cell` and
`with_cell_test`. Install the latter into `CreatureAcceptanceHelpers`, then use
`with_creature_acceptance` to connect the generator. `CellHelpers::check` supplies
the `0x004f3550` dependency, now supplied by `with_occupancy_test`. Dimensions must be stable and positive
in XY; missing providers and dimension mismatches throw as host setup guards,
not recovered engine branches.

```bash
cmake -S reconstruction/pathfinding -B working/build/pathfinding -DCMAKE_BUILD_TYPE=Release
cmake --build working/build/pathfinding --parallel 2
ctest --test-dir working/build/pathfinding --output-on-failure
./tests/test-reconstruction.sh
python3 tests/test-native-movement.py --component cell
python3 tests/test-search-support.py
```

Validation on 2026-10-02: all six CMake suites and ASan/UBSan reconstruction
checks pass. The native comparison passes 35,201 cases covering all 16 provider
result masks, five parameter values, nine dimension combinations, all their
canonical XY positions, several Z values including out-of-range ones, and
negative XY normalization. It compares return values and exact query order,
object arguments, parameter contents and pointer identity. A separate regression
confirms that the branch parameter is read after the first callback; integration
checks verify wrapped destination footprints, category preservation and the
special-move bypass.

The optional native harness requires Linux x86 and a working `g++ -m32` toolchain.
Its Python driver verifies the executable hash before copying it. The harness
maps that disposable copy privately at its preferred image base and redirects
`0x004f3550` to a fixture after verifying the helper prologue. It retains the
original footprint routine and coordinate helpers. Shared loader setup also
redirects the five lower movement helpers used by the existing movement test;
those are not called by this footprint routine. No game process is attached,
no on-disk executable is patched, and the driver verifies the input hash again.
This proves the recovered routine under fixture dependencies, not live terrain
or collision equivalence. The [follow-up occupancy comparison](pathfinding-occupancy-test.md) also runs
both original routines together without redirecting `0x004f3550`.
