# Effect movement and terrain occupancy

Build: No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence: high within the selected offline contract; no live replacement.

Terrain catalog entries are 356 bytes; their first 32 bytes form the 8x8x4
occupancy bitmap consumed by `004e10e0`. Its thiscall receiver is the entry base;
stack arguments are local X/Y/Z and callee pops 12 bytes. The selected parent
supplies `(fineX >> 2) & 7`, `(fineY >> 2) & 7`, `(fineZ >> 2) & 3`.
The helper reads byte `Y + Z*8` and tests MSB-first mask `80 >> X`, returning
one for a set bit and zero otherwise. It does not mutate the bitmap.
This is a recovered runtime catalog layout; installed-file-to-catalog construction
is outside this comparison.

The complete `004883f0` parent compares those coordinates with cached fields
`+1aa/+1ae/+1b2` at `00488946`. If all match, sampling is skipped, even when
current cell and terrain catalog entry changed earlier in the step. If any differ,
it calls real `004e10e0` at `0048897f`. A zero result publishes the local cache
and continues. A nonzero result writes parameter 7 (`+48`) `ffffffff`; for
selected types 3/13/22/24/36 it returns `1` via `00488b70` before publishing the
new subcell cache or entering the creature collision loop. Type 35's exceptional
terrain mutation branch remains excluded and unimplemented.

Trajectory/counter, previous fine XYZ, wrapped parameters, fine/current-cell/
recount, terrain pointers, previous cells and movement-cache work are already
committed. On a cell change with membership updates enabled and an unblocked
destination, old-cell unlink/column cleanup and new-cell insertion/flag clearing
also occurred: the terrain-hit effect remains active and linked in the new cell.
With updates disabled, original chains and flags remain intact. A terrain-hit
return `1` therefore differs from blocked-cell return `1`, which skips insertion
and leaves an enabled effect detached. The caller must handle these outcomes;
removal/recycling and live scheduling are outside the comparison.

`EffectTerrainOccupancy` provides four owned catalog bitmaps, passed by const
reference to the existing transition operation. The default is zero-filled and
preserves prior callers. No bitmap pointers or host layouts are stored in the
recovered record. The operation follows original cache-gated sampling and early
return ordering. Its name remains `transitionEffectEmptyWorld` for compatibility;
the contract now excludes creatures while supporting terrain occupancy.
Unsupported bounds/chains/input still throw atomically; disabled membership's
selected current-cell/chain divergence admission is retained as native policy.

`effect-terrain-occupancy.json` records a separate whole-parent occupied/clear
bitmap stream and fresh ordinary, disabled membership, blocked, height and
cleanup/unlink reruns. Each compares original32 (including native32), native64,
and ASan/UBSan. Original calls use the complete unmodified parent and real helpers,
with hash/entry-byte checks and no code patches or stubs. Full guarded records,
cells, columns and entire terrain catalogs are checked. Disabled calls also
require the entire cell allocation to equal its pre-call image. Source and PE
hashes remain stable during execution; original media manifests verify before/after.

Fixtures use both membership settings, all local X/Y/Z ranges, clear/full/mixed
bitmaps, same-cell fine movement, changed-cell movement, wrapping, zero iterations,
pending/matching initial caches and repeated calls. The bit-hit return code and
full state are serialized, including preserved subcell caches and membership.
Selected types, creator `-1`, kind 0/68, terrain ordinals 0..3 and the prior bounded
counter/count/dimension/trajectory contract remain in force; no creatures are
present. Strict normal/sanitized units check hit ordering and membership enabled/
disabled state, matching-cache bypass, existing height/lighting/rollback behavior,
and CMake/CTest integration. Historical TL08..TL12 fingerprints remain unchanged;
TL13 records current expanded execution separately.

Remaining gaps include creature occupancy/collision, type-35 terrain mutation,
other metadata/types and termination, original trajectory setup, XY-only stepping,
installed catalog production, column construction, caller lifecycle and live
replacement. Disabled-height combinations remain outside the dedicated disabled
fixtures; bitmap-hit interaction with blocked flags/height exits is not newly
compared by the dedicated bitmap fixtures.
