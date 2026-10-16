# Cure in the native battle and campaign

This native policy combines the compared Cure contracts with the existing authored
arena, rather than claiming original campaign behavior. `SkirmishEffects` is the
application boundary; the native game core contains no reconstruction dependency.
CombatState is the single health owner and Wizard the single mana owner. The
Cure runtime hydrates an ephemeral projection for admission/delivery and saves
only effect/ANI/actor-status/learning bookkeeping. Health changes are applied to
the live Fighter through the selected ordinary-health backend and checked against
the Cure result. Moving creatures, melee, projectiles and healing share one outer
transaction and one World tick.

A queued Cure admits against a living friendly Sentinel and charges3 resource
units. It creates a generation-bound World effect and the recovered type11 spatial
record; the effect phase transitions/binds77. Subsequent phases tick the actual
owned ANI player, follow current logical actor positions and complete on raw1.
Positive magnitude8 caps at combat's maximum. The existing native arena uses an
authored animation fixture; installed recipe74 is exercised separately through the
same runtime. These constants/timing are fixtures, not recovered spell balance.

Normal scheduling is mana regeneration and orders, native movement/melee, Arc Bolt
impacts, Cure, then outcome settlement. Alternate admission retains effect ticks
without ordinary movement/mana/AI advancement; unadmitted ticks freeze everything.
Dead targets release, removal cancels associated effects and terminal outcomes
cancel all remaining effects without additional healing or refunds. Newly summoned
and reused creature slots enroll using full generations. Failed runtime calls roll
back the whole candidate; ordinary admission refusals consume no Cure mana/effect.

The production window exposes Cure against a friendly Sentinel and shows an owned
vector halo at its logical position. Both CLI and window submit the same order.
Skirmish/campaign checkpoints preserve pending commands and embedded selected ANI;
load is paused, with no asset dependency. New campaign content binds the Cure
profile; legacy saves select their original exact profile. Replacing missing or
unknown runtimes, malformed inner payloads and mismatched effect tags refuse.

See [the component format](../formats/native-cure-battle.md) and
[the arena](../../apps/skirmish/README.md). Source-bound execution covers moving
healing, cap/death/pool refusals,40 removal/reuse cycles, checkpoint continuation,
legacy campaign profiles, installed ANI playback, actual Qt controls/dialogs and
fresh-process campaign resume. Original full dispatcher/cadence/allocator, special
targets/negative healing, physical audio, original SPR pixels and live original
campaign replacement remain separate milestones. Creature AI remains the authored
arena policy, not the complete recovered creature-rules laboratory.
