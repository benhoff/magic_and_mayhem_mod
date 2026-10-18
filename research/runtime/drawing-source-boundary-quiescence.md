# Observed primitive callback quiescence

Whole-update callbacks drain observed primitive frames and record their source
prefix under the same recorder lock. Waits release that lock between attempts so
other threads can retire. Same-thread pending work refuses immediately; an
other-thread stall refuses after5000ms. Normal callbacks and terminal closure
use the same rule. A stop during an unlocked wait publishes no further boundary.

The accepted cadence timestamp must also lie inside that drained critical
section. SceneDrawingUpdatePrepareV1(record, accepted, counter, mode) returns a
nonzero generation token while retaining the recorder lock. Cadence samples QPC
then calls SceneDrawingUpdateCommitV1(token, record, accepted, counter, mode).
Commit requires the exact owner, generation and four input words, invokes the
source callback and releases the lease. SceneDrawingUpdateCancelV1(token)
releases without a boundary after a failed QPC sample. Invalid owner, token,
reentry or tuple refuses; an unsuccessful commit leaves its valid lease for
explicit cancellation. Generation exhaustion refuses. No host pointers cross
this versioned ABI. Mixed old Scene/new cadence binaries refuse missing exports.

The older SceneDrawingUpdateBoundaryV1 remains a direct, draining callback for
controlled callers. It alone cannot establish ordering before a cadence QPC
sample. The existing wire journals and all interval admission gates are unchanged.

These waits intentionally alter diagnostic scheduling. They do not protect
physical backing aliases, unhooked writers, every primary output route or all GPU
contexts. See source-boundary-provenance-20261017.json for retained execution,
negative controls and the remaining unrelaxed admission failures. No original
comparison or native replacement milestone follows from observer quiescence.
