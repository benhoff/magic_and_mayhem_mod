# Qt continuous command recovery

2026-10-06. Intentional native policy `NR.command-host-recovery`; original
addresses are deliberately absent. This adds automatic host orchestration to
[explicit producer recovery](opengl-command-recovery.md), preserving original
DirectDraw calls and the original game window. It does not establish original
recovery semantics, bypass original rendering or claim driver equivalence.

## Ownership and admission

`LiveCommandSession` owns one consumer and a `RenderControl` client. Continuous
v2 launches create a separate fresh [control channel](../formats/render-control-v1.md).
Launch staging validates immutable identity, exact size, zero initial fields,
matching initial session IDs and paths under `working/`, before reading/staging
game artifacts. v1 remains the bounded, non-recoverable preview.

A failed native poll cancels and destroys the old consumer, drops its queued
records and clears the viewport GPU frame. The host creates a fresh command file
with a higher session ID and sends its ANSI Wine path plus expected ID. It does
not poll candidate command bytes until a matching READY response. Fresh consumer
storage and producer CPU checkpoints cannot retain old texture/resource IDs.
The existing producer additionally rejects reused physical files, non-increasing
session IDs and non-fresh/malformed candidates, retaining its sixteen-file budget.

Three recoveries are allowed per launch. Missing/invalid/refused responses or a
6000ms reply deadline permanently cancel the launch control channel. Initial
presentation and accepted recovered streams require a complete PRESENT within
10000ms. A failed or silent recovered stream before its first complete frame
falls back immediately or at that deadline, without an automatic retry loop.
After a successful fresh frame, a later failure can use the remaining budget.
The UI suspends forwarded input and clears/hides the native presentation while
recovering; input resumes after a complete new frame, subject to movie playback.
Fallback keeps original-window interaction available. Neither failure nor cancel
terminates the game. Producer exit cancels pending negotiation and drains only
an already active consumer; it cannot activate a late candidate.

## Native quiescence

The optional persistent control worker is started by normal `RenderStartup`,
including when primary native readiness has failed, never by DllMain. It survives
retirement of individual queue retry workers. Loader detach sets a stop flag;
OS process cleanup owns the worker handle/mapping. Dynamic unloading remains
unsupported while installed callbacks can run.

Every installed render/identity/palette observer callback and DirectDrawCreate
holds a counted admission lease across the original call and its observer work.
Surface clipper observation is covered by its shared property helper. Input
cooperative level and enumeration/procedure-resolution hooks do not mutate
render checkpoints. The administrative gate waits up to 250ms for active
callbacks AND observed pixel/DC ownership to drain. If a Lock has returned while
the game writes borrowed pixels, the gate reopens to let Unlock complete, rather
than holding admission closed across that lease. It then checks again.

Once exclusive, the worker calls RenderShutdown(0) to close producer admission,
finish or refuse the old stream, and join/retire retry-worker storage. Its result
can be false because the host deliberately cancelled the old ring. Private
recovery requires the request's expected session ID, the existing lifecycle,
tracker/drain serialization and a fresh wire resource namespace. Administrative
RECOVER prefers complete owned state, chosen behind the tracker and exclusive
callback gate after old-worker join. It uses the same strict checkpoint predicate
as CHECKPOINT, queues resource creation and initial PRESENT before READY, and
retains initialized source pixels. When completeness is unavailable before claim,
it uses fresh observations with current verified lifetime metadata. A failed
claimed checkpoint refuses rather than resetting/retrying that candidate. The
local RenderRecover export retains its fresh-pixel policy. Historical reports
below remain pinned to their original implementation; see the
[automatic checkpoint validation](opengl-automatic-checkpoint-recovery.md). Unknown
operations or changed request identity/path/session/sequence refuse. The worker
checks cancellation before and after admission; candidate bytes stay invisible
to the Qt consumer until READY.

New callbacks wait at most 50ms on the administrative gate, never on peer ACK.
On expiry they hold a counted bypass lease, record collision, and forward the
original method without native observation. HRESULT/LastError pass through.
A collision, including one finishing between the earlier check and gate-release
CAS, retires any candidate and replies REFUSED. Gate release preserves active
bypass counts, preventing new exclusive admission from ignoring them. This is
an intentional finite administrative delay, not a real-time OS scheduling
promise. CPU snapshot ownership checks apply only to observed hooked interfaces.

## Validation and remaining boundaries

The reproducible synthetic harness is `tools/test-render-host-recovery.py` with
`working/build/render-ring`. It drives real asynchronous PE32 hooks/worker and a
single Qt viewport/context via `tests/live-command-recovery-test.cpp`. Fake
original CPU storage is independent, padded borrowed rows are poisoned on
Unlock, and every displayed pixel is compared with the fixture's mathematical
color sequence. Tests check monotonically newer frames after handoff, immutable
old terminal rings, cleared presentation, original Lock/Unlock HRESULT/error
and equal call counts, zero ordinary consumer readback/upload, and zero consumer
surfaces at terminal state. No CHECK command supplies expected pixels.

Scenarios cover healthy and repeated handoffs, three-attempt exhaustion, held
leases, a 500ms original callback, forced 200ms administrative delay/bypass
collision, malformed reply, absent control worker/reply timeout, process exit,
cancellation during negotiation, mutated operation/request, no fresh frame,
and a stream failing before its first presentation. Delays are selftest-only.
Native and ASan/UBSan/leak-checked control tests separately cover publication,
matching ACK, malformed identity/reserved/status/online/sequence, cancellation,
stopped/refused peers, finite requests and untouched existing files.

Original game recovery, movie/recovery interaction during actual execution,
real drivers, unobserved lock/DC leases, abrupt process death during original
callbacks, allocation/thread/map failure injection and adversarial file
truncation/replacement remain pending. Existing historical evidence and source
hashes remain immutable; shared-source changes do not renew old original
comparison or live observation claims. The exact executed matrix is pinned in
`opengl-command-host-recovery-pe32.json`; final queue/control regressions have
separate records. The accounting history report reviews 27 committed files and
17 changed behavior contracts in `4eb3977..529019f`, separately from this work.

Final-source evidence: [13 host scenarios /143 complete frames](opengl-command-host-recovery-pe32.json), [nine invalid launch configurations](opengl-command-host-recovery-launcher.json), [native/sanitized queue regressions](opengl-command-host-recovery-queue.json), and [nine explicit API scenarios /33 sessions /66 frames](opengl-command-host-recovery-explicit.json). Ten native and ten ASan/UBSan control cases with leak checking pass; four Qt/OpenGL CTest regressions, generated protocol checks, production/selftest PE32 builds and the Qt shell build pass. Original artifacts were not consumed.
