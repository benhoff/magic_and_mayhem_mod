# Blocked effect destinations

Build: No-CD executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence: high within the selected offline scope. No live replacement.

The complete `004883f0` movement routine tests destination cell flag `40000000`
at `00488718` (membership insertion guard) and `00488839` (early return guard).
For the selected types 3/13/22/24/36, no creator, kind 0/68 and empty collision
context, entry into a different blocked cell:

1. Advances the real trajectory and change counter; saves previous fine units.
2. Wraps X/Y, publishes fine units and current-cell recount coordinates.
3. Unlinks from the old cell using real `00488330`, including ordinal-zero
   `00534520` column cleanup when applicable.
4. Saves previous cell coordinates, changes current cell and cell pointer.
5. Skips destination membership insertion and flag `80` clearing.
6. Updates the terrain catalog pointer and wrapped-neighbour cache using real
   `004ead30` (a distant transition copies previous cells into the cache).
7. At `00488b69`, writes DWORD `ffffffff` to record `+48` (parameter 7),
   returns `1` and skips all remaining authored iterations.

The early return preserves prior subcell caches `+1aa/+1ae/+1b2`; it occurs before
terrain-bit sampling and creature-neighbour rebuilding. Creature pointer `+198`
is unchanged (null in this comparison). Destination heads, flags and existing
chain nodes remain unchanged. The effect stays active with next link `ffff` and
its current cell now pointing at a cell whose chain does not contain it. Return
`1` is the recovered movement outcome; caller handling of that return and
destruction/recycling are outside this comparison. Calling this operation again on that detached record is an explicit
native refusal, not a comparison of original lifecycle scheduling.

The flag is checked only after a cell change in this path. An initially blocked
cell with an existing chain is admitted: zero-iteration and same-cell calls
complete with `3`, while a subsequent transition observes the new destination.
The reconstruction does not insert effects into blocked cells during placement.
Fixtures set this flag after preparing an already-valid placement to test this
bounded original input; no live-world interpretation is inferred.

`transitionEffectEmptyWorld` now accepts blocked touched cells with catalog
ordinals 0..3. Return `1` commits the supported original partial state; return
`3` commits ordinary completion. Unsupported bounds, terrain, trajectory,
chain or later failures still throw without committing the candidate. This
atomic exception policy is separate from original malformed-input behavior.
The owned cleanup-column model and selected source types remain unchanged.

`effect-blocked-destinations.json` records a separate whole-parent blocked stream
and reruns the ordinary zero/nonzero movement and callback/unlink comparisons.
Each compares 32-bit native inside the hash-checked original helper, standalone
64-bit native and ASan/UBSan. Full guarded effect records, cell arrays, column
arrays and terrain catalog bytes are checked; the return code is serialized.
Fixtures vary first/later blocked entries, old-chain head/middle/tail, existing
or empty destination heads, null/allocated cleanup columns, wrapping, cache
advances and initial blocked same-cell/zero-iteration inputs. Strict units verify
active-but-detached state, untouched destination chains/flags/subcell caches,
recount updates, stopping before remaining steps, refusal on resumption, moving
lighting and existing atomic failure behavior. CMake/CTest also runs those units.

TL08 and TL09 fingerprints remain historical. Edited transition/test/runner
files make their corresponding recorded results stale; TL10 records current
execution for the expanded contract. Neither historical hashes nor their
original statuses are rewritten. Remaining gaps include height exits, disabled
membership, occupied terrain/creature collisions, special metadata/types and
termination, original trajectory setup, column construction, removal/recycling
and live scheduling.
