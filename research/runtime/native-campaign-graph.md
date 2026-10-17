# Native campaign graph and Crossroads chapter

`NP.campaign-graph` is intentional native progression policy, separate from
original campaign scripts, rewards and balance. Source-bound headless execution is recorded below. All acceptance is headless; this
increment has no campaign-map window or presentation milestone.

## Bounded progression contract

A graph has two to sixteen missions in topological definition order. Mission zero
is the sole entry and the last mission is the finale. Each node declares
`requires_all` and `requires_any` prerequisites, an optional exclusive `choice`
group, and zero to four existing spell rewards. Every nonentry node must have
prerequisites. IDs and lists are bounded and unique. Alternative nodes in a choice
group require identical prerequisite lists; entry/finale cannot be alternatives.

The core enumerates the bounded completion-state space when validating a graph.
It rejects cycles/forward references, missing alternatives, unknown or duplicate
rewards, unreachable missions and legal choices that dead-end before the finale.
This checks progression reachability assuming mission victories; it does not
prove combat solvability, balance, placement or available actor capacity.

`availableMissions()` lists eligible idle missions. `selectMission(realm, region)`
changes only the idle selection, without awarding anything or advancing the
attempt counter. A selection can change until its mission starts. A successful
start commits an exclusive choice in the same transaction as battle creation;
a failed factory/initial-wave admission commits nothing. An active battle forbids
selection changes. Defeat/draw retain the selected mission and choice for retry;
there is no abandon-or-switch action after a loss in this increment.

A victory appends one mission index to the ordered completion history and marks
only that region completed/owned. The first eligible successor becomes the default
idle selection. Winning the finale completes the chapter even though the skipped
alternative region remains incomplete and owned by its initial owner.

Rewards belong to graph nodes. Graph mission plans must use the existing
version-2 objective format with empty `reward_spell`, or be absent for an ordinary
battle. Spell ownership is derived from the initial roster plus completed nodes
in recorded victory order; duplicates are suppressed. Selection, start and loss
do not grant rewards. Wrong spell order/content, attributes, experience or
inventory refuse restore. Locked casts and queued locked spells use the existing
campaign spell gate. Existing costs, damage, health and regeneration are unchanged.

The graph, region flags/owners, selected mission, active attempt and last result
are checked together before publication. Restore replays the ordered victories
through prerequisites and committed choices, binds every choice to a completed,
active or failed/retry attempt, verifies finale completion and derives the reward
roster. The resource adapter retains exact saved battle bytes. This validates
structured continuation, not authenticity against an attacker rewriting an entire
otherwise valid history and world. Loading an older save intentionally rewinds
its timeline; there is no global award database.

## Crossroads

`apps/skirmish/plains_campaign.json` authors one four-node native chapter:

| Mission | Prerequisites | Reward |
| --- | --- | --- |
| Opening | Entry | None |
| Last Stand | Opening; exclusive Celtic road | Cure |
| Watchpost | Opening; exclusive Celtic road | Magic Sphere |
| Arena finale | Either road | Chapter completion |

Opening clears a Plains guard and the rival. Last Stand reuses the two timed-wave
objective design. Watchpost holds for 48 admitted ordinary ticks, clears its guard
and defeats the rival. The finale uses the existing ordinary arena battle. Terrain,
wizard/Crocodile profiles, animation, spell effects and combat values are reused.
No creatures or combat systems persist across missions; earned spells do.

The compiled JSON binds mission IDs, maps, prerequisites, choices, rewards and
plans into content identity. Editing the definition requires rebuilding and makes
old saves of this new chapter incompatible. Other native campaign identities,
definitions and save versions remain separate and are not silently migrated.

```
mnm-map-campaign-cli ASSET_ROOT create-crossroads idle.mnmc
mnm-map-campaign-cli ASSET_ROOT start idle.mnmc opening.mnmc
mnm-map-campaign-cli ASSET_ROOT select fork.mnmc selected.mnmc celtic watchpost
mnm-map-campaign-cli ASSET_ROOT start selected.mnmc route.mnmc
```

Existing `cast`, `attack`, `resume` and `inspect` commands operate on the same
session. Inspection includes available mission IDs, ordered completions, committed
choices, Cure/Sphere availability and the current objective continuation.

## Automated acceptance

CAMPAIGN03.crossroads-headless prospectively binds 630 unchanged
source keys and the Crossroads headless scenario. Normal and ASan/UBSan/LSan
contracts agree: both full-health routes complete using production native orders,
with a 308-tick opening, 477-tick Last Stand, 308-tick Watchpost and
19-tick ordinary finales. Separate earned-spell fixtures verify Cure healing and
Magic Sphere damage. Asset-free contracts cover malformed/forged saves and graphs,
failed-start rollback, defeat/retry, exclusive choices, reward derivation and
terminal completion. All 114 headless CTests pass.

19 retained transition/effect checkpoints round-trip exactly and match direct
versus split fresh-process continuation. Production player resumes from both route
checkpoints reach the exact reference win bytes; CLI create/select/start matches
the installed contract. Seven old native save profiles covering versions 1–4
round-trip unchanged. Installed input hashes, compiler dependency closure and the
original 2,927-file manifest bookends pass. This result supplies headless native
integration only; historical shared-source fingerprints and their freshness
limitations remain unchanged. No window or original/live milestone is promoted.


The fast asset-free `campaign-graph-core-test` is registered with CTest. It uses
actual native skirmish timer fixtures for both routes, injected factory failure,
base defeat/retry, clone continuation, malformed wire data, forged choices/rewards,
selection locks and terminal completion. Installed acceptance in
`tests/campaign-graph-test.cpp` separately uses full health and production native
player orders through the common opening, both roads and both ordinary finales.
Separate reward fixtures spawn a valid actor and damage it only to exercise
actual earned Cure healing; Sphere delivers against an enemy actor. These fixtures
do not weaken the complete-chapter journeys.

`tests/test-campaign-graph.py` prepares normal/sanitized builds, matches the two
contracts, runs headless CTests, compares independent CLI create/select/start and
save continuations, resumes the production player from both route checkpoints,
and checks retained native versions 1–4. Prospective scope/source declarations,
compiler dependencies, installed hashes and original-manifest bookends bind the
result. Transition inputs and logs stay under `working/`; large continuation output
files and builds stay in a task-specific memory-backed directory. No scratch output
is deleted by the runner; its path and resulting hashes are recorded.

Remaining boundaries include window presentation, original campaign equivalence,
arbitrary scripting, abandoning a failed route, mission solvability/capacity
reclamation, new combat/balance features, crash/power-loss equivalence and live
replacement. Historical shared-source evidence keeps its hashes and freshness
limitations; this chapter cannot promote those original/UI contracts.
