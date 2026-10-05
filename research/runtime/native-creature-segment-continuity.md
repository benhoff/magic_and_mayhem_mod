# Native planar segment initialization and continuity (NS04)

Scope: recovered planar category-zero setup, a headless one-creature native
adapter, and deterministic native-v4 continuation. This does not establish a
playable native world, ANI event production, original save writing or live
replacement. NS03 evidence remains the historical validation of its v3 policy.

## Recovered contracts and confidence

The pinned No-CD PE SHA-256 is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The complete setup entry is `0x00510e80`; static export is
`working/decompiled/creature-behavior-lnbhltzt/motion_segment_setup.asm`.
`tests/test-original-segment-setup.py` verifies this hash and every redirected
entry's bytes before running the original in a disposable i386 process.

55,296 cases cover eight new and old directions, previous actions 0/2/15,
previous categories 0/4, old vertical state, preservation candidate, acceleration
0/64/500, prior rate 0/30/720, and boost/slow modifiers. The original scalar
helpers (`0x505840`, `0x505920`), wrap helpers and signed direction helper
`0x4ead10` execute unchanged. Eligibility, accepted category zero, occupancy,
animation selection and action transition are controlled dependencies; rejected
eligibility and their full side effects are outside this comparison.

High confidence for the tested contracts:

- Scalar adjustment occurs before the continuation decision. A matching moving
  action/direction/vertical/category carries progress minus 192 and cumulative
  directional displacement minus one segment, preserving rate, accumulator,
  sample cursor and cycle residual snapshots.
- A reset selects the sample bank by direction parity and resets displacement,
  residuals and sample/animation state. Original action 2 retains admitted
  speed/accumulator through turns; other actions require a preservation candidate,
  unchanged vertical state and signed direction difference at most one, or reset
  speed/accumulator. The reset branch applies scalar adjustment a second time.
- The shortest signed direction helper is deterministic; no RNG is introduced.
  Carry branches return through different instruction paths but all preserve the
  initial sample pointer and initial residuals. Snapshot assignments at
  `0x511b94` precede the shared action transition at `0x511bb8`.
- Motion sample pointer (`+b8f`) and initial pointer (`+b93`) are separate from the
  supplied animation clock. Cycle reset restores initial residual X/Y (`+b83`,
  `+b87`). The comparison checks these fields in addition to rate/progress.

The extended `0x5104b0` regression compares 960,696 action transitions, including
zero-rate and separate sample/animation cursor cases, plus 136 original route
consumption/coordinate-snap cases. Animation events and environmental callbacks
are controlled; the comparison does not recover the original ANI event stream.
Both runners verify all 2,927 immutable original files before and after.
Machine-readable evidence and final source/artifact hashes are in
[native-creature-segment-continuity.json](native-creature-segment-continuity.json).

## Native integration and persistence

`move-continuous` opts into a planar category-zero driver through the app-owned
frozen navigation adapter. The native world remains independent of reconstruction,
Qt and injected hooks. Matching successive segments retain completed history
across waypoint consumption and route-prefix replanning. Grid consumption snaps
fine coordinates exactly as NS03 established; turn/setup uses canonical grid
coordinates and flat Z. New orders/cancellation clear history as a native policy.

V4 owns rate/duration, accumulator, carried progress/displacement, fine XYZ,
current and initial sample indices, animation clock, residual snapshots, previous
segment metadata and current-segment tick count. Restore binds the exact map
bytes, validates history against the profile/previous edge and reconstructs then
replays the ongoing segment before committing. History is bounded state, not
proof of reachability from every past tick. Restore replay is capped at 100,000
ticks. The 48-sample storage limit is explicit and out-of-range advancement fails
transactionally. Older v1/v2/v3 layouts and behavior remain compatible; current
storage budgets tighten to cover larger owned records.

26 normal and 26 ASan/UBSan CTests cover native lifecycle, movement, original
helper models and v1/v2/v3 regression oracles. The new independent Python v4
oracle verifies exact boundary/intra-cell bytes and a Python-authored save.
Fresh processes agree for 18 vs 6+12 ticks, a boundary restart, fractional-rate
22 vs 10+12 ticks, turning restarts and 70-tick prefix continuation. Malformed
checksummed continuation, driver mismatch and missing/changed resources are
refused. Unit tests cover rollback, failed in-place restore, order/cleanup reset
and sample cursor refusal. Leak detection is disabled in the traced sanitizer
environment, as in NS03.

## Remaining boundaries

The supplied twelve-frame event cycle remains native policy. Recover and compare
ANI frame/event production next, then widen setup to vertical/category-four,
reverse and special profiles with terrain heights and original eligibility.
Dynamic multi-creature occupancy, shared scheduling, AI/combat/spell callbacks,
campaign behavior and original version-20 save writing remain separate work.
