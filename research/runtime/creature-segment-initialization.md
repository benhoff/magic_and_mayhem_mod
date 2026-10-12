# Original full segment initialization (NS30)

Pinned No-CD `0x510e80..0x511c0c` extends the earlier ordinary forward slice.
Retained `working/decompiled/creature-behavior-lnbhltzt/motion_segment_setup.asm`
has SHA-256 `f2c197cee6275d4ec61d2926cee0c08ff194178bef270aa891dead39959f6f52`.
Static control-flow findings have high confidence for the pinned body; native
comparison, lower-service ownership and application integration are pending.

- Cursor exhaustion clears `b8b`. Types27/22 bypass516ee0; other actors return
  when516ee0 succeeds,6ac iszero, and516d60 succeeds.
- Direction608 is saved in614;610 is signed route vertical state, distinct
  from the motion consumer's nonzero vertical predicate. Camera-relative/reverse
  facing and overlay/sprite/auxiliary updates precede108/10c route assignment.
- Special723 uses73c; ordinary505840 base and505920 rate/sample-count run before
  acceptance. Positive778/77c modify the rate input, not the stored raw base.
-514360 must succeed and return the route mode. Failure invokes511c40 and
  action15: cooldown7 on success, otherwise phase2 and marker clear.
- Definition+5ad, protection8c9,723 and target/current522f50 gates can enter
  action15/cooldown5 before occupancy release/reservation.
- Continuity requires matching direction, signed vertical, action2 and mode.
  It subtracts192 from progress and sign-directed travel, retaining samples and
  residuals. Reset paths clear travel/residuals and may clear accumulator/rate;
  ordinary scalar work runs again after reset.
- Mode0/4, modes1/2/3 with descriptor3c, type12 special banks and vertical fallback
  select different animation bases and sample locations. Vertical fallback
  writes inline71e and may invoke50ad10 for a live hostile target.
- Reverse setup sums a bounded sample span using wrapped signed32 additions,
  divides by6, updates direction-specific residuals, selects the last sample,
  snapshots residuals/pointer and enters action2. Raw pointer/fault domains remain
  separate from native admission and require caller-staged rollback.

This work does not change the earlier forward adapter's scope or refresh its
historical evidence. Full setup comparison is the next bounded experiment.

Development comparison recovered4eb030 as a target-cell thiscall receiving
current XYZ, rather than a mode-valued fastcall. This supplied geometry service
produces the505920 argument; both cold calls preserve its ordering. Native
sample-bank selection is logical; the eventual composition layer must bind its
owned read-only sample storage to the motion consumer. No full ANI producer or
World/session integration is claimed here.

Malformed route modes above4 jump through511ae0 and can read the uninitialized
local sample-count slot on special723 paths. The failed development record is
retained at compare-dev4; the comparison cannot assert launch-independent state
for that stack-dependent domain. Valid modes0..4 remain the recovered input
domain; native refusal of malformed modes is intentional admission policy.

The finalized development port passes24576 paired cases plus ten native-only
refusals. All ten standalone native CTests pass. Profile and direction table
bytes are pinned within the guarded510e80..511c40 region. The five mode targets
are5115a1,5115ee,5115ee,5115ee,511a96; reverse direction0..7 targets are
511b39,511b43,511b63,511b73,511b4d,511b55,511b61,511b71. Direction2/3 entries
intentionally enter after negation instructions shared with direction6/7.

The test compares every actor byte and ordered lower-service calls, preserves
read-only definition/sample/terrain/visual storage, covers all mode/vertical/type12
profiles, and checks early/precheck/fallback/hazard/continuity/reverse/hostile attack
branches. The ten refusal domains are invalid route count/mode/world dimension,
planar or layer position, zero distance divisor, target/either owner and missing
owned samples. They require caller-staged rollback, separate from baseline
equivalence. Final prospective source/scope-bound execution is in progress.

Fresh final execution in `original-creature-segment-setup-20261012.json` passes
24576 paired cases and ten native refusals, with stable compiler/declaration
inputs and original manifest checks before/after. AI.segment-initialization
records scoped implementation/comparison; NP.segment-setup-admission records
separate native policy. Integration/replacement staynone. All ten native CTests
pass. The next original scope is whole actor route request and displacement.
