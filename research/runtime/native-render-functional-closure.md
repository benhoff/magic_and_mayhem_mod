# Native renderer functional closure

The 2026-10-18 increment addresses four implementation limits independently of
the historical renderer acceptance milestones.

Canonical owned CPU copies now convert among indexed8, RGB555, RGB565, BGR24
and BGR32, with nearest-neighbour scaling and raw source-key comparison before
conversion. Indexed conversion requires a bound palette; nearest colour ties
select the lowest index. These are explicit native policies, not newly recovered
Wine/Windows colour or stretch semantics. Existing same-format bytes are retained.
Bridge-ineligible copies publish their complete current owned CPU result through
CREATE/UPDATE, including valid clipped prefixes before an application rectangle
error. Those producer bytes are separate from CHECK/diagnostic output.

Mode3 retains the private-view pure-store fast path. Other owner-Lock reads and
writes redirect to a continuation which saves GPRs, flags and FX state, restores
current history and pending bytes outside exception dispatch, then retries the
original instruction. REP, SIMD and RMW execute on the CPU after restoration;
the handler does not decode/emulate them or wait for the renderer. Foreign or
expired leases and the mandatory World interval remain protected. No general
application data-race semantics are promised.

The initial checked No-CD image row body at533aec..533aff is replaced with native forward
DWORD/residual-byte copying. Its DWORD-granularity overlap, defined final AND
flags and caller-visible registers are preserved. The jump skips all twenty
original row-body bytes. That increment retained original outer clipping/preparation;
the subsequent [implementation follow-up](native-render-implementation-closure.md)
replaces the whole533a00 entry. Both owned operands require current CPU history
before native copying.

Native-only Hard restart now retires one generation and starts a fresh admitted
generation. Atomic retirement records preserve previous errors and explicitly
withhold normal-completion credit. The owner verifies the old runner PID/start
ticks is no longer alive and requires one final normally completed generation.
Stopping the process and releasing renderer/input/media owners precedes relaunch.
Closing a failed session remains nonzero; no in-place recovery or progress
preservation is introduced.

The four policies are registered before acquisition as
`NP.native-copy-transforms`, `NP.native-cpu-instruction-resume`,
`NP.native-image-row-copy` and `NP.native-only-generation-restart`.
Execution results and remaining implementation defects will be recorded here
without renewing historical report hashes or claiming whole-game replacement.

## Focused execution

[Initial controls](native-functional-closure-controls-20261018.json) pass150
copy cases/17,550 independently expected pixels across25 format pairs, reduction,
enlargement and source keys. The actual PE32 protected backing executes raw loads,
RMW, REP, SIMD and locked RMW; the old original row body is poisoned with traps.
Actual Qt generation-state and process-stop controls include an already absent
Wine server, diagnosed kill errors and a failed wait barrier. Sources are stable.

[Initial bridge/FIFO controls](native-functional-closure-readers-20261018.json)
pass19 ordered restores and38 exact WORD updates, including pending-byte merges,
partial Locks and foreign-reader refusal. A post-restoration Unlock followed by
self-BltFast must succeed and publish a complete current CPU checkpoint; every
checkpoint WORD is checked against the independent FIFO image model.

The first live stress identifies the formerly unexplained80004001 precisely:
`NativeDrawGpuCopyPlanV1` line39 rejects a source whose physical CPU backing was
already restored, while `game_unlock_after` had incorrectly reasserted
`gpu_history` from its resident origin. Successful Unlock now queries the exact
unborrowed physical authority before assigning that bridge flag. This preserves
the existing strict GPU-only admission rather than admitting stale metadata.

The first actual native-only restart identifies a separate lifecycle mismatch:
Wine `-k` returns1 with no diagnostics when its private server is already absent.
Process stop now follows that case with the mandatory successful `-w` barrier;
diagnosed errors, other statuses and a failed/timed-out barrier still refuse.
Historical failed attempts remain retained in the acquisition directory.

## Retained execution for that increment

[Final controls](native-functional-closure-final-controls-20261018.json) and
[final bridge/FIFO controls](native-functional-closure-final-readers-20261018.json)
pass with stable declared sources. They include resident admission at80ms,
refusal beyond250ms, the finite reader extension and spurious-wake schedule.
Nonresident diagnostic copies retain their50ms policy. General CPU instruction
classes have headless integration; no positive live execution of every class is
inferred from installing their continuation.

[The native-only input stress](native-functional-closure-stress-20261018.json)
passes20-second movement/camera/HUD input phases, Mini Cancel/resume, Quit No/Yes
and normal original exit0:217Worlds complete/import/return,4,828producer calls
execute natively, forwarding is zero. Four533aec restoration tags are observed
in its retained lifecycle log. This exercises the repaired copy/history route;
creature-movement semantics and original pixels are not asserted.

[The final native-only restart](native-functional-closure-restart-20261018.json)
injects a live producer failure, observes500ms of terminal cleanup, invokes public
Hard restart, verifies a new launcher/channel and completes a fresh3-World game
and normal Quit with673native calls/zero forwarding. The old cancelled generation
is recorded as restarted without rendering acceptance. A subsequent staging
failure cannot overwrite that retirement, and can itself be restarted.

All2,927original files verify before/after both acquisitions. The complete
[acquisition catalog](native-functional-closure-acquisition-20261018.json)
preserves setup failures, runtime defects and superseded controls. Neither these
functional repairs nor the bounded journeys certify complete replacement,
original-driver/pixel equivalence or sustained hardware performance.

## Subsequent implementation follow-up

[The implementation follow-up](native-render-implementation-closure.md) adds the
whole clipped-image routine, extended2D effects/palettes/display depths and opt-in
native RAM graphics checkpoints. Current isolated PE32 and OpenGL controls pass,
and the complete Qt host builds. Quiescent context rebuild preserves frame and
surface identities. Recovery from in-flight draw/producer faults, hung drivers
or lost Qt platform surfaces still requires Hard restart; universal unsaved-progress
preservation remains unimplemented. Older reports retain their original sources
and are historical when their fingerprints differ from the current implementation.
