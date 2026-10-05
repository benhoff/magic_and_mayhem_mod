# Effect height exits

Build: No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence: high within the selected offline contract; no live replacement.

The whole `004883f0` movement parent saves previous fine XYZ from record
`+14/+18/+1c` into `+1ea/+1ee/+1f2`, then invokes real trajectory helper
`004df500` at `00488587`. The helper writes parameter XYZ `+2c/+30/+34`,
trajectory words at `+128`, and the change counter at `+1ce` directly.
At `0048858e` the parent rejects signed-negative Z; at `0048859e` it rejects
unsigned Z greater than or equal to layer count `006c549c * 16`.
Both branches reach `00488bb5`, which returns `2` without further mutation.

Consequently a height exit commits the advanced raw parameter XYZ, trajectory,
change counter and saved previous fine XYZ. Horizontal wrapping has not run:
raw X/Y can be negative or exceed the horizontal world period even though the
published fine coordinates remain valid. Current fine XYZ, cell XYZ, current
cell and terrain pointers, membership chains, subcell/recount caches, previous
cell coordinates and wrapped movement cache retain their last valid values.
Parameter 7 is unchanged; this path does not use the blocked-destination `-1`
marker. The effect remains active and linked. If earlier iterations completed
in this same call, their published changes and cleanup remain committed.
Remaining authored iterations do not run. A zero-iteration call never checks
an authored trajectory step; it completes with `3` from a valid initial record.

`transitionEffectEmptyWorld` now commits this supported partial state and returns
`2`, in addition to ordinary `3` and blocked `1`. It copies the raw parameters
on exit while retaining the last valid fine/cell state. Input bounds and corrupt
or unsupported states still throw atomically. Reusing a height-exit record is
refused by the existing native parameter/fine-coordinate coherence admission;
caller handling, removal/recycling and original resumption are outside this
comparison. Atomic exceptions and that admission are intentional native policies.

`effect-height-exits.json` records a separate whole-parent height comparison,
plus fresh ordinary movement, blocked destination and cleanup/unlink reruns.
Each has matching original32/native64/ASan-UBSan streams. The original helper
contains a 32-bit native comparison as well as the complete unmodified parent.
Full guarded effect records, cell arrays, column arrays and terrain catalog bytes
are checked after original calls; return codes are serialized. The owned one-step
trace used only for step/transition counts starts its counter at zero to stay
within per-call admission. The compared whole-parent call uses the actual initial
counter 0..32; selected trajectories stay within the original distance threshold. No original
instructions are patched or stubbed. Input hashes and original media manifests
are verified before and after execution.

Height fixtures cover lower/upper boundaries, first and later authored steps,
zero-count controls, signed Z deltas including `80000000`, horizontal raw values
that would exceed native wrapping admission if the height check did not happen
first, initial change counters 0..32, five emitting types, kind 0/68, head/middle/
tail links, terrain ordinals 0..3, null/allocated cleanup columns and last-valid
movement/cache/reference/recount state. Strict normal/sanitized unit tests check
first/later exits, preserved membership and caches, unwrapped parameters,
unchanged parameter 7, retained active state and refusal on resumption. Existing
lighting, rollback and blocked-state units also pass through CMake/CTest.

TL08/TL09/TL10 fingerprints remain historical. Their affected transition/test/
runner results become stale; TL11 records the current expanded proof without
rewriting old evidence hashes or promoting their statuses. Disabled membership,
occupied terrain/creature collisions, special metadata/types and termination,
original trajectory setup, XY-only stepping, column construction, caller lifecycle
and live scheduling remain outside the implemented/validated contract.
