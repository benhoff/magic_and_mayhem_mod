# NS21: bounded native melee encounter

This milestone owns a headless encounter over the existing native world,
navigation, movement and checkpoint interfaces. It adds target acquisition,
approach, attack windup/recovery, fixed damage and death cleanup. It does not
replace original gameplay or establish original combat equivalence.

## Recovered baseline and intentional policy

The [No-CD combat research](creature-ai-combat-spells.md) maps creature-target
behavior `0x0050afa0`, melee animation events `0x0050e7a0`, melee calculation
`0x0050c0d0`, defended damage `0x00514860`, signed health change `0x005076f0`
and lethal transitions `0x00514990`. The original damage is animation-event
driven and includes RNG, type matrices, defense, status, response and special
death branches. Those findings remain static/partial. NS21 performs no new
original execution and does not assign native timing or damage to those ranges.
`GP.combat` remains separate from `NP.combat-encounter`.

NS21 explicitly chooses these native policies:

- At most 32 same-profile creatures with one-cell XY footprints, in a bounded
  nonwrapping domain. Target eligibility requires a living registered creature
  on the same logical layer, a different owner and a configured Chebyshev
  acquisition range (1..64). Owner inequality is a fixture hostility rule,
  not recovered diplomacy or visibility.
- Acquire the nearest eligible target, retaining an eligible current target;
  equal-distance candidates preserve slot order. Approach a free adjacent cell
  minimizing logical distance, with Y/X tie order. Reservations include ongoing
  fine edges. The normal recovered navigation adapter decides route legality
  under its existing planning budget. Unreachable endpoints remain blocked;
  this slice has no exhaustive alternative-endpoint search or deadlock recovery.
  An admitted approach is retained while its endpoint still borders the target;
  transient fine-edge reservations alone do not reset the route.
- Movement advances before combat. Logical same-layer adjacency starts windup
  and cancels remaining movement at the committed cell. This can snap a fine
  mover to that cell, as native Stop already does. It is not original reach,
  facing, engagement or animation/action selection.
- A newly started attack waits its complete configured number of ordinary
  admitted ticks, then applies fixed positive damage clamped to current health.
  Recovery waits its configured ticks before another windup. Slot order resolves
  due hits; a lethal earlier hit prevents posthumous retaliation. No damage is
  applied merely because a target was acquired or an approach requested.
- Health zero marks the native entity cleaned and cancels movement and incoming
  attacks. The slot/generation remains allocated; cleaned bodies no longer block
  navigation. Corpse rendering, death animation, release timing and original
  cleanup/ownership repair remain excluded.
- Nonadmitted ticks leave all bytes unchanged. Alternate ticks retain the world
  session's sequence behavior without advancing combat. Search suppression
  postpones planning, not already admitted melee clocks.

The demonstration has two synthetic creatures: health 20 each, damage 6 versus
4, two-tick windup and three-tick recovery. These are test values, not game balance
changes. No executable, installed game data or original artifacts are modified.

## Ownership and persistence

`CombatSession` stages a copy of the movement session and typed combat records.
It publishes only after movement, combat, world and navigation validation all
succeed. Exceptions discard the entire staged encounter, including acquired
targets, movement and damage. Native services remain independent of Qt,
build-specific reconstruction and injected hooks. Only the sandbox's navigation
adapter depends on the existing reconstruction.

Combat subsystem v1 is encoded into the existing owned `State.systems` blob;
native checkpoint versions and their byte layouts are unchanged. The native
snapshot checksum covers the subsystem. Full-generation target identities,
health, per-creature parameters, action and remaining phase ticks survive save
and restore. Restore validates resources and all combat records before commit.
The movement-only session now refuses nonempty subsystem state when asked to
advance, preventing a combat checkpoint from silently becoming movement-only.
The movement scene can inspect compatible visual checkpoints but does not run
combat; the new console harness owns encounter advancement.

The [subsystem wire format](../formats/native-combat-subsystem.md) records its
layout and rejection rules. Original version-20 saves remain separate.

## Reproduction and validation boundary

Build `game` through the storage guard and run its CTests. The executable
`mnm-combat-sandbox` accepts `create MAP OUTPUT`, `create-fine MAP OUTPUT`,
`resume INPUT OUTPUT TICKS`, `trace INPUT TICKS` and `inspect INPUT`.
Create starts at tick zero; ticks are bounded to 0..4096. Outputs refuse overwrite.

```bash
python3 tools/create-movement-fixture.py working/combat-fixture.frozen
working/build/native-combat-ns21/mnm-combat-sandbox create \
  working/combat-fixture.frozen working/combat-start.mnms
working/build/native-combat-ns21/mnm-combat-sandbox trace working/combat-start.mnms 40
working/build/native-combat-ns21/mnm-combat-sandbox resume \
  working/combat-start.mnms working/combat-half.mnms 7
working/build/native-combat-ns21/mnm-combat-sandbox resume \
  working/combat-half.mnms working/combat-finish.mnms 33
```

Use new output paths and the main checkout's retention guard for retained runs.
These commands consume synthetic data only. Manifest checks are required if
future experiments instead consume installed/original inputs.

Validation includes independent subsystem byte construction and event-derived
damage/timing checks; complete encounter and intra-cell motion checkpoints in
fresh processes; hostility/layer/range/tie/blocked-route cases; lethal order and
retained dead identities; command/actor limits, malformed metadata, changed or
missing resources and whole-encounter rollback. Accepted execution evidence is
registered separately after these checks run. This document's scope is headless
native integration, not a native window combat demonstration or live replacement.

## Accepted execution and retained limitations

The normal game build passes 38 CTests; the ASan/UBSan build passes four focused
combat/world/movement CTests with leak detection enabled. Final scoped normal and
sanitized runs each execute the production combat contract checks and 32 exact
fresh-process continuations (25 coarse and seven fine-motion), plus 15 CLI/wire/
resource refusals. The coarse 40-tick encounter produces seven hits, one death
and final health 8/0. The slow fine-motion encounter also completes within its
512-tick bound. All five action phases occur and restart exactly; source and
binary hashes remain stable during each declared execution.

The tracked normal/sanitized records are `native-combat-normal-20261012.json`
and `native-combat-sanitized-20261012.json`. Full traces/checkpoints are retained
under `working/tests/native-combat/run-reviewed-normal` and
`working/tests/native-combat/run-reviewed-sanitized`; the build/CTest logs and
command list are under `working/tests/native-combat/final-validation-logs`.

The earlier [fine-motion failure](native-combat-fine-negative-20261012.json)
retains 512 ticks with unchanged logical positions and 682 approach requests.
Route re-selection against temporary fine-edge reservations repeatedly reset
progress. Retaining an admitted target-adjacent endpoint fixes this tested
branch; general reservation deadlocks remain open. The initial successful source
declarations omitted one transitive header, `assets/path_resolver.hpp`; the
[provenance limitation](native-combat-provenance-gap-20261012.json) preserves
those reports, and compiler-complete declarations precede the accepted reruns.
The final contract test additionally checks occupancy at the corpse's actual XYZ.
Earlier reports are unchanged and are not used to promote current statuses.

`NP.combat-encounter` supports scoped implementation and headless integration.
Original combat comparison and live replacement remain none. Existing shared
movement evidence may be historical/source-stale after the movement-only guard
change; these native encounter results do not refresh or establish original
AI/scheduling/damage/cleanup equivalence. No installed/original artifacts were
consumed by this synthetic milestone.
