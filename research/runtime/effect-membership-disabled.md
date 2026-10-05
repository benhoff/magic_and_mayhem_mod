# Effect movement without membership updates

Build: No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence: high within the selected offline contract; no live replacement.

The complete `004883f0` thiscall parent takes one unsigned stack argument.
At `00488686`, a zero argument skips old-cell unlink `00488330`. At `00488708`,
it skips destination insertion and flag `80` clearing. Thus membership-disabled
movement never invokes ordinal-zero cleanup `00534520`: effect heads, every
record's next WORD, cell flags and cleanup-column data remain unchanged.

Other selected movement work continues: real trajectory/counter advancement,
previous fine XYZ, horizontal wrapping, published fine/current-cell/recount
coordinates, previous-cell coordinates, current cell/terrain pointers and wrapped
movement-cache updates. Unblocked paths sample real empty terrain occupancy and
refresh subcell caches, then return `3`. A changed blocked cell still produces
parameter 7 `ffffffff` and return `1`, preserving old subcell caches; the moving
record's next link survives, and the record remains active in its original chain.
Earlier iterations commit before the early return. A bounded subsequent disabled
call can move out of that blocked cell and return `3`; parameter 7 stays `-1`.
Original caller handling of the result is outside this comparison.

A current cell pointer and chain location can now diverge. The native pool's
record `cell` remains the current cell reference; it is not changed to disguise
that divergence. `transitionEffectEmptyWorld(..., columns, false)` admits the
selected moving slot anywhere in a valid owned chain even when its current cell
differs, while retaining bounds, active-node, cycle/duplicate, and other-record
cell-coherence checks. The slot must remain present in a chain. Updates enabled
still require every chain node to match its current cell, so re-enabling updates
before reconciliation is refused atomically. This is intentional owned admission,
not recovery of original behavior on malformed or already-detached inputs.

Cleanup columns are ignored when updates are disabled; native code does not
validate or access them. Original fixtures deliberately point the cleanup receiver
at address `1` in selected calls to demonstrate that it is never dereferenced.
Only globals in the privately mapped image are changed; no PE instructions are
patched or stubbed. Both the executable hash and entry bytes are checked.

`effect-membership-disabled.json` records a separate whole-parent disabled stream
and fresh ordinary, blocked, height and callback/unlink reruns. Each compares
32-bit native inside the original helper, standalone native64, and ASan/UBSan.
Full guarded effect/cell/column/catalog arrays are checked; disabled calls additionally
require the entire cell allocation to equal its pre-call image. Return codes and
owned state are serialized into matching streams. Fixtures include first/later
blocked entries, enabled-placement source head/middle/tail links, bounded repeated
disabled calls, initial blocked same-cell/zero-count cases, wrapping, cached-cell
advances, zero/nonzero terrain and null/allocated/inaccessible cleanup receivers.
Selected emitting types remain 3/13/22/24/36, kind 0/68, creator `-1`, authored
counts 0..8 and the existing counter/dimension/wrapping bounds. Height behavior
is rerun separately; the new disabled fixtures stay within valid height.

Strict normal/sanitized units check unchanged links/flags, ignored malformed
unused column input, blocked-to-unblocked continuation, coherent-update refusal,
existing height/lighting/rollback behavior and CMake/CTest integration. Immutable
originals verify before/after. Historical TL08..TL11 evidence fingerprints remain
unchanged and their affected shared-source results become stale; TL12 records
current execution. Occupied terrain/creature collisions, other metadata/types,
special termination, original trajectory setup, XY-only stepping, column construction,
caller removal/recycling and live scheduling remain outside this contract.
