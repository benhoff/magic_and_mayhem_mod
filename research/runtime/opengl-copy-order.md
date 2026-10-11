# Opt-in bounded copy scheduling

The original tutorial summon observation exposed concurrent successful copies:
outer return PC `0x58c488` on thread452 while its source changed through a copy
at return PC `0x58c05d` on thread408. These sites are already scoped under
`RI.copies`; semantic caller ownership and original driver read timing remain
unknown. See the preserved [prior observation](opengl-world-summon-observation.md).

## Intentional native policy

`MNM_RENDER_ORDERED_COPIES=1` opts a continuous producer into one process-local
copy owner, spanning hook admission, the original Blt/BltFast call, native commit
and existing capture work. The shared owner prevents another hooked copy from
mutating source/destination between those boundaries. This changes original copy
scheduling; it is not recovered baseline behavior. Default behavior is unchanged,
and the flag is ignored outside continuous mode. It creates no COM references,
new borrowed ownership, worker or wire fields. Existing resource/queue/recovery
caps and successful-original-only commit checks remain.

The lease is separate from the tracker guard: no tracker is held across a wait
or original operation. A contending thread yields for at most a 50ms tick-based
budget (scheduler pauses can exceed that measured budget). The owner is released
on every normal return, including original failure, by a scoped cleanup. Original
arguments, return result and thread-local LastError are preserved.

Same-thread reentry never waits and never owns the outer lease. Existing generation
checks still refuse a successful nested write that makes the outer copy uncertain.
A cross-thread timeout forwards the original exactly once without producer capture.
It marks any active legacy capture history incomplete under its existing atomic
invalid flag, then advances the pixel epoch and queues target uncertainty both before and after
that unobserved call, preventing a pending copy from committing an interrupted
history. Original success is not invented; neither timeout nor nested reentry is
silently converted into a valid copy. Other mutation families (locks, DC, palette,
flip, lifetime/property changes) remain governed by existing generation/epoch
checks; this gate does not serialize the whole DirectDraw API.

Bounded diagnostics are `copy_order_wait_acquired`, `copy_order_timeout` and
`copy_order_reentrant`. Copy scheduling never waits indefinitely for a consumer.
If an original method synchronously needs another thread's copy, the finite wait
can time out and invalidate the native stream rather than deadlock. This chunk
does not assert a complete original callback/dependency graph or clean abnormal
thread termination while a lease is held.

## Independent fixture validation

The initial [six-case matrix](opengl-copy-order-native-20261006.json) compares 408
complete native frames. `cross-copy` and `cross-fast` launch a real second Wine
thread while an original fake primary copy is pending. The worker copies that
primary into its source surface; it must enter its original only after the outer
commit. Independent fixture pixels, original counts/results/LastError, native
frames and orderly resource retirement are checked. Both calls share the gate.
`copy-timeout` makes the outer original wait for the worker: the worker must
forward after the budget and both native updates must remain uncertain, ending
in GAP. `source-write` reenters on the same thread and retains refusal without
waiting. Identical metadata and mixed RGB16 regression each compare 201 frames.
The source guard confirms no fingerprint changed while the matrix executed.

Reproduce:

```sh
xvfb-run -a python3 tools/test-render-mutations.py working/build/renderer-live --case cross-fast --case cross-copy --case copy-timeout --case source-write --case meta16 --case mx16
xvfb-run -a -s '-screen 0 1800x1000x24' python3 tools/test-live-render-routes.py working/build/renderer-live --mode campaign --ordered-copies --require-world-summon --world-seconds 15
```

The second command requires an optimized native probe, Tesseract, original UI
prompts/count transition, Active native publication through the summon and stable
terrain/portrait regions. Independent captures are unsynchronized and do not
compare animated actors, effects or full World pixels. Original media are checked
before and after; no binary mutation occurs outside existing preparation scripts.

Pending: repeated active gameplay across more scenes, original callback dependency
mapping, other mutation-family concurrency, synchronized actors/effects/full-frame
comparisons, hardware driver equivalence, and any decision to enable this scheduling
policy by default. Existing historical evidence hashes are retained.

## Initial source-stable original result

The [strict live record](opengl-copy-order-live-20261006.json) passes with the
original Zombie summon confirmed independently and native instruction/count
regions matching. Native publication remains Active through three post-summon
terrain/portrait comparisons and forced World recovery. It publishes
308 further frames over25221ms after World recovery,
finishing at653 frames with zero terminal resources and no ordinary native
readbacks or viewport uploads. All13 named comparisons pass within tolerance;
source fingerprints and source preferences are unchanged. All2,927 immutable
files pass verification before and after. This finite run does not prove
repeatable unrestricted gameplay or animation equivalence.

Exact committed history `01a66c2..89ab1db` was reviewed separately against
parent/current file and behavior receipts with no unresolved accounting gaps.

The final [source-verified matrix](opengl-copy-order-native-verified-20261006.json)
repeats all six cases and408 independent frames after adding the legacy-history
invalid flag on timeout. Earlier result fingerprints remain unchanged; they do
not claim validation of that final edit. Timeout with legacy archive replay
enabled remains a separate pending test.

## Final source and remaining refusal

The [final strict attempt](opengl-copy-order-live-verified-20261006.json) confirms
the original summon and matching native instruction/count plus first sustained
terrain/portrait sample. At the second sustained sample, native publication
refuses at497 frames. This is a failed strict test, retained as negative evidence.
The [diagnostic review](opengl-copy-order-live-refusal-20261006.json) verifies all
recorded source fingerprints still match and preserves the bounded lifecycle log.
It records a pixel-tracker miss at surface token `0x01783310`, then GAP3. The
subsequent copy refusals are unprepared; there is no prepared-copy generation
conflict or copy-gate timeout in this bounded trace. These omissions are scoped
to the bounded log, not proof that no other operation occurred.

The original process ran through its confirmed summon and the script verified
all2,927 immutable files before and after the attempt. The normal native finish
request was not reached; the test's failure cleanup stops both processes. Fallback
status shows zero resident surfaces/palettes, but this does not assert END/drain
acknowledgement. The initial passing strict record predates only the timeout
legacy-history invalid flag; its fingerprint is preserved without claiming it
validates final source. This chunk establishes the opt-in scheduling mechanism
and independent cross-thread ordering, not repeatable native active gameplay.
Next: classify the contended pixel operation and tracker hold causing the miss,
then validate bounded ownership across those other mutation families.

## Current crowded-battle follow-up (2026-10-10)

The [fresh twelve-case PE32/GPU result](native-surface-order-passed-20261010.json)
uses prospective contract/source claims and its own Wine prefix. Cross-thread
Blt/BltFast, writable locks, DC release and flips retain complete pixels and
original call counts; copy/lock/DC/flip timeouts and same-thread source mutation
end in explicit GAP. Seven valid sessions retire resources and acknowledge END;
five intentionally uncertain sessions refuse. Identical metadata and mixedRGB16
regressions bring the total to418 complete independent pixel comparisons.
Original HRESULT/LastError assertions remain in the actual original-call fakes.
No runtime renderer algorithm was changed by this follow-up.

This confirms current synthetic refusal behavior, not the scheduling cause of
the historical original-game GAP under rejected Sleep1 pacing. The separate
[three-minute crowded native battle](native-crowded-passed-20261010.json) has
zero fallback/recovery. Arbitrary original callback dependencies, interrupted
owners, actual driver timing, other maps and longer hardware sessions remain
pending. Historical evidence and its source hashes are preserved unchanged.


2026-10-11: contending copy-lease acquisition now uses `Sleep(1)` rather than repeated `Sleep(0)`, allowing the holder to run. The50ms tick budget, atomic immediate admission, same-thread nonwaiting reentry, original-once forwarding and conservative timeout remain intact. [Eight independent fixtures](native-copy-contention-yield-fixtures-20261011.json) pass13 complete frame comparisons, four successful sessions and four expected GAP refusals with fixture original counts/HRESULT/LastError checked;30 compiler-native inputs match the725 predeclared source closure. Sleep precision, fairness, abandoned-owner behavior and original-driver scheduling are not proven.

[Full public gameplay](native-draw-public-nvidia-gameplay-pass-20261011.json) now passes under the private accelerated NVIDIA/zink display with DirectDraw disabled/absent: casting/combat/movement/spells, nine portrait cycles, Mini/resume and normal Quit, with no fallback/recovery. Combat command/paint33.3999/25.1262FPS over131.138seconds; resumed59.8223/31.5958 over60.546seconds. Worst resumed window18.2648FPS remains explicit. Portrait16.994seconds averages20.0659FPS, worst19.1431 and maximum stable face channel error1. Previous hardware timeout failures remain unchanged. These are different live scenes, so no controlled speedup or guaranteed contention elimination follows. Final Qt renderer identity is cleared on normal Quit; keeping its last active identity is still a provenance improvement.

## Address-based lease waiting — 2026-10-11

Two fresh default NVIDIA campaign attempts still time out after the earlier
Sleep(1) change. The50ms budget is retained. Contended acquisition now waits on
the observed atomic owner value and release wakes all address waiters. The
first CAS, same-thread reentry and conservative original-once timeout remain.
Release preserves LastError around the wake call. Diagnostics report waiter
elapsed time, holder elapsed time and wrapper source lines. No fairness,
controlled speedup or universal frame-rate assertion follows from this policy.

Installed Wine Kernel32 WakeByAddressAll is an unimplemented stub; its failed
synthetic attempt is retained. The builder imports the implemented KernelBase
address-wait exports instead. This targets modern Windows/Wine and does not
claim recovered original Windows98 scheduling. Eight fresh independent fixtures
pass13 complete-frame comparisons, four valid sessions and four expected GAP
refusals with original call counts/return/LastError and native resource cleanup.
Thirty linked native compiler inputs and42 bridge/compiler/builder inputs match
129 prospectively declared sources. [Fixture evidence](native-copy-address-wait-fixtures-20261011.json).
Live gameplay robustness remains a separate milestone.

The full default NVIDIA campaign after address waiting still fails. Fresh holder
diagnostics identify BltFast ownership spanning53ms and117ms; this distinguishes
actual long ownership from Sleep timer granularity. The fixture pass does not
fix or promote live gameplay robustness. Stage profiling of that critical
section is the next investigation; the deadline is not extended.
