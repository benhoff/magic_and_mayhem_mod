# Native authored skirmish (NS22)

This milestone owns a bounded authored 8×8 arena, two stationary wizards,
one summoned Sentinel profile and Arc Bolt. All combat, mana, AI and spell
rules are intentional native policies. Original combat/casting/mana behavior,
installed worlds, campaign assets, veterancy and live replacement are excluded.
The original cast admission at No-CD `0x0057b710` remains separately documented
in [creature-ai-combat-spells.md](creature-ai-combat-spells.md); this fixture does
not reproduce it. No original artifact is consumed by the arena or these tests.

## Ownership and phases

`SkirmishSession` owns the simulation independently of Qt and legacy hooks.
The app constructs synthetic frozen navigation bytes and resolves their identity;
its adapter remains the only dependency on recovered navigation. The semantic
controller owns playback, selection and checkpoint actions. Widgets express
cell selection, orders and save/load without engine addresses or wire offsets.
Rendering reads the committed snapshot; a projectile's visual arc derives from
remaining flight ticks, with no independent animation clock.

An ordinary admitted tick regenerates mana, admits pending player orders, makes
a bounded rival decision, advances movement/melee, completes existing cast
windups, advances existing projectiles and commits the outcome. Newly started
casts keep two full windup ticks; newly launched projectiles keep four full
flight ticks. Nonadmitted ticks do nothing. Alternate ticks retain simulation
phases and pending orders while advancing only the existing sequence policy.
Publication is transactional: navigation or validation failure discards the
whole staged tick. Save/load validates a complete candidate before replacement.

Creature Move suppresses attacks until arrival, then resumes automatic targeting.
Stop holds until another order. Attack pursues only the requested hostile target
and holds after losing it. Full-generation identities remain retained on death;
dead bodies leave occupancy. Wizards stay fixed at `(1,6,1)` and `(6,1,1)`.

## Authored rules

| Rule | Value |
| --- | --- |
| Initial/maximum mana | 20 |
| Regeneration | 1 every 3 ordinary ticks, capped |
| Summon / Arc Bolt cost | 8 / 4 |
| Cast windup / bolt flight | 2 / 4 ticks |
| Wizard / Sentinel health | 48 / 24 |
| Sentinel melee | 6 damage, 2 windup, 3 recovery |
| Arc Bolt damage | 10, clamped to remaining health |
| Rival decision wait | 8 intervening ticks |
| Living Sentinels per owner | 6 |
| Retained actors / projectiles / pending orders | 32 / 32 / 64 |

Summoning admits an unoccupied playable cell within Chebyshev distance two of
the wizard; completion rechecks occupancy and capacity. Failed completion refunds
the summon cost, capped at maximum mana. Busy, insufficient-mana, blocked and
capacity admissions do not debit mana. Bolt locks its target identity and aim
cell at admission. It fizzles if that target is dead at launch, and misses if
the target moves or dies before impact. It ignores intervening terrain. The
rival summons one living Sentinel when possible and otherwise bolts the player
wizard. Dead retained actors eventually exhaust the actor pool; slots are not
recycled in this fixture. Airborne bolts can resolve after their source dies;
both wizards dying in the same committed tick is a draw. Terminal cleanup holds
all creatures, clears casting/projectiles/orders and freezes later ticks.

The rules fingerprint is a schema/fixture identity, not an original balance
fingerprint. Changes require an explicit format identity review. The format is
[native-skirmish-subsystem.md](../formats/native-skirmish-subsystem.md).

## Validation boundary

The contract and independent-process runner exercise pending command persistence,
cast and flight timing, mana debit/cap/refusals, summon completion collisions,
move/stop/attack, fixed-aim misses, projectile bounds, simultaneous terminal damage,
resource/semantic restore rollback, wire corruption and complete match continuation.
The window runner uses the production entry point with test-only instrumentation:
actual mouse events, semantic buttons, real file dialogs, timer admission,
paused time, rejected load and fresh-window/headless continuation. Evidence records
will be registered after the final declared-source executions pass.

These fixture results do not promote `GP.combat`, `GP.commander` or `GP.mana`,
and do not remove Wine from the original/hybrid campaign. The next behavior
milestones remain original combat/spell validation, richer entity profiles and
installed map/asset integration. User-facing reproduction is in the
[application README](../../apps/skirmish/README.md).

## Executed evidence — 2026-10-12 UTC

The declared-source Debug run passes the production contract and independent
wire/cast/flight/mana oracle, 32 exact fresh-process victory continuations and
16 malformed/CLI refusals. The ordinary fixture wins at tick 32 and the idle
fixture loses at tick 29. The real window journey wins at tick 31 after summon,
move, stop, explicit attack and bolts; its complete tick/event trace and final
snapshot match headless replay of its accepted inputs. Five fresh-window saves
at pending order, summon windup, actual movement, airborne bolt and melee
windup continue to the same snapshot and canonical canvas. The unattended real
timer window loses at tick 29. A real load dialog also restores the casting
checkpoint; a rules-corrupt load preserves the active match, and a 250ms paused
interval leaves it unchanged.

- [Debug simulation result](native-skirmish-normal-20261012.json)
- [Window integration result](native-skirmish-window-20261012.json)
- [Automatic melee regression](native-combat-regression-normal-20261012.json)

Historical NS21 reports are preserved. The v2 controls change their source
fingerprints; the fresh v1 regression keeps the existing automatic-only scope.
Forty normal CTests pass; the window run adds the production UI journey separately.
Full runs, command/build logs and captures are retained under
`working/tests/native-skirmish/`. A failed earlier harness checkpoint assumed
windup at tick 12; observed recovery caused that check to fail. The final harness
saves at tick 13 and checks the actual phase before continuing. Earlier raw
negative/development runs remain retained and are not registered as success.

These are bounded fixture checks, not exhaustive malformed-state fuzzing,
interactive latency/load testing or original gameplay comparisons. Larger
creature crowds, every simultaneous order/cast/death combination, projectiles
against arbitrary terrain, sound and original animation/asset presentation remain
separate work. Unit cleanup retains identities, so extended fights can exhaust
32 actor slots. ASan/UBSan with leak detection passes six focused CTests and the same 32
headless continuations/16 refusals. The v1 automatic regression also passes
32 coarse/fine continuations and15 refusals under sanitizers. Qt/window code
was exercised in the normal build; it is outside this sanitizer run.
[Skirmish sanitizer result](native-skirmish-sanitized-20261012.json) and
[automatic melee sanitizer regression](native-combat-regression-sanitized-20261012.json).

Committed history `82f9bff..122f5c8` has 61 exact file transitions and 536 affected
behavior transitions with matching retained receipts and zero unresolved gaps:
[history review](coverage/native-skirmish-history-20261012.json). This is accounting,
not a retrospective gate pass or new engine validation.

## NS23 health follow-up

New arena sessions use the [verified ordinary numerical health projection](ordinary-health.md).
Health backend identity is checkpointed independently of the arena rules blob;
older saves retain their authored backend. Defense/status profile loading,
original wizard death and callback/lifecycle effects remain excluded. Native
healing is an engine API; spell/AI/mana/profile balance is unchanged. New evidence
IDs supersede current-source validation only within their declared scopes; all
NS22/NS21 records and hashes remain preserved.
