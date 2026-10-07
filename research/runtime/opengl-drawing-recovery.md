# Drawing during direct session recovery

Direct `RenderRecover` now obtains exclusive callback admission before opening a
candidate or resetting producer state. Continuous production counts callbacks
whether or not administrative control and ordered drawing are configured.
Administrative recovery passes its already-owned gate explicitly. This is native
ownership policy, not reconstructed DirectDraw or application scheduling.

The existing 250 ms gate admission and 50 ms callback wait remain finite. A short
callback can finish and permit recovery. A prolonged callback, same-thread
reentry, admitted Lock/DC, pending captured pixels or successful bypassed borrow
refuses admission before candidate claim. Refusal preserves the retired channel
and unclaimed candidate, so release and retry can establish a fresh session.
No tracker is held while waiting for callback admission or calling an original.

A callback that times out forwards the original exactly once, preserving its
result and LastError. It invalidates pixel, metadata and alias provenance before
and after that original. Exclusive-gate collision evidence survives contending
close attempts. Direct recovery also guards its final gate release/collision
check against another closer clearing that evidence. It cannot return success
if an original crossed the reset gate. Before claim, the candidate stays unused;
after claim, its stream is explicitly failed and the file/session is spent.
Worker-visible storage remains owned until normal shutdown joins the worker.
Fresh retry uses a different file and higher session ID after a claimed failure.

## Borrowed ownership independent of pixel observation

Successful Lock/GetDC calls bypassed by gate timeout or ordered drawing timeout
retain an opaque receiver/kind/handle entry in a fixed 64-slot table. These entries
are independent of derived pixels, alias epochs, tracker generations and consumer
resources. Completing the callback does not end a borrowed interval. Only a
successful original Unlock or ReleaseDC on the exact receiver (and DC handle)
removes its matching entry, including a release which itself bypasses observation.
Failed release keeps it pinned. Reads/publication of entries use atomics.

The table never dereferences a receiver or a DC handle and does not infer COM
ownership from a pointer's continued existence. Capacity exhaustion or successful
GetDC with an unreadable/null output marks ownership uncertain permanently;
recovery then refuses until process restart. Unobserved cross-interface release,
final Release of a bypassed borrow, owner termination do not resolve its ownership. Pointer reuse beyond observed
lifetime boundaries is unvalidated. An unmatched entry conservatively prevents
recovery. Normal admitted alias/Lock/DC tracking remains separate. These are
explicit failure boundaries, not claims of original driver equivalence.

## Validation

Synthetic PE32 originals own separate native pixels and poison borrowed rows at
Unlock. Native Qt/OpenGL consumers receive only producer commands; expected frame
hashes come from independently generated fixture pixels. Schedules cover a held
original Lock callback with ordered drawing enabled and disabled, a short
callback which permits recovery, same-thread Unlock reentry, admitted Lock/DC,
bypassed Lock with ordered drawing enabled and disabled, bypassed DC with failed
and successful ReleaseDC, and a callback crossing an already claimed reset.

Each case publishes two complete primary frames in the old session and two in a
fresh session. Checks retain LastError/results, exact original counts, complete
frame hashes, fresh session and resource IDs, terminal cleanup, frozen retired
ring/archive bytes and untouched unused candidates. Bypassed Lock/DC remains
unrecoverable after its callback returns; failed DC release does not unblock it.
The claimed collision must fail explicitly before the fixture retires its worker
and retries with a new candidate. SELFTEST-only barriers before candidate opening
and after reset are excluded from production builds.

Original application/driver scheduling, full gameplay frames, abandoned owners,
opaque-table exhaustion, unobserved alias release, repeated contention at every
reset instruction, administrative worker shutdown and dynamic unloading remain
unvalidated. Historical evidence and source hashes are retained. Fresh execution
records and the exact reviewed committed history range are appended separately.

Fresh [drawing evidence](opengl-drawing-recovery-native-20261006.json),
[recovery](opengl-drawing-recovery-recovery-20261006.json),
[transition overlap](opengl-drawing-recovery-transitions-20261006.json),
[lifecycle](opengl-drawing-recovery-lifecycle-20261006.json),
[checkpoint](opengl-drawing-recovery-checkpoint-20261006.json) and
[orchestration](opengl-drawing-recovery-orchestration-20261006.json) records total
60 cases/456 complete frames. Every execution fingerprint matches the updated
staged source tree. The checkpoint run initially fingerprinted a concurrently
edited renderer build file; independent commit856f6ef then committed that exact
file. It does not assert execution of that separately modeled owned backend.
The [isolated prior-base rerun](opengl-drawing-recovery-checkpoint-isolated-20261006.json)
is independently retained:20 cases/312 frames with the earlier build-file hash,
now historical against the updated base and not counted twice.

Production and SELFTEST DLL builds pass; production exports exclude test controls.
Exact committed history `0c80a96..856f6ef` was reviewed separately against
parent/current file/behavior receipts with preserved prior ordering. This review
asserts neither past gate passes nor new historical execution. Unrelated owned
backend changes and accounting remain preserved in the updated commit base.
