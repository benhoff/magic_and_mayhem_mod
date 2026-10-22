# Spell progress accounting — 2026-10-20

This is a source and register reconciliation, not a new engine execution result.
Confidence is high for the explicit implementation/test crosswalk. Whole-spell
equivalence and current-source execution remain separate, pending claims.

## Delivered work and its boundaries

The previous Core gameplay & spells selection included the 104 spell-dispatch
IDs and broad gameplay gaps, but omitted the `casting` subsystem and recovered
wizard task bodies. Simulation owned those component results. This made the
spell track's implementation count omit delivered spell work even though the
engine overview could already count the same components through Simulation.

The revised track includes casting, shared spell readiness, actor-target task
execution and Magic Sphere task execution, plus native spell integration.
Simulation exposes the same casting/task/encounter milestones. Shared behavior
IDs still count once in engine overview totals; adding a milestone is not a new
implementation or execution claim.

| Group | Contracts | Implementation at review |
| --- | ---: | --- |
| Casting foundations | 4 | 4 implemented within scope |
| Wizard spell task execution | 3 | 3 implemented within scope |
| Effect and delivery components | 7 | 6 implemented, 1 absent |
| Native spell integration | 5 | 5 implemented within native policies |

The explicit crosswalk is in `apps/coverage-ui/tracks.json`. Component statuses
and their existing evidence are read from the refreshed register. They are not
copied to whole-spell comparison, integration or replacement stages.

`AI.wizard-target-task` implements the eight original execution bodies for
41/42/74/78/79/83/84/85, with explicit lower services. Its recorded NS87 original
comparison covers 65,536 fixtures; it does not cover all effect semantics or
whole-controller candidate/scoring/preparation/lifetime. See [wizard research](wizard-ai.md).
`AI.wizard-projectile-task`, `CAST.projectile-update` and `CAST.sphere-motion`
cover separate Magic Sphere execution/update/motion scopes. See
[projectile delivery](projectile-delivery.md) and [Sphere motion](sphere-motion.md).

The nine corresponding `SP.*` entries now explicitly link delivered task/effect
implementation and tests, and are partially implemented. Cure additionally links
its recovered effect, resource/status cleanup and creation/animation components.
The seven actor-target spells beyond Cure have task code only; their effects
remain unimplemented. These partial records credit existing code without a new
scoped implementation completion claim. Whole-spell comparison, integration and
replacement remain unchanged. Existing component execution records and their
source hashes are preserved; new spell-row links do not retroactively bind them
to whole-spell claims.

Native Cure/World/battle and Sphere/battle integrations, and NS87's installed
wizard encounter, retain their own policies and bounded execution records.
NS87 admits Crocodile17, Cure41 and Sphere73; its native summon delivery/factory
policy is not the original summon lifecycle, so `SP.017` remains unimplemented.
Likewise, `CAST.cure-delivery` retains the missing complete original lifecycle
despite supplied native compositions. See [Cure delivery](cure-delivery.md),
[Cure effect](cure-effect.md) and [native Sphere battle](native-sphere-battle.md).

## Catalog denominator

All 104 IDs remain in the central register, detailed coverage, binary links and
the complete original spell-lifecycle/dispatch-inventory milestone. Of these,
29 configuration slots are explicitly named Unused and 75 have other names.
The latter include internal/unnamed IDs and are not asserted to be 75 distinct
playable spells. The 29 Unused-named slots are listed separately in the spell
panel and excluded only from the Core gameplay & spells implementation summary.
Their reachability is unresolved; configuration names do not prove dead code.
They remain requirements for the full inventory milestone.

At this review the 75 other catalog IDs contain nine partially implemented
entries and 66 unimplemented entries. No whole spell is marked implemented
within scope. The component groups above provide the missing progress context.
Counts are contracts, not effort estimates, and a shared task counts once as a
component even when it contributes to several partially implemented spells.

## Validation and remaining work

Dashboard regression checks exercise deduplication, catalog separation, missing
crosswalk rejection, delivered task/effect links and independent milestone gates.
They provide presentation/accounting validation only. No original artifacts are
consumed and no engine code, gameplay balance or historical evidence is changed.
Changed UI/whole-spell scopes retain historical evidence and pending current
validation. Original lifecycle composition, all remaining effects and strategic
tasks, installed-world equivalence and live simulation replacement remain open.
