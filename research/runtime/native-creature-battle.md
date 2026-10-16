# Installed creature rules in the shared native battle

A separate versioned battle-rules section binds the existing
Crocodile/type17 adapter to the native arena's authoritative actors and health.
The selected recovered selectors, reach, facing, melee RNG and stun/knockback
contracts remain those documented in creature-control.md. New orchestration is
native composition, not a whole original encounter or scheduling comparison.

The intended bounded domain is same-profile ordinary creature versus creature.
Stationary wizards remain under the authored arena rules and are excluded from
the type17-only melee/selector projection. Asset bytes and every mutable RNG,
ANI, target/generation, recent-attacker and interruption field belong to portable
continuation. Actor health is projected from CombatState only while calling rules
and is never duplicated in the stored rule section.

Arena navigation remains its existing native point driver; an additional native
2x2 standing/edge check admits Crocodile footprints, including wizard obstacles.
Attack24/stun112 directional animation selection and four-tick route requests are
native policies inherited or adapted from the lab. Installed map construction,
full original movement/cadence, wizard/special-species melee, complete AI, original
Sphere collision catalogs, physical sprites/audio and live replacement are pending.

The driver binds separately from effect ownership through `MNMBAT02`. Normal
admission runs recovered selection/ANI-event resolution around native movement,
then the existing bolts/Cure/Sphere and outcome phases. Full slot/generation
handles survive compact selector projections. Health and death use the same
CombatState/HealthRules mutation as ordinary spell damage. A native stop clears
attack continuation; blocked footprint edges hold rather than overlap. Long stun
fixtures keep wizard AI quiescent explicitly; the playable wizard policy is
unchanged. A retained Sphere can revisit actors over a full toroidal lifetime;
the mixed fixture's first16 phases contain one20-point hit and one8-point Cure.

Development checks include120 restored RNG/ANI/stun continuations,
moving-target pursuit, death/retarget, installed360-health summoning, missing
binding/malformed-envelope refusal, removal/reuse, footprint refusal, stop and
failure rollback after spell admission and rule advancement. Fresh source-bound
execution evidence and the real campaign/window journey remain separate milestones.

[CREATUREB01 execution](creature-battle-bound-20261016.json) pins406 sources,
four native behavior contracts and five scenarios. Normal and ASan/UBSan/LSan
runs agree:8 melee hits,39 observed interrupted phases with recovery, one Sphere
impact and one Cure in the mixed16-phase fixture; pursuit, death/retarget,
generation reuse, rollback and checkpoint reconstruction pass. All58 legacy native
CTests, six additional sanitizer contracts and installed Cure checks pass.
Seventeen installed inputs and immutable originals match before/after execution.
The new-profile campaign/window journey remains pending at this milestone.

All-visible hostility, bravery30 and application-supplied admission remain native
adapter choices. Sphere keeps its existing declared one-cell collision fixture;
the new2x2 policy is standing/navigation/melee admission, not installed projectile
collision-catalog equivalence. No original configuration or balance data changes.

## Campaign and window integration

The installed profile is opt-in through `--installation ROOT` in the skirmish
window and campaign CLI. Native standalone checkpoints resolve their embedded
profile. Campaign definitions fingerprint the exact initial installed battle;
matching installation bytes must be supplied on create/load/inspect/start/cast/
resume, even between duels where no battle world remains. This reuses the existing
campaign resource-binding boundary without changing its wire format. Missing or
mismatched binding refuses before publication. Legacy campaign profile fallbacks
remain independent, and next/retry keeps the selected installed driver.

The controller translates saved rule phases to Attacking/Pursuing/Recovering/
Stunned/Knocked back/Blocked and a Crocodile label. Widgets consume those semantic
views, display the native2x2 footprint and pick all its cells. Headless inspection
adds `creature_rules` only for a bound profile; legacy trace bytes remain unchanged.
Explicit arena/ordinary-health admission bounds the adapter's supplied world.

The retained development window journey selects Cure and Sphere through actual
secondary footprint cells, saves at phase2 during attack/flight, pauses and resumes
14 phases in fresh Qt, CLI and event-counting processes. Campaign bytes agree;
the tail contains two melee hits, one8-point Cure and one20-point Sphere impact.
A second journey queues a player move, saves during movement and resumes eight
phases while the enemy pursues; all three consumers agree and the enemy moves.
Missing campaign binding refuses without changing its input or creating output.
Idle campaigns reload/start with their selected profile. Native tests additionally
force victory/defeat, restore between missions, verify next/retry driver binding
and reject a second settlement. These are native resource/progression policies,
not a recovered original campaign or wizard AI comparison.

The initial407-source positive window run is preserved separately as
[CREATUREB02 initial execution](creature-window-initial-bound-20261016.json),
with its exact intermediate sources and two binaries under
`working/tests/creature-window-initial-bound-20261016-v2/`. It predates the
subsequent interruption safety review and does not establish current-code or
exhaustive move/stun interaction validation.

The review found that cancelling native movement without cancelling move control
could fail publication, and broad target/hold resets could clear a physical stun.
The new semantic-order regression reuses an actual pre-stun opaque continuation,
without changing RNG algorithms or installed balance. Linked against the retained
prior arena archive, it fails with `ordered movement/control mismatch`; that
failure and link/input provenance remain under
`working/tests/creature-stun-prior-backend-20261016/`.

The fix clears a stunned mover's destination/control atomically, preserves stun
ANI/timers through Stop and target death/removal, advances physical recovery while
held, and refuses new move/attack orders during interruption. Held actors display
Stunned rather than hiding that state behind Holding. Final source-bound safety
validation remains a separate execution from the initial positive window run.

[CREATUREB03 final execution](creature-window-bound-20261016.json) now validates
these safety branches on407 current sources, four native contracts and six
scenarios. Native and sanitized results agree, including interrupted-order
refusal and physical stun across target death/removal. All58 legacy CTests, six
additional sanitizer contracts, installed Cure checks and both retained new Qt
journeys pass.17 installed inputs and original manifests match before/after.
The new creature milestone is native preview; original full AI/movement/cadence,
installed world/collision catalogs and live replacement remain independent.

The runnable local copy is `working/build/creature-battle-20261016/mnm-skirmish`;
use `--installation working/game-nocd --campaign` for the installed profile.

The consolidated application also hosts installed Plains terrain campaigns.
The optional creature driver is retained through mission-aware SkirmishSession
creation/restoration and remains selected only for arena resource definitions.
Map saves resolve their own resource/effect environment; loading a map/legacy
arena/installed arena replaces terrain presentation transactionally. The real
Qt profile-switch scenario covers both resource flags, New buttons, exact saves
and refusal without mutation when either required binding is absent. This adds
no recovered creature AI, terrain creature movement or original campaign claim.
