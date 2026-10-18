# Native wizard AI

NS87 adds a bounded native admission after the ordinary NS86 skirmish. The end-to-end path
covers wizard decisions, spell/target selection, mana admission, cast delivery,
owned effects and portable continuation. Independent component recovery and native
composition have separate evidence; completion of one component does not establish
full original wizard behavior or full campaign integration.

All original addresses refer to No-CD Chaos.exe SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Preferred-image addresses identify build-specific routines, never native pointers.

## Discovery

[Task-family discovery](wizard-task-discovery-ns87-20261017.json) retains
constructor kind IDs 14–59, their seven original virtual-table slots and literal
spell arguments at availability/readiness call sites. Kind 21 references Cure
(41); kind 17 references Gooey Blob (33); other spells have individual evaluators.
Kinds 14, 15, 37, 58 and 59 have no literal at those two inspected call sites;
that is an unresolved mapping, not evidence that they cannot cast. Kind 15's
retained body enumerates summoned-creature choices. Static exports under
`/var/tmp/magic-and-mayhem-ns87-lab` include read-only pseudocode and bounded
assembly, with original manifest verification before and after. New Ghidra bodies
are discovery candidates; the central binary inventory is not silently broadened.

## Shared spell readiness

`51e530` first compares configured cost with maximum mana as unsigned whole units.
It multiplies cost by 256 and the requested count as DWORD arithmetic, compares
current fixed-point mana as signed, and scans all 42 inventory slots when that
initial balance is insufficient. Each available kind-19 entry contributes its
magnitude shifted by eight; the separate item count is not multiplied here.
The actor and inventory are unchanged by this query.

A nonpositive signed deficit returns zero. Otherwise, kinds 0/24/25/26 can use a
positive configured production rate. Active, eligible, producing actors of the
same owner each add rate shifted by eight. Positive accumulated income yields
the original signed ceiling division multiplied by the configured period.
Other cases return `INT_MAX`. Original DWORD wrapping and signed interpretation
are preserved. Producer predicate `412aa0`, resource generation/consumption and
configuration provenance remain independent contracts.

`wizardSpellAvailability` separately projects `40a950`'s present first wizard and
minimum across observed peers. An observed spell with an infinite wait remains
distinct from an unknown spell. Whole peer traversal and `51e6c0` are pending.
The spell-control comparison executes whole `51e530`; its native availability
assertions do not claim execution of whole `40a950`.

## Original composition boundaries

Original wizard source registration and queue admission, complete per-spell
candidate/scoring/lifetime/replacement control, survival and full repertoire
remain outstanding. The native decision-to-effect admission below composes
selected independently recovered contracts with explicit native policies; it
does not close the original whole-controller gap. Commander features, veterancy
and mana balance changes remain separate.

## Actor-target task execution

`wizard_target_task.*` projects eight complete bodies for spell IDs
41/42/74/78/79/83/84/85. Cure retires when its target is dead or at maximum health;
other task families use their respective target-status admission. Issued tasks
retain their continuing flag without issuing again. A zero mana wait precedes the
actual mana check and inventory-use branch. Spell 74 continues through positioning
after inventory use; the other seven return to await another visit. Spell 79 omits
the second target-health check that the other seven perform. Lower callbacks may
change health/mana, so these ordering differences are observable.

The positioning helper and cast delivery remain explicit services. The original
bodies initialize descriptor words 0 and 7–11; source/target fine-coordinate
production belongs to cast admission. Incidental uninitialized descriptor words
1–6 are excluded from task-body comparison. The source/target IDs, mode 1,
attribution sentinel and final control word are compared with ordered lower calls.
The prospectively bound NS87 run matches65,536 original/native fixtures and379,624
checks under sanitizers. Evidence is scoped to these task bodies and supplied
lower services.

## Native decision-to-effect admission (NS87)

`native-encounter run-wizards INSTALL 0 0 32 32 SEED TICKS NEW.snapshot`
activates an opt-in, headless, eight-slot mixed encounter. Slots0/3 are wizards,
1/2/4/5 are initial Crocodiles, and6/7 are one-time summon destinations. The
immutable profile uses installed `Greek41.wzd` (Gaea:700 health,145 mana),
`Greek Male.ani`, encrypted creature/HTH/spell/effect-animation definitions,
Crocodile ANI and the Celtic Plains MAP/TTD crop. Both factions receive authored
mid-game knowledge of Crocodile17, Cure41 and Magic Sphere73. Cure is an explicit
fixture input; these three bits are **not** a reconstruction of Gaea's starting
WZD repertoire. Original spell costs remain intact: Crocodile costs51, which
exceeds Cornelius's50 maximum and was the reason to choose Gaea for this fixture.

The shared recovered count selector visits four native families (fallback, Cure,
Sphere, summon), with four candidate visits per controller/frame. It checkpoints
partial selection, cursor, best candidate and strategic RNG. Utility values,
healing thresholds, current-position target choice, task admission/cadence and
positioning are native policies. Cure prefers injured allies, with priority for
an endangered wizard; Sphere selects enemies with a physically clear predicted
shot; summoning fills an unused same-owner Crocodile slot, up to three living
creatures. None of those utility scores asserts recovery of the original
per-spell inference/evaluator pipeline. The existing ordinary owner source
planner is excluded from this mode; ordinary creature autonomy still runs.

Actual cast admission debits fixed-point mana and records observed/learned
spell state. Source and target centers use the owned size/height definition,
with actual installed directional `LRCOffsets` and current motion direction.
The initial development profile omitted centers, causing shots to hit terrain
at the caster's feet; a second adapter correction translates native one-based
occupancy identities to zero-based effect collision ordinals. Both corrections
are application bindings, independently of the original task/effect comparisons.
Prediction and delivery now share normalized launch setup, the actual owned
terrain catalog, trajectory stepping and creature footprints. Prediction admits
only a shot whose first current-position collision is the intended enemy; moving
target prediction, original `44a500` scoring and original stand selection remain
pending. A target can move after selection; cast-time prediction rechecks before
spending mana.

Cure executes the recovered41 task, cast admission, selected type11-to77
transition and the actual installed Cure animation. Animation completion applies
recovered healing/status cleanup to the owned health/maintenance projections.
Magic Sphere executes its recovered73 task, type3 admission, trajectory/motion
initialization, terrain/creature collision and defended actor damage, including
recent-attacker/time updates. Effects use16 owned slots and generation counters.
The native host releases bounced/resolved projectiles instead of owning their
original impact/bounce visuals, and supplies logical audio without a sound backend.
Summoning uses a native legal-ground search and first-use identity activation;
original `48f490`/`48f690` allocation/factory and full summoned-creature creation
are excluded. Corpse slots are retained and never recycled by this admission.

Producer eligibility is an explicit controller input, false for both ordinary
mixed journeys. The authored care test enables same-owner production using
file defaults5 units per5 ticks (pinned `5e1908`/`5e1904`); this is not recovery
of original power-point ownership or `412aa0`. Native inventory consumption uses
one available kind19 item, decrements its count and credits its configured
magnitude; whole original `51d570` inventory use remains pending. These are
native resource policies, separate from the independently compared `51e530`
readiness arithmetic. No spell cost or balance configuration was changed.

Wizard health, ordinary melee/reach/stun and reservation cleanup run through the
owned encounter. Wizard defeat retires its native tasks and releases occupancy
through the fixed-pool lifetime policy. Campaign loss, scripts, transformations,
owner transfer, special wizard death effects and progression remain outstanding.
This is native decision-to-effect integration for three admitted spells, **not
completion of full original wizard AI or its entire repertoire**.

## Portable state and failure boundary

[Wizard continuation format](../formats/native-wizard-continuation.md) records
CMD1v20 and the immutable `MNMWIZ01` profile. Active selectors/candidates/tasks,
issued latches, start/expiry/serials, learned/observed state, resources, inventories,
producer input, effects, ANI interpreter, trajectory/collision caches and strategic
RNG are portable values. No Qt/Wine callback, original address or host pointer is
serialized. Old CMD1v1–19 hosts keep their layouts; writing wizard state as v19 or
older refuses. Static wizard/configuration/actor inputs are checked against the
selected immutable profile on restore. Bounds and task conservation are checked
before publication; active ANI state is also restored against its exact sequence
before the next wizard update. Invalid sequence-specific interpreter state refuses
the staged step. An optional operation-local verifier runs after wizard work;
exceptions discard the complete staged frame, including mana/effects/commands.

First-use activation also resets the summoned creature's execution wander home
to its spawn point, with an explicit verifier assertion.

The bounded native test runs two2048-frame mixed seeds, an authored512-frame
injured-ally/inventory/producer fixture and a512-frame wizard projectile-defeat
fixture. Care/duel modifications are explicit test inputs; public cold-process
continuation is checked against the unmodified mixed profiles. Independent
original task/effect tests supply their lower services and do not claim a whole
original mixed encounter. Development failures and superseded binaries remain
under `/var/tmp/magic-and-mayhem-ns87-lab`.

## Reproduction and remaining original work

Run `tools/test-wizard-spell-control.py`, `test-wizard-target-task.py`,
`test-wizard-projectile-task.py`, `test-wizard-ground-profile.py --asset ANI`,
`test-cure-effect.py` and `test-projectile.py` with new output paths and prospective
`--claims` declarations for source-bound original/native/sanitizer comparisons.
`tools/test-wizard-ai-integration.py --install INSTALL --build-root NEW_BUILD
--output NEW_OUTPUT --claims CLAIMS` performs clean native/sanitized builds,
legacy/core regressions and cold/public CLI restores. Use the main checkout's
retention guard before producing storage. These comparisons bracket immutable
original use with manifest verification; no original file is patched.

`tools/export-wizard-ai.py` reproduces204 pinned read-only disassembly ranges and
optional Ghidra pseudocode. `export-wizard-ai-inventory.py` seeds the nine compared
wizard task entries; `review-wizard-ai-inventory.py` admits only their exact
reviewed body boundaries/hashes, retaining the prior6692 functions, flows and
unknowns. The resulting6701-function inventory does not infer semantic coverage
for every function. Static constructor/vtable mapping of46 wizard task kinds and
the confirmed execution dispatch slots remain separate from native implementation.

`AI.wizard-controller` remains partial understanding with no whole-controller
implementation/comparison/integration promotion. Remaining work includes original
source/brain admission and scheduling, availability peer traversal, every original
per-spell prepare/next/save/scoring/lifetime/replacement/survival branch, other
spells and secondary/status/area effects, original resource/item/factory ownership,
script/campaign consequences and a captured original candidate-to-cast pipeline.
Commander orders, veterancy and mana economy changes remain outside this work.


## Additional collision boundary discovered during NS87

A broader first collision experiment increased per-visit steps from0–4 to0–16
for moving types3/13/22/24/36 with spell73. Fixture297's second visit returned4
in the original and3 in the native model. Read-only disassembly at488b19–488b53
shows a post-motion normal spell allowance of metadata distance+32, compared
with the signed trajectory counter divided by2 (truncation toward zero).
Types3 and71 explicitly continue past that condition; other moving types return4.
This newly exposed nonprojectile branch is `FX.motion-distance-stop`, with no
implementation/comparison promotion. The failed experiment and its exact source,
claims and executed binary remain at `collision-accepted-v1/failed-source` under
the NS87 raw evidence root. Subsequent comparison restricts the moving record
to actual type3; it does not erase or validate the excluded branch. Wider catalogs,
nonzero distance metadata and type71 remain separate original-comparison gaps.


The initial native integration build completed clean native64 and ASan/UBSan/LSan
compilation and native/core/ordinary runs, but was stopped before full sanitizer
acceptance to correct the summoned wander-home input. That v1 run is explicitly
superseded, not accepted evidence. The final v3 validation uses the same configured
build trees incrementally, rebuilding changed source and test inputs; its
`clean_rebuilds` field is false and `incremental_rebuilds` true. All declared and
compiler dependency hashes must remain stable across that execution, and exact
executed binaries and portable artifacts are retained separately. A clean
bootstrap is not substituted for current-source integration evidence.


The expanded motion bounds are restricted to actual type3. Other previously
admitted moving types retain their old iteration<=8/counter<=64 limits, and the
new spell73 creature-collision admission refuses non-type3 records. This preserves
prior bounds while `FX.motion-distance-stop` remains unresolved. V2's native
summon-fix journey passed, but that integration run was superseded before final
acceptance to apply this restriction; v3 carries the current source bindings.
The initial import hashes in `wizard-effects-import-ns87-20261017.json` describe
the import event, before these subsequent local admission guards.


## Accepted current execution

`NS87.native-wizard-encounter` binds341 current compiler/build inputs and the
headless three-spell scenario. Native64 and ASan/UBSan/LSan agree on5120 frames,
426277 checks,116 sliced restores,27 refusals and4 complete-frame rollbacks;
63 casts,28 heals,27 actor impacts,4 summons, one mana-item use,204 production
visits,9 health-to-zero transitions and3 wizard defeats. All portable wizard
snapshots/traces match across builds. Seventeen cold-process restores and the
public `run-wizards`/`resume` path produce identical snapshots with `PATH` set
to a nonexistent directory during resume. Ordinary skirmish/encounter and both
sets of core command regressions pass. Installed/original inputs and declared/
compiler source hashes remain unchanged; exact executed binaries are retained.
The final run is explicitly incremental after the retained clean bootstrap.

The current type3 collision result is `NS87.projectile-collision-original-v3`;
the earlier passing source version is retained under its original evidence ID.
Historical records are not rehashed to match subsequent admission guards.
The independent component records promote only their stated original scopes;
headless integration promotes only `NP.native-wizard-encounter`. Full original
`AI.wizard-controller` remains partial/unimplemented/uncompared.
