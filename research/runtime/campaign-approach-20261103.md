# Bounded campaign approach correction — 2026-11-03

This owned batch starts at local master `72c9418dc`. It investigates the failed
v29 software640 nominal route without changing original gameplay, leases,
helper/scenario deadlines, cast budgets or natural report assertions.

## Confirmed omission

The [retained original review](campaign-approach-results-20261103/approach-threat-diagnosis-01.json)
joins v29 gameplay and spell records with its helper metadata. At the cleared
bridge, wizard0 has141HP at31,52,4. Enemy6 is living, type11/owner1, at32,52,4,
targeting wizard0. Follower3 is living type14/owner0 with90HP at29,53,4 and
already targets enemy6. The old threat predicate admits only type10/owner2.
It ignores enemy6 and advances toward Camulos.

Two original enemy6 melee events after bridge arrival reduce wizard141→127→111.
Six original Camulos Fireball events then reduce111→91→71→51→31→11→0.
The ignored attacker is a confirmed omission and source of30HP loss after the
bridge; it is not the sole cause of defeat. Another24HP melee loss happened
before exact bridge arrival. No original AI, damage or collision formula is
recovered by this join. The review's internal `troll` field names are labels;
no species-name identification is claimed for observed type11.

## Input policy correction

The driver now prioritizes living nearby hostile actors whose original target
is the wizard or a living owned Zombie follower. Camulos retains the separate
boss casting/portal path. Only a type10/owner2 threat may seed the existing
initial-bridge resupply identity. Other hostile types/owners no longer inherit
that identity. First selections are retained as bounded helper metadata; full
passive original events remain the authority.

Regression cases use the retained bridge pose, preserve the neutral bridge
attacker case, and reject dead/inactive/friendly/unassigned-owner/unengaged/
remote actors. Only a living owned type14 follower is protected. Ordinary
combat selection and defeat policy retain their existing behavior. The early
review pins an earlier authored test draft; its payload and exact draft preimage
are retained, and subsequent guards record the corrected exact-coordinate fixture.

Live outcome and current pixel comparison remain pending. The change does not
implement native commander orders or grant victory from targeting, casts, damage
or actor death. Reliable nominal progression/victory, projectile interception,
assembled-map identity, populated lifetime, save persistence and supported-player
acceptance remain open. Original files remain immutable.

## First original result and bounded follow-up

The [v30 enabled-intro software640 acquisition](rendering-progression-results-20261101/approach-v30-software-low-nominal-01.json)
enters the requested first World and exercises the corrected selection. The
first type11/owner1 attacker is killed through original29→9→0 player Fireball
damage. The helper admits three casts in that burst and observes two same-target
damage events; this does not establish where every projectile went. Wizard
health is141 at resupply and92 on exact bridge arrival. Later the support-order
witness refuses with wizard7HP/12mana and follower29HP. The whole acquisition
fails; it supplies no victory or pixel acceptance. Original manifests, source
closure and stopped private Wine pass; all314artifacts remain retained.

V31 interrupts post-refill bridge staging when a living attacker targets the
wizard or its valid follower within seven cells. At most four interruptions
are admitted; when no nearby engaged threat remains, the same exact bridge and
follower checks resume. Existing stage clocks are not reset. Type11/owner1
bursts use at most two affordable casts, then observe original damage before
any follow-up. These are authored input policies, not lethal/damage formulas.

[Current controls](campaign-approach-results-20261103/v31-controls-01.json) pass
132Python, sevenQt startup and30Region cases:169total. The first standalone
restricted-context unit invocation could not initialize its private X server;
its [failure](campaign-approach-results-20261103/v31-unit-restricted-context.json)
is retained separately from the successful host-context guard run. V31 nominal
outcome and independent pixels remain pending. The original900s scenario,
540s helper, cast budgets, sole bounded Space pair and natural-report authority
remain unchanged.

V31's [resupply review](campaign-approach-results-20261103/v31-resupply-threat-review-01.json)
confirms74HP loss113→39 to original type11/owner1 melee while resupplying.
The post-refill interruption never runs. V32 therefore suspends either resupply
or post-refill bridge staging for affordable nearby combat, then resumes that
exact phase. Initial bridge-death resupply starts only once; suspension cannot
restart its clocks. All four-interruption, stage,540s/900s and cast limits remain.

A three-second original melee-source/victim witness handles an attacker whose
target field briefly clears between actions. Current living type/owner roles,
actual applied health loss and victim identity must match. Stale, dead, changed
roles and unapplied damage refuse; unsigned original tick wrap is tested. This
is a temporal input heuristic, not stable object/generation or recovered AI proof.

[V32 controls](campaign-approach-results-20261103/v32-controls-01.json) pass
136Python/sevenQt startup/30Region cases:173total. Current original result and
independent comparison remain pending. The prior two failed acquisitions and
all source preimages remain immutable.

## Closeout: original outcomes remain failures

The [verified three-case review](campaign-approach-results-20261103/closeout-review-02.json)
retains v30, v31 and v32 as separate failed nominal software640 attempts. V32
enters first World and preserves182HP from resupply start through completion,
then spends recovered mana fighting owner1/type10 escorts and ends in the
original `Defeat!` report, discriminator0. No support-order or Space burst is
recorded. All291artifacts, source closure, stopped private Wine and immutable
original manifests pass their retention checks. No pixel comparison is run for
a failed semantic acquisition.

V32 records no type11 Fireball/melee damage and no route interruption. An escort
kill and live suspension validation are therefore not established. The182HP
resupply observation is not attributed to the unexercised suspension branch;
these source/time/RNG variants are not a matched causal comparison. V30 remains
the original proof that broader selection can engage and kill the omitted
attacker. V31 supplies the exact74HP resupply-loss evidence.

Replaying V31's actual helper-observed resupply poses against current V32 code
admits the new combat gate at the113HP before pose and69HP arrival pose, including
the transient cleared target field. It refuses at the39HP completion pose where
the attacker is remote. This is retained-input decision eligibility, not a new
original-engine execution or proof of the resulting fight/outcome.

Current173controls pass. The new suspension/recent-melee branch has synthetic
controls and retained-input eligibility evidence only. Reliable nominal
victory/progression, support-order camera/selection/aim/timing and death
interference, target priority/mana recovery, assembled-map/RNG identity,
populated object/generation lifetime, same-role slot reuse, save persistence,
full simulation and supported-player acceptance remain open. Existing evidence
fingerprints and engine statuses are preserved.

The prepared owned checkout is
`working/worktrees/campaign-approach-20261103`. Its current preparation is
`research/runtime/rendering-progression-results-20261101/approach-v32-inputs-01.json`.
Use a new report name for another bounded case; current source, private game,
launch manifest, prefix and production build are pinned there. A different
checkout needs its own preparation. Strict requested-outcome/report/Realm/fresh
World/cleanup assertions remain enabled; diagnostic skips never grant nominal
acceptance.
