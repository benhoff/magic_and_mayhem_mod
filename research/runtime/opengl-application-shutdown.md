# Terminal application shutdown ownership

2026-10-06. Confirmed within synthetic PE32/Qt schedules; intentional native
policy. No original-game, hardware-driver, crash-handler or replacement claim.

`RenderShutdown(milliseconds)` remains recoverable session retirement. The new
`RenderStop(milliseconds)` permanently latches the producer launch closed to new
startup, resource publication and recovery requests. Existing admitted original
callbacks can finish their bookkeeping. A successful outstanding Lock/GetDC must
be returned by its application owner; shutdown never fabricates Unlock/ReleaseDC.
An uncertain/abandoned borrow or an in-flight callback refuses cleanup and retains
its storage. Returning a refused borrow permits an explicit retry.

Public stop joins the administrative worker before acquiring the shared lifecycle
lease, callback gate or tracker: the worker may need those same owners to finish
or refuse an active recovery. The control STOP operation lets the worker perform
terminal publication cleanup without joining itself. The main-image ExitProcess
wrapper invokes public stop, which joins that worker and unmaps its control view.
Qt window close requests semantic stop and continues bounded consumer polling;
the application's existing original-menu exit policy remains authoritative for
ending the game process. Input remains suspended while the host waits for stop.

Once callback admission is quiescent, shutdown emits resource deletions and END
for a valid session, closes new observer admission, drains the bounded publication
queue and waits for owned-copy acknowledgment through END. The drain budget is
clamped to five seconds; the administrative join uses at most one second of the
requested budget, callback quiescence retains its existing 250ms bound and the
publication join uses one second. These are separate phase bounds, not one shared
absolute deadline. A zero drain budget still allows bounded publication joining.
Cancellation, invalid identity/ACK, a missing reader beyond capacity or timeout
refuses clean completion. An already published END can remain visible after an
ACK timeout; END alone is not proof of successful terminal completion.

Only verified worker joins permit releasing their mappings and queues. CPU pixels,
lock reservations, surface/palette/alias storage and optional archive/history
storage are retired after all callback/borrow owners are quiet. A failed join
retains worker-visible storage and can be retried. Final completion is cached:
repeated successful stops succeed, a safely retired failed stream keeps returning
refusal, and neither can restart. Late hooks forward each original operation once
without accessing retired rendering storage. Frame/input/media mappings and
bounded diagnostic storage remain process-owned because other installed hooks
can still use them.

The host adds STOP=3 to the existing rendering control v1 operation enum without
changing wire size, field offsets or status values. STOP uses the next request
sequence, zero target session/path length and an entirely zero path payload. It
can follow all three allowed recovery requests as sequence four. READY means
terminal publication cleanup succeeded; REFUSED means clean completion was not
established. ONLINE_STOPPED means the administrative worker has returned, not
that its handle/view has been joined/released. Public stop owns that final join.
A matching reply and drained renderer END may arrive in either order; Qt waits for
both. Refusal/cancellation permanently disables recovery for that launch. Older
control v1 producers reject the unknown STOP operation; the host takes its usual
explicit fallback. No implicit successful downgrade is assumed.

Hook installation pins the DLL with GetModuleHandleExA(PIN|FROM_ADDRESS) before
installing callbacks. FreeLibrary cannot unload code that installed vtable/IAT
callbacks still reference. Dynamic unloading is unsupported. DllMain process
detach only signals worker stop/refusal; it does not join, pump, call originals,
free worker storage or finalize archives under loader lock. Normal ExitProcess
performs terminal ownership first; refused borrowed/join cleanup is reported and
the original process exit proceeds with retained storage owned by OS teardown.
Forced termination relies on OS reclamation and offers no END guarantee.

Validation uses real synthetic original Lock/DC methods, independent full-frame
pixels, the actual administrative/publication workers and native Qt/OpenGL
consumer. SELFTEST-only worker exit barriers make failed joins reproducible and
are absent from the production DLL. The complete record and regression records
are linked from the coverage register. Arbitrary callback destruction, opaque
borrow capacity/alias uncertainty, malicious peers changing control bytes at
arbitrary instruction boundaries and original application/driver shutdown remain
pending; these do not receive original comparison or live replacement status.

Fresh records: [checkpoint](opengl-application-shutdown-checkpoint-20261006.json), [drawing](opengl-application-shutdown-drawing-20261006.json), [host](opengl-application-shutdown-host-20261006.json), [native](opengl-application-shutdown-native-20261006.json), [orchestration](opengl-application-shutdown-orchestration-20261006.json), [recovery](opengl-application-shutdown-recovery-20261006.json), [transitions](opengl-application-shutdown-transitions-20261006.json). The seven matrices pass81 cases/655 complete-frame comparisons, with16 literal control cases in native and Address/UndefinedBehaviorSanitizer execution. Production/SELFTEST DLL builds, generated protocol checks and full Qt shell build pass. Production exports contain RenderStop and exclude SELFTEST controls. The committed856f6ef..ef5b5a4 range was reviewed separately with exact parent/current hashes and receipts and no unresolved history gaps. Historical comparisons remain unchanged and may be stale after shared source edits.
