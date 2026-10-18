# Ordinary owner task scheduler (NS83)

The pinned NoCD415c40 driver increments its owner clock even when the brain is
disabled, then (when enabled) plans before executing tasks. In count mode0,
4075b0 gives planning `floor(total/2)` units and execution the remaining units.
Unused planning units are not transferred. Mode1 introduces division by250 and
optional rdtsc accounting; those modes and the World allocation producer are
outside this milestone.

The bounded projection has one ordinary enabled direct-root source, brain0 and
empty specialized source queues. It retains phase4 when the planning call returns
with fewer than one unit, even if the selector completed its sweep. It enters the
pending root before checking its budget, whereas execution checks its independent
budget first. An empty execution queue is copied from the installed task map;
each visited task is removed from that pending queue even when skipped. Execution
may overshoot its budget: the ordinary task6 body debits8 after admission.

415c40 tests task+14 before invoking the body and stores `!continuing` there.
This is distinct from task+15, which423590/4284e0 use to bypass replacement gates.
The historical native `OwnerPreviousTask::completed` field models+15. NS80/NS82
tests explicitly supplied that flag after direct body calls; they did not prove
the caller writes it. NS83 keeps a separate `executionComplete` projection.

428b40 retires an ordinary enabled-source task when its value is below1 and
`pow(progress, age) < expiryThreshold`, or at `clock >= expires`, or when result+14
is set. Task6's expiry virtual432fb0 is a no-op. Cleanup runs at an idle planning
phase and again after a resumed root finishes, before execution. The source's
task pointer and pending execution entry are removed. Feedback/training, object
destruction and insertion into other source queues remain explicit services.

## Comparison boundaries

The fixture invokes actual415c40,423590,428b40 and432900, together with NS82's
real prepare/group/probe/features/inference/selection/save/install chain and
original task-map copying/erasure. Perception4181d0, source maintenance417580,
snapshot428e40/swap428f80 and source queue insertion416fe0 are declared services.
World/model/ordinary actors and stable observations are authored. Clone,
destruction, training registration and scoring retain NS82's supplied boundaries.
Private mapped text is protected after declared redirects and checked unchanged;
the immutable executable is hash-pinned and manifest-bracketed.

Simultaneous displaced tasks, multiple sources, specialized source queues,
split/merge, dual brains, wizard/campaign creation, training and original World
budget/cadence remain pending. A bounded caller comparison cannot establish those
excluded services or a complete original AI implementation.

The first advancing-clock development comparison exposed a missing kind6
`tick <= preparationTick+22` deadline in NS82's logical test projection. NS82's
fixed-clock slices did not exercise that limit. The native production planner
already guarded that deadline; the reusable fixture now does too. Initial paused
state also required explicitly matching the authored selected-value field. These
development mismatches are not accepted execution evidence.

## Native integration and checkpoints

Fresh sessions carry an optional scheduler in CMD1v16. Planning precedes execution,
each receives half the authored native allocation4, result/invalidation flags stay
separate and ordinary expiry/result/progress cleanup removes the active task.
The pending root phase, execution serial and execution-result flag are serialized.
Native world tick, existing fresh-sweep caches, origin-change probe restart,
command suppression and single active-task replacement remain native policies.
Displaced simultaneous original task maps are not reconstructed here.

Older checkpoints decode with the scheduler absent and keep their original native
cadence, event policy and stored lifetimes. Writing a scheduler to a pre-v16 format
refuses rather than dropping its state. Native comparison includes explicit legacy
journeys as well as scheduled actual-event/expiry journeys, portable restores,
malformed state refusal and transactional frame rollback.

Execution evidence is registered after fresh, prospectively bound runs. Static
export and development failures are retained separately and do not promote
comparison or integration status.


## Accepted original execution

The prospectively bound original32/native64/ASan+UBSan+LSan run agrees across
64 episodes,14,080 caller slices and4,865,080 checks. It includes12,142 body
invocations,3,503 pending-root slices,384 same-call new-task executions,192
result completions and320 retirements (including64 progress retirements before
expiry),64 expired skips and384 disabled calls. Seven native malformed-domain
refusals are separate from original equivalence. Full strategic RNG, installed
features/task fields, command sequences, result flags, pending queue/phase and
execution budgets match; the task count budget may overshoot by its fixed8 debit.
Original text, executable, declared sources and compiler inputs remain stable.

The progress branch routes an explicitly supplied accounted quantity1,000,000
through actual514990 into the installed task, then advances age to three quarters
of its produced lifetime. This proves dispatcher/caller behavior with that input;
it does not recover the original scoring producer. Native actual-health credit
and portable integration have their separate evidence contract.


The one-byte432fb0 `RET` is outside the current Ghidra function bodies. The
register retains a one-byte recovered-range exception, backed by pinned entry
bytes and actual task6 expiry dispatch in the caller comparison. It does not
invent a broader function boundary or remove remaining unknown ranges. The
first bound report remains historical after adding this exact original-range
link; a second prospectively bound run supplies the current contract and anchor.


## Accepted native execution

Fresh native64 and ASan/UBSan/LSan runs agree over276 prospectively declared sources. The scheduled actual-event journey passes2059 checks/1900 restores, actual damage cost34 and natural expiry cleanup for damaged and undamaged controls. The separately retained legacy journey passes3807 checks/3650 restores and retains the undamaged expired task. Two512-frame installed-model seeds pass1074 checks/128 restores/39 refusals. Both five-test core suites, visibility/knowledge/danger regressions, six cold-process restores and CLI whole/split identity pass. Original/installed inputs, declared sources and compiler closure remain unchanged. Runtime linkage requires neither Wine nor Qt. This establishes bounded native integration, separately from original caller equivalence and the pending World/source services.
