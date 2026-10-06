# Startup and shutdown transition ownership

RenderStartup and RenderShutdown now own the existing lifecycle serialization
word across their entire state transition. Recovery already owns that same word.
A contending startup/shutdown returns false without reading readiness, finishing
resources, disabling capture or draining worker storage. Previously startup
checked readiness before acquiring scheduler serialization, and shutdown finished
capture before acquiring it: a losing shutdown could disable a newer session or
prevent a concurrent transition from proceeding safely.

Startup keeps lifecycle ownership while checking tracker-protected readiness,
starting the retry worker and starting configured administrative control. Shutdown
keeps it while closing capture under the tracker, draining and joining, and
releasing worker-visible storage. Scheduler helpers now expect lifecycle ownership;
SELFTEST-only scheduler startup retains a scoped acquiring wrapper. Every early
return releases lifecycle ownership. Original HRESULT/LastError forwarding,
worker retention after a failed join and existing recovery checks are unchanged.
This is intentional native lifecycle policy, not recovered application ordering.

With continuous opt-in ordered drawing, shutdown also acquires a separate bounded
callback quiescence lease before touching tracked state. A short active callback
can finish its original and commit before capture closes. Acquisition uses the
existing50ms admission budget; timeout returns false without invalidating pixels,
metadata or the stream. Before completion, same-thread reentry returns false immediately, since the
calling original has not completed and cannot be waited on safely. A later call
outside the callback can retry. Once the worker has joined, the cached completion
result returns before callback quiescence, preserving idempotence during later
original-only drawing callbacks. This differs from a drawing admission timeout,
which forwards an uncertain original write and must invalidate state.

Shutdown releases drawing ownership after capture admission closes and before
publication draining/joining. It keeps lifecycle ownership throughout. No tracker
is held across callback waits, originals, draining or joins. The50ms admission is
additional to the existing capped drain interval and1000ms join; it is not a new
end-to-end wall-clock guarantee. Default-off drawing retains its prior callback
policy, while full lifecycle serialization applies independently of that opt-in.
Borrowed CPU/DC intervals remain governed by existing ownership/refusal checks.

## Validation

The independent PE32 factory fixture establishes a complete original-native
primary through actual installed DirectDraw1 hooks. Six new schedules exercise:

- Worker startup followed by shutdown while primary Unlock is admitted. The
  worker must not complete shutdown before the original returns. Both native
  frames precede final retirement and END; subsequent startup refuses.
- An original Unlock waits for the worker's shutdown timeout. The refused call
  must leave startup ready and the original's pending update can still commit.
  Later shutdown emits END and retires storage normally.
- An original Unlock calls shutdown on its own thread. Immediate refusal keeps
  its pending update and full native pixels valid; ordinary later shutdown passes.
- A controlled SELFTEST-only lifecycle guard blocks worker startup/shutdown, then
  releases. The next native update and clean retirement prove capture was not
  disabled by the losing transition. This runs with ordered drawing on and off.

Fixtures verify exact original counts, LastError, lifecycle results, complete
independent RGBA hashes, admission diagnostics and final zero consumer storage,
readbacks/uploads and unique CREATE/DELETE/END identity. The guard export and
import are absent from production builds and supply no pixel/checkpoint state.
A sixth schedule completes shutdown, then performs another original Unlock that
reenters shutdown and must receive the cached successful result. Its original
pixels update while the ended stream remains unchanged.
Existing lifecycle, factory startup and late checkpoint regressions are recorded
under fresh evidence IDs; historical source hashes are unchanged.

External recovery collisions, simultaneous shutdown callers during long joins,
tracker contention during closure, abandoned/terminated callback owners,
administrative worker shutdown ownership, dynamic unloading and original
application/driver/full-frame equivalence remain pending. No default rendering
replacement or gameplay change is claimed.

Exact committed history `f86d3fd..e7c56c2` was reviewed against parent/current
file and behavior receipts with no unresolved entries, preserving prior journal
order. This review asserts accounting only, not past gate passes or validation.

Fresh [race matrix](opengl-lifecycle-races-native-20261006.json) passes6 cases/12
complete frames. [Lifecycle regression](opengl-lifecycle-races-lifecycle-20261006.json)
passes11 cases/8 frames; [factory regression](opengl-lifecycle-races-startup-20261006.json)
passes7 cases/13 frames; [late checkpoint regression](opengl-lifecycle-races-checkpoint-20261006.json)
passes6 cases/186 frames. Production and SELFTEST DLLs build. Interim pre-cache
results remain under working/tests/lifecycle-races-interim-* without current-code
validation claims; final records retain their exact execution fingerprints.
