# Original creature execution recovery

Target: compose complete original AI in the native simulation. NS26 controller
and autonomous handlers remain independently validated; execution work is new
and pending comparison. No gameplay balance changes or live replacement claim.

Build: No-CD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Preferred-image addresses are static evidence, not stable host pointers.

## Recovery boundaries

- `52a780`: rotating route budget and staggered 20/90-slot maintenance.
- `519ae0`: complete resumable route continuation and failure handling.
- `522ad0`, `5244f0`: periodic terrain relocation and creature valuation.
- `523be0`, `5243b0`, `512220`: queued orders and delivery.
- `50aa30`, `50ab30`, `50a930`, `50db60`, `50da50`: behavior/action transition and dispatch.
- `50b2c0`: ranged aiming.
- Scenario/strategic policy and effect/action ownership need caller discovery.

Static recovery and implementation will be recorded per behavior. All new
execution, integration and replacement stages remain pending until fresh tests.

## Storage authorization

The user authorized storage as needed for completing the AI work. Bounded builds
and experiments use a private 90GiB reserve policy under `working/retention/`,
while the repository default stays at 100GiB. Cleanup is not authorized.

The retained export label `periodic_follow_work` for `5244f0` is misleading: its
body computes combat valuations from all 28 HTH/response matrices and statuses.
`524460` installs the actual follow reference; pursuit resides
in `513850`. `523470` owns strategic hostile selection and roaming.
This correction follows instruction reads and writes, with high
confidence within those bodies.

## Additional development comparisons

The action/periodic, queue/follow/reference, strategy and behavior fixtures have
passed 28,672, 32,768, 8,192 and 53,248 paired original/native cases respectively,
with native64, original32 and sanitized64 outcome agreement. Together with the
24,576 control/route cases, these are 147,456 development cases. They have no
prospective declarations and are not current readiness evidence. Fresh, scoped
source-bound runs are still required after the complete source closure stabilizes.

The strategy fixture initially omitted the alternate engagement release's write.
Its second development build raced a shared header edit; that run is invalid as
a behavioral comparison. The stable third run passed. All negative artifacts
remain retained. Future runners must fingerprint their complete source closure
even for development, to refuse mixed-version compilation.

Per-frame `509e50`, movement behavior `510010` and death behavior `50d580` are now
instruction-derived native ports with independent comparison fixtures in progress.
The first lifecycle comparison found that lifespan admission (`49d070`) branches
past immediate removal when its marker was previously clear. The native model
now preserves that branch. Owner upkeep, enabled-display effect teardown and
contribution chains are separate explicit service domains still awaiting closure.

The original timer at `660` is separate from autonomous substate `684`; recent
damage timer `6b0` is separate from the decision age field `811`. Maintenance
stores these distinctly. The `94` counter projects to the decision model's
Blood Lust presence flag; the full counter remains owned by maintenance.

## Implementation in progress

Logical state and typed world services live in `reconstruction/creature-ai/`.
New source files cover scheduling, transitions/orders, route continuation,
periodic terrain relocation/valuation, ranged tactics and strategic roaming.
They are independent of Qt and private PE mapping. Existing NS26 models and
source-bound evidence remain unchanged.

The development fixture passed 20,480 cases across seven scheduler/transition
bodies, then 24,576 cases across those bodies plus route continuation. Both
native64/original32/sanitized64 runs agreed. These development runs preceded
subsequent source/header edits and have no prospective claims: current-code
status promotion and all remaining execution/composition require fresh binding.
Raw runs remain under `working/tests/original-ai-execution-20261012/`.

Full-image textual follow-field discovery finds the actual execution entry
`513850`; `523470` is nonnetwork hostile/roaming policy, not follow pursuit.
`5244f0` is valuation. Earlier follow labels were hypotheses and are superseded
by instruction evidence. Follow lifecycle repair also references `508b00`.

All source additions remain work in progress. The final accounting pass must
reconcile the census, review exact changed source versions, register fresh
prospective comparison claims and rerun appropriate independent regressions.

`513850` follows a living reference only when raw XY separation reaches three
cells on either axis. It normalizes the destination, supplies the follower's
movement descriptor to `4f5230` with slot argument `-1`, and issues a move only
on success. It does not use wrapped distance for the initial separation gate.
`508b00` clears a removed follow reference irrespective of actor health, then
repairs main/alternate targets and engagement counters for living actors.
Type5/action10 or20 can request action25 after repair. New native projections
retain these ordered effects; their independent comparisons are pending.

The first valuation comparison caught a stack-relative mapping error. `+8ed`
is the scalar added as a percentage to HTH; `+124` selects the two global status
bonuses. They are named `valueScale8ed` and `bonusMode124` without assigning
unconfirmed gameplay meanings. The failed result remains in `compare-actions-dev2`;
corrected comparisons are underway.

Further comparison corrections: valuation includes the type20 modifier from the
`5248f4` table; action23 joins action24/25 in overlay repair when kind1 is
present. Valuation preserves x87 operation order across the final integer
conversion; associating the factors differently produced a one-unit mismatch
in fixture329. Queue duplicate adjustment receives requested Z, as directly
observed in fixture21; an earlier stack-relative hypothesis of zero was rejected.
All negative runs remain retained; none establishes current validation.

Hardware-breakpoint observation of fixture329 (`valuation-debug2`) confirms x87
control word `037f` (64-bit extended significand, nearest arithmetic rounding)
and final value `478114.9978123284228`. `59bee0` switches to truncation, converts
with `FISTP QWORD`, restores the control word and returns its low DWORD. Invalid
conversion yields low DWORD zero, not signed32 saturation. Native decimal float
literals gained excess precision in the i386 build; constants now use the exact
original binary values expressed as hexadecimal long-double literals. This
finding is high-confidence for the isolated fixture. Original live startup FPU
control and cross-architecture numerical equivalence remain separate boundaries.

## Full motion action (NS28)

`5104b0..510e3a` now has a complete control-flow projection in
`full_motion_action.*`. High-confidence instruction findings within this body:

- `b43` stores each sample increment; `b4f/b53` store its signed XY deltas.
  Vertical movement clears both deltas. A direction above seven retains their
  previous values; it does not clear them. All accumulator/product/displacement
  arithmetic wraps as signed32 before truncating division and height clamping.
- Type12/mode2-or3 forces a32 increment and one iteration after positive quotient
  admission. Each iteration reloads the sample and class/mode predicates.
- Forward/reverse sample cursor updates, flying/non-form bypass, event1/2 rewind,
  terrain footstep selection, and type4/12/15 event6 effects precede completion.
- Completion checks the actor again after512460. Reverse behavior14 flips facing,
  conditionally binds overlay/sprite/auxiliary presentation, resolves pending2/3,
  missing targets, pending5 and pending8, and releases engagement as appropriate.
- Ordinary pending8 releases the route effect and every kind2 queued effect in
  list order. Response, displacement pause,22-byte movement-query construction,
  legality refusal, type27 reset and behavior16 admission preserve their order.

The development corpus has57344 paired cases, including8192 extreme signed32
cases and11466 out-of-range direction inputs. Main and alternate engagement
release, missing reverse targets, every visual path, both effect kinds, terrain
cues, queued-effect release and movement failure are checked as nonempty branches.
Six native-only refusal cases cover zero/overflowing division, excessive substeps,
invalid sample cursors, an endless supplied event stream and invalid world size.
They are intentional admission policy, separate from original behavior. Refusal
is handled transactionally by a caller's staged state; this primitive does not
promise in-place rollback.

The original fixture executes the entire motion body, selected unchanged50a930
resolver arms and unchanged coordinate constructors. It compares ordered service
calls, every actor byte, queued nodes, guarded sample storage and read-only
terrain/visual tables, and verifies motion/resolver text is unchanged. Lower
segment/behavior/action/response/ANI/visual/sound/effect/stun/queue/geometry services
are supplied with explicit ABI assertions. This is headless projection evidence;
real ANI timing, full512460, world geometry/ownership and application integration
remain separate. Current actor grid positions are valid16x16x4 coordinates; raw
invalid pointers/division and broader coordinate-constructor domains are excluded.

A prospective final declaration and fresh original/native/sanitized run are
required before promoting this new scope. Earlier motion slices and development
reports remain immutable; none is silently upgraded to this full projection.

The prospective finalv2 run in `original-motion-action-v2-20261012.json` passes
57344 paired cases and six native-only refusals with native64, original32 and
ASan/UBSan/LSan64 agreement. Distinct widths1..2/heights1..4 and movement kinds0..7
verify the legality-query field ordering. All eight native executor CTests pass.
`AI.motion-action` now records this scoped comparison; `NP.motion-admission`
records the separate native policy. Native application integration staysnone.
The first scoped result and all development runs remain historical.
