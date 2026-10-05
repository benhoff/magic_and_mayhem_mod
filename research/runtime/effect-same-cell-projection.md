# Same-cell effect movement and lighting input

Build: no-CD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

This chunk reconstructs a bounded path through the complete movement routine
at `004883f0`: effect types 3, 13, 22, 24 and 36, no creator, empty terrain
occupancy, no nearby creatures, unchanged cell coordinates, and metadata kind
0 or 68 with zero distance allowance. Trajectory initialization remains authored.
These are isolated input conditions, not facts about every installed instance
of those types or a replacement for their original initialization.

The original reads an iteration count from record `+0x50` (parameter 9).
Each iteration copies current fine units `+0x14/+0x18/+0x1c` to previous units
`+0x1ea/+0x1ee/+0x1f2`, then calls the previously compared `004df500` with
trajectory receiver `+0x128`, coordinate parameters `+0x2c/+0x30/+0x34`, and
counter `+0x1ce`. Z outside `0..mapLayers*16-1` exits with result 2 after some
original mutation. X/Y add one period if negative and subtract one if at least
the period. The selected native path admits only one-period excursions.

The resulting parameters become current fine units. Logical shifts 5, 5 and
4 derive cell coordinates and write recount metadata `+0x1c2/+0x1c6/+0x1ca`.
When these cells equal the record's existing `+8/+0x0c/+0x10`, the original
skips cell unlink/insertion, terrain pointer replacement and cache updates.
Thus cell identity, chain links, cell flags, initial cell caches and current
stamping positions remain unchanged in this scope.

Subcell coordinates are `(X>>2)&7`, `(Y>>2)&7`, `(Z>>2)&3`. A change from
cached `+0x1aa/+0x1ae/+0x1b2` invokes the real terrain bit lookup at `004e10e0`
using the record's terrain pointer. Empty occupancy returns zero and stores
the new subcell coordinates. Kind 68 skips creature gathering/checking; kind
0 exercises the actual 27-cell neighbour traversal with empty creature heads.
An occupied terrain bit or creature collision leads to additional behavior
outside this reconstruction.

The ordinary continuation compares the signed counter divided by two with
`metadataDistance + 32`. Native admission bounds the initial counter to 0..32,
iterations to 0..8, and the distance allowance to zero. Each step increases
the counter by at most four, so the continuation remains within the threshold.
The complete original routine returns 3 when the requested iterations finish.
Zero iterations leave record fields, trajectory, counter and previous units
untouched, including pending recount metadata.

`EffectProjectionState` owns trajectory/counter/previous-unit storage separately
from the existing owned placement record. `projectEffectSameCell` validates
dimensions, active/initialized state, selected type/creator/kind/count and
consistent initial fine/cell positions. It advances candidate copies and commits
both only after every iteration succeeds. Height exits, cell changes, excessive
horizontal excursions and unsupported input are atomic native refusals. The
original's partial mutations and failure returns are not being emulated.
The caller must supply the stated empty occupancy/no-creature context; this
helper does not inspect or certify an arbitrary live world.

After a nonzero successful update, the existing `lightingSource` adapter can
consume the now-produced recount coordinates. The native integration test
creates an owned record, verifies its pending-source refusal, advances motion,
passes the source to `TerrainLightingCycle`, and checks the resulting light
field. This is headless composition, not original whole-lighting equivalence or
an installed scene effect producer. No widget, Wine hook or live game is used.

The [comparison record](effect-same-cell-projection.json) pins the executable,
all linked source dependencies, helper hashes and output stream. The oracle
executes the whole unmodified `004883f0`, including real `004df500` and
`004e10e0`, with private owned records, empty terrain bits and empty creature
heads. There are no patches, stubs or child redirections. It compares every
byte of the complete record (including guard bytes), cell allocations and empty
terrain allocation after each call, as well as native trajectory/counter state.
Native 32-bit code runs inside the reference helper; separate 64-bit and
ASan/UBSan streams match. Unit tests cover lighting composition, zero iterations,
one-period wraps, height/type/count/kind/counter/consistency refusals and rollback
when a later iteration crosses a cell.

```sh
python3 tools/test-effect-projection.py
```

The recorded run matches 4,096 fixtures, 6,391 complete-parent calls and
20,842 trajectory steps, including 5,221 wrapped steps and 63,703,760 checked
cell-allocation bytes. Normal/sanitized unit tests and standalone CMake/CTest
pass; all 2,927 immutable originals verify before and after.

Confidence: high within the stated complete-parent path and empty collision
context. Remaining work includes cell changes, occupied terrain and creature
collisions, other metadata/types/termination rules, original trajectory setup,
type-21 placement, slot recycling/removal and live movement/lighting scheduling.
