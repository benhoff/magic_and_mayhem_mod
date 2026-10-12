# Creature control: reach, stun and bounded autonomous AI

NS25 adds isolated native models of No-CD build `40209ca7` and a native encounter
that composes them. Understanding, original comparison, native integration and
live replacement remain independent. Full original AI remains incomplete.

## Recovered contracts

* `50bd60`: wrapped XY, 1/2-cell footprints, vertical overlap, moving target
  endpoint and conditional diagonal `4f4ba0` query. Terrain/collision is supplied.
* `50c930`: eight-sector relative direction and signed tie conditions.
* `4f4b30`: toroidal weighted distance; not Euclidean distance.
* `50d060` and `50f590`: ordinary stun admission, pending behavior, engagement,
  mode/timer, animation-stop reentry, expiry, colors and audio helper ordering.
  The comparison uses empty owned effect queues and supplied transition helpers.
* `521160`: always consumes the initial RNG word, requires a roll above50,
  probes opposite then permitted flank directions, layers0/-1/+1, rejects unsafe
  status/behavior/motion, and stops after the first legal move request even if
  that request fails. `521420` supplies the independent knockback response.
* `50f7c0`: ready handler can call the behavior callback twice in one tick and
  reads callback-mutated cooldown before continuing.
* `518420`: complete branch precedence with explicit helper returns; execution
  of helper callbacks is not a claim that their bodies were recovered.
* `518750`, `5189e0`, `518c50`, `518f80`, `519100`: bounded assistance, home,
  combat scoring and corpse selectors. Strict score ties preserve slot order.
* `519330`: ordinary recent-attacker branch; the Evasion matrix is a health
  strength threshold, not a pursuit radius. Negative/admission-failed entries
  choose substate2, and successful positive admission chooses substate1.
  Its hostile spell46 scan remains pending.

An early static stack interpretation suggested that knockback requested X=0.
The retained negative comparison `compare-dev6` disproved it: intervening pushes
shift the stack reference to the probed candidate X. The model uses the actual
candidate coordinates. Do not propagate the rejected interpretation.

## Native composition and boundaries

`game/simulation/creature_encounter.*` owns targets, timers, cursor/RNG blobs,
footprints and atomic checkpoint continuation. The application profile adapter
supplies recovered rules through an immutable interface; the service has no Qt
or build-specific reconstruction dependency. The Qt controller owns its timer,
while widgets express semantic actions.

The native fixture uses four type17 actors, installed HTH/FearHate/Assist/Evasion
matrices and Crocodile animation bytes. A flat toroidal BFS and occupied-corner
policy supplies movement and diagonal admission; attack24 and stun112 plus facing
are explicit native animation choices. One-tick knockback relocation, decision
cadence, corpse retention and a160-quiet-tick stalemate stop are native policies.
They are not reconstructed full movement/animation/AI behavior. No balance values
in game data are changed.

Remaining original boundaries include complete target/assist admission bodies,
spell and interaction selectors, hazardous-map producers, hostile spell46 scans,
all-species special states, effect/queue ownership, action selector composition,
shared AI/movement scheduling, command behavior and live replacement.

Reports are retained under `working/tests/creature-ai-ns25/`. Final source-bound
execution records and their counts are registered separately after validation;
development successes do not silently establish current final-code validation.

## Final execution evidence

The current scoped original and native control comparison passes97,516 cases:
29,178 reach,31,752 relative-facing,3,840 knockback admission,1,454 stun/response,
60 callback-mutated ready cases,18,432 controller decisions and12,800 selector
fixtures. Original target text and full supplied creature records are guarded;
all compiled translation-unit dependencies are recorded separately. The original
manifest passes before and after every comparison.

Normal and ASan/UBSan/LSan native runs each pass6,397 one-tick checkpoint clones,
36 fresh-process continuations and8 semantic malformed-state refusals. Seven
seeded encounters plus a controlled lethal-target fixture exercise stun,
knockback, movement, death, retargeting, victory and stalemate. The normal run
also exercises actual Qt Play/Pause/Step buttons, timer, API save/reload, failed
load preservation and final snapshot equality with the headless process.
Qt sanitizers and actual picker interactions remain pending.

Seed1 finishes at tick812 with two surviving allied creatures at26 and66HP.
The reports are `creature-control-original-v2-20261012.json`,
`creature-control-native-20261012.json` and
`creature-control-sanitized-20261012.json`. Earlier records remain historical;
no fingerprints were refreshed retroactively.

The shared Game/combat CMake additions also changed fingerprints of earlier
uncommitted health, melee, lab and skirmish milestones. New NS25 regression
records repeat the affected original comparisons, native lab continuations,
ordinary-health adapter, skirmish orders/continuations and actual Qt file-picker
flows. Their older reports remain immutable. This is build/source readiness
upkeep, not wider original gameplay equivalence.
