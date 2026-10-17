# Native typed mission objectives

This is intentional native mission policy, not recovered original mission
scripting. Behavior `NP.mission-objectives` extends the finite staged mission
without changing old definitions, combat values, driver formats or campaign IDs.
MISSION02.objectives-headless prospectively binds620 unchanged
source keys and the headless objective scenario. Normal and ASan/UBSan/LSan
contracts agree: full-health Last Stand completion in477 ticks,
11 event/timer boundaries,74 timer continuations,
blocked/partial wave refusal and reuse-generation retry, forged/malformed/stale
continuation refusal, protection before prerequisites and after completion,
simultaneous draw, no-reward policy, defeat/retry and Cure/finale transition.
12 saved boundary/finale states round-trip exactly and match direct versus
split fresh CLI continuation. A complete production-order journey also matches
across a process split. All113 headless CTests and the retained staged native
contract pass; five old save profiles round-trip unchanged. Installed hashes,
compiler dependency closure and original2927-file manifest bookends pass.
Earlier failed development strategies remain under working/ and supply no
positive validation. Only the new native policy gains headless integration;
older shared-source evidence retains historical hashes and freshness limitations.


## Bounded authoring contract

A version-2 plan has at most 16 ordered objectives and eight hostile groups, each
with one to eight existing creature spawn locations. Dependencies name earlier
objectives. Groups may depend on any objective; a combined objective/group graph
check rejects cycles, including a wave waiting for its own elimination. Every
group must be referenced by an elimination objective. IDs are unique across the
plan. Text, JSON bytes, coordinates, timers and dependency lists have explicit
limits. Unknown fields/types/targets and unused typed fields are refused.

Three objective kinds are available:

- `eliminate`: wait for an admitted group's retained full-generation fighters,
  or the named player/rival wizard, to have zero health.
- `survive`: count 1–4096 ordinary admitted battle ticks after prerequisites were
  already complete at the start of the tick.
- `protect`: require a named wizard to remain alive for the entire attempt,
  including before its prerequisites and after this objective completes.
  Its prerequisites control completion only, never disable the protection guard.

Ordinary admitted tick order is: try ready groups, advance the existing battle,
then evaluate objectives against the previous objective-completion snapshot.
There is no same-tick dependency cascade. A group is published atomically by the
existing battle spawn transaction, and is never retried after admission. A
blocked group remains pending; other battle work and independent timers continue.
The existing battle capacity is 32 total unit roles, including retained mission
corpses. A definition passing graph/type validation is not a proof of solvability:
placement, capacity, incompatible goals or deliberate player obstruction can keep
an objective pending. Authors must budget their groups within the existing battle
capacity; this increment does not add corpse reclamation or a mission solvability
solver.

Alternate ticks run the existing battle path but neither admit groups nor advance
objective timers/completions. Unadmitted ticks change nothing.

Base combat defeat/draw takes precedence, then any protection failure, then all
objectives complete yields victory. Generic objective success can end a survival
mission while the rival remains alive. The battle's wizard-death victory gate
stays disabled; the campaign owns settlement. A result clears the whole battle,
advances the existing attempt/transition policy and grants the optional supported
Cure reward once per adopted timeline. Restoring an older save intentionally
rewinds that timeline; there is no global award database.

## Last Stand

`apps/skirmish/plains_survival.json` authors a new campaign using the same Plains
crop, installed wizard/Crocodile profiles, costs, health and damage as the prior
staged mission. Survive 24 ticks, eliminate the first wave, survive another 24,
eliminate the second wave, defeat the rival, and keep the player wizard alive.
Cure unlocks for the existing arena finale. Raw compiled JSON bytes join the
content revision; editing the plan requires rebuilding and changes save identity.

The original three-stage campaign remains separate and byte compatible. The new
campaign is exposed through the CLI:

```
mnm-map-campaign-cli ASSET_ROOT create-objectives idle.mnmc
mnm-map-campaign-cli ASSET_ROOT start idle.mnmc started.mnmc
mnm-map-campaign-cli ASSET_ROOT inspect started.mnmc
```

`resume INPUT OUTPUT TICKS` and the existing summon, attack and spell commands
work with the saved graph. Inspection prints stable objective IDs, completion,
timers and admitted group sizes. This increment has no window controls or UI
acceptance milestone. Use the CLI for the new campaign.

## Continuation boundary

The native campaign codec adds version 4 only for active version-2 objectives;
old idle/settled/v1-stage campaign encodings remain unchanged. Resource-bound
restore validates the exact plan shape, gates, timers, dependencies, groups,
roles, ownership and handle generations before publication. All hostile creatures
must belong to an admitted group. Completed objectives must satisfy their typed
conditions. Already settled graphs and failed protection cannot be retained as
active campaigns. This validates structured continuation, not tamper resistance
against an attacker rewriting a complete valid world and campaign together.

Automated acceptance lives in `tests/mission-objectives-test.cpp` and
`tests/test-mission-objectives.py`. It separates full-health production-order
completion from reduced-health boundary/failure fixtures. Fresh execution records
bind prospective source/scope claims and installed hashes. Original equivalence,
arbitrary scripts/branching campaigns, friendly creature protection, new species,
physical rendering/audio, usability/balance and live replacement remain outside
this work.
