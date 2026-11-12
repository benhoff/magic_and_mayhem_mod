# Target-candidate and Space-toggle diagnostics — 2026-11-03

This native diagnostic policy extends the
[bounded order observer](campaign-order-admission-20261103.md). It reads the
existing original candidate pointer at `0x656600` against the active creature
pool, separately from primary selection, spell-bar receiver and final command
receiver. It does not recover the complete picking function or object lifetimes.

## Wire and admission boundaries

`MNMCOA01` version 2 retains the 16-byte header and 256-byte row. Flag 64 means
the current pool snapshot validated and the candidate was either null (word 63
zero) or an active, readable, slot-checked creature (word 63 is slot plus one).
Unreadable, foreign or inactive candidates have flag 64 clear and word 63 zero.
This distinguishes known null from unavailable identity. Version 1 remains
readable with its original reserved field and flag restrictions. Version 3
uses 272-byte rows: word 64 counts actual returns from the already-forwarded
original World callback; words 65–67 are reserved zero. `MNMCAI01` version 3
uses 144-byte rows, required flag 128 and the same ordinal in word 32, with
words 33–35 reserved zero. Readers retain the older wire contracts and reject
invalid flags, reserved fields and backwards ordinals (allowing uint32 wrap).
Sampling adds no original ticks and writes no original simulation state.

`candidate_after_hover` accepts only a new latest World sample with a validated
matching numeric candidate, at or after the independent admitted hover's World
return ordinal. A later callback supersedes the candidate until a new World sample.
The physical helper still obtains hover readiness through the complete-prefix
guard, including pending/nested callbacks. A candidate is not an attack order,
and neither candidate nor buffer submission proves delivery or engagement.
V3 hover admission requires a strictly later actual World return than the
matching original no-button motion return. Publication alone, a held ordinal,
mixed ordinal/clock rows and outstanding callbacks refuse admission. Legacy
rows retain the older simulation-clock proxy. This is a native test policy,
not a reconstruction of all picking or pause semantics.

## Bounded scenario

`run-rendering-progression.py --checkpoint restore --checkpoint-probe
paused-orders` requires the exact verified original checkpoint and the existing
private input diagnostic. It first repeats the X-selection, opcode 5 submission
and subsequent movement assertions. It then tests three centred wizard body
points through real pointer motions, checking candidate identity independently
of the selected Zombie.

The helper sends one ordinary Space key toggle and moves the pointer. It
requires original input/World-return admission and a separately validated Zombie
candidate while the sampled simulation clock holds across later World returns.
The second Space toggle must restore clock progress and hover readiness. The
helper attempts release on failure as well. `paused-cast` instead arms Fireball
through observed spell-bar changes and sends one ordinary right click on
validated empty ground at least four cells from the Wizard and Zombie. It
requires an original accepted spell row and mana debit; buffer submission and
post-release state are recorded independently.

`--checkpoint-probe combat` resumes only the verified CF52 checkpoint, follows
the original bridge route and stops after an original Zombie-to-Camulos opcode 7
submission. It skips the opening route and cannot prove nominal progression.
At an observed close firing position it uses one bounded Space interval, direct
candidate-checked left selection, a sole-Zombie primary bitmap, candidate-checked
right press and exact actor/target command bytes. Submission remains separate
from subsequent gameplay combat-target engagement, damage and delivery. One
real 16-pixel motion away and back may elicit a callback; it never substitutes
for the final-point original callback and later World observation.

The diagnostic keeps the default lightweight capture policy. Original Save/Load
witnesses, exact unchanged save bytes, normal Quit, stopped Wine, source closure
and resource retirement remain independent assertions. Failed observations are
retained and do not become nominal progression or rendering acceptance.

## Confirmed observations and limits

All report paths below are under
`rendering-progression-results-20261101/`; payloads are immutable SQLite reports.
The original save is 356531 bytes, SHA-256
`9ec512d4c05a75c20e7cc7befec8398e112c2ca700917f34e93f5501099ee7c1`.

* `target-pause-probe-01.json`: the simulation clock held at 2622 while the
  candidate changed from Wizard (slot plus one 1) to Zombie (4). The older
  clock-based gate refused despite continuing original World samples.
* `target-pause-probe-02.json`: with V3, the clock held at 2625 while World-return
  ordinals advanced 84, 87, 91 and 95. Original motion return 86 preceded World
  87 and the independently matched Zombie candidate. Readiness recovered after
  the second Space toggle. This supports the narrower return-based policy.
* `target-pause-cast-01.json`: original Fireball 71 was accepted while the clock
  held at 2652, before release. The Wizard's raw mana changed 12800 to 11264
  (50 to 44); HP remained 184. A separate opcode 4, 33-byte buffer append was
  observed. These observations prove one accepted cast and six-mana debit,
  not damage, target lifetime, mana regeneration or complete pause equivalence.
* `target-support-attack-01.json` failed: X while the pointer still described
  the HUD (cursor 16) did not produce sole-Zombie selection during the bounded
  pause. The cause is unresolved; this does not establish that all paused
  keyboard selection fails.
* `target-support-attack-02.json` failed before any selection button: the
  Zombie candidate matched, but the non-cast guard correctly refused mode 2.
  Clicking the Fireball icon again had left spell mode active. The driver now
  selects directly before arming a spell; it does not treat an icon click as
  cancellation acknowledgement. Both failures completed normal Quit and
  retained unchanged negative evidence.
* `target-support-attack-03.json` passed with direct selection: left click at
  `(96,266)` matched Zombie slot 3 and produced sole primary selection `[3]`;
  right click at `(480,234)` matched Camulos slot 4 and appended
  `00000000070000000e0000000304` (opcode 7, length 14, actor 3, target 4).
  The helper stopped before the volley and released Space; original Save/Load,
  normal Quit, unchanged inputs, stable sources and stopped Wine all passed.
  This is buffer submission, with `command_delivery_proved=false`.

The successful V3 probes have exact source snapshots in
`campaign-target-admission-results-20261103/v3-source-snapshot-01.json`.
Later driver versions do not refresh those original hashes. The isolated
`return-regression-01.json` distinguishes held-clock/advancing-return,
held-return/advancing-clock, wrap and pending-entry cases; its logical fixtures
are separate from original execution. Earlier and intermediate source versions
are retained independently.

At the checkpoint-only stage, delivery, engagement, live route suspension and
fresh nominal progression/rendering remained pending. The later fresh results
below close one configured progression/rendering branch; the twelve-case matrix
and route suspension remain pending. Native save parsing still rejects
the verified original-loaded checkpoint at offset 300. No balance or original
simulation implementation is replaced.

## Repeat in the owned checkout

Use catalog-only inputs as described in [check storage](../../docs/check-storage.md)
and import the hash-verified checkpoint before using these commands. From the
owned checkout, these unused output names provide one bounded repeat; choose
new names for subsequent immutable reports.

```bash
python3 tools/prepare-rendering-progression.py \
  --report research/runtime/rendering-progression-results-20261101/target-user-inputs-01.json
python3 tools/working-retention.py --root . run --reserve-gib 2 -- \
  python3 tools/run-rendering-progression.py --case victory --software \
  --size 640x480 --skip-intros --checkpoint restore \
  --checkpoint-name checkpoint-camulos-05 --checkpoint-probe paused-cast \
  --checkpoint-reference research/runtime/rendering-progression-results-20261101/checkpoint-seeded-software-low-05.json \
  --deadline-seconds 360 \
  --preparation research/runtime/rendering-progression-results-20261101/target-user-inputs-01.json \
  --report research/runtime/rendering-progression-results-20261101/target-user-paused-cast-01.json
python3 tools/working-retention.py --root . run --reserve-gib 2 -- \
  python3 tools/run-rendering-progression.py --case victory --software \
  --size 640x480 --skip-intros --checkpoint restore \
  --checkpoint-name checkpoint-camulos-05 --checkpoint-probe combat \
  --checkpoint-reference research/runtime/rendering-progression-results-20261101/checkpoint-seeded-software-low-05.json \
  --deadline-seconds 420 \
  --preparation research/runtime/rendering-progression-results-20261101/target-user-inputs-01.json \
  --report research/runtime/rendering-progression-results-20261101/target-user-support-01.json
```

Both commands use lightweight capture and cannot satisfy the rendering oracle.
A nominal full run additionally requires `--evidence-mode full`, fresh campaign
entry without checkpoint/skip-intros and a separately passing
`check-rendering-progression.py` result. Retire only completed, queued outputs
through verified dual-store handoff after all physical consumers finish.

## Lightweight presentation readiness and private-display startup

The first fresh lightweight attempt, `target-nominal-victory-01.json`, timed
out without a flow/helper result. Its original World produced gameplay samples;
source/binary closure, immutable-input verification, stopped Wine and artifact
retention passed. No normal-Quit or outcome is inferred. Prefix return initially
refused a transient host client; `readiness-negative-closeout-01.json` retains
the later public recovery receipt and the unchanged negative acquisition.

Native controls isolated an independent presentation defect: `GlViewport` only
updated its painted World identity inside optional capture recording. With
capture disabled, the progression driver could not observe the required swapped
World generation. `readiness-negative-control-01.json` retains the exact old
binary/source: OpenGL and raster both fail without capture and pass with it.
`HOST.capture-independent-world-readiness` now records paint/swap identity
without enabling framebuffer readback or capture files. Queued frames do not
advance swapped identity; dedicated frame generations, legacy GUI identity and
menu clearing are covered. Guard06 passes four actual native presentation
cases, plus183Python and7Qt campaign cases. Broader renderer/pixel equivalence
remains separate.

The next attempt, `target-fresh-route-02.json`, failed before the original game:
the private X server exited during the build, reporting `Cannot write display
number to fd 4`. The launcher accepted digits from one pipe read and immediately
closed the reader, without requiring the terminating newline. This is consistent
with the observed startup failure; no compiler/gameplay failure is inferred.
`HOST.private-display-line-readiness` now waits for a complete bounded line across
split reads (32 bytes,10 seconds), retaining EOF/malformed/timeout/exited-server
refusals. `private-display-control-01.json` records15isolated controls and10actual
private-server startups in both extension configurations. The original failure
and source hashes remain unchanged. This narrow startup correction does not
renew broader output-isolation or campaign acceptance claims.

## Fresh route combat boundary

`target-fresh-route-03.json` passes source/binary stability, input verification,
stopped-Wine and complete artifact retention, but remains failed: the original
natural report is defeat (outcome0, “You were killed.”), not the required victory.
Its first actual software640 World has generation1, paint60 and82frames before
physical helper entry. This supplies live evidence for capture-independent
readiness, not campaign or pixel acceptance.

The Wizard cleared the bridge through six original Fireball hits (107HP→0),
returned to CF52 PoP and recovered from16 to36mana. Later engaged escorts (Goblin
slot6 and type10 slot5) consumed the reserve before Camulos; no boss support
submission or bounded boss pause occurred. These negative fields remain intact.
`INPUT.campaign-boss-mana-reserve` now gives the observed supported, rearmed
Wizard a bounded boss priority when mana covers an authored HP/20,6-mana estimate.
Four focused guards cover reserve thresholds, changed/dead/ambiguous roles,
low health and suspended route refusal. Actual original cast, target, damage and
natural-outcome assertions still decide success; this is test-driver strategy,
not a recovered damage formula or balance change. The later supported diagnostic below supplies live observations; the estimate
remains a native heuristic rather than a recovered formula.

## Current supported fresh diagnostic passes

`target-support-attack-04.json` repeats original checkpoint Save/Load/Quit with
all current source/binary fingerprints. It retains50mana before the volley,
records the candidate-checked sole-Zombie selection and actor3→target4 opcode7
submission, and stops before casting. All73callback pairs bind to the independent
Region envelope; seven checkpoint phases now retain333pairs. Guard08 passes
202Python,7Qt driver and4native paint/swap cases. These current input contracts
are bound by `scope-binding-headless-03.json` and `scope-binding-live-03.json`.

`target-fresh-route-04.json` succeeds as a skipped-intro lightweight diagnostic:
original natural victory has10XP for talking to Camulos and40XP for defeating
him; original OK, settled Realm/travel, actual Celtic region2World and normal
Main Quit pass. Both World observations have fresh generations; all graphics
owner counts retire to zero. It remains ineligible for nominal pixel acceptance.

The supported boss interval accepts six original Fireballs in9.22seconds while
Space holds the simulation clock; Wizard HP remains84 and mana37→1. The same
living original Zombie later exposes combat target4 after the actor3/target4
buffer submission. This is temporal target-acquisition observation, not complete
command-delivery causality. Four attributed player Fireball damage rows reduce
Camulos100→80→60→40→20. No original Zombie melee boss-damage row was observed;
no melee lethal is inferred. The original script owns Camulos's retreat and
portal, and the living Wizard enters it at16HP before the50XP report.

`HOST.campaign-supported-boss-diagnostic` and
`supported-boss-diagnostic-review-01.json` bind this narrow live sequence,
including all327paired callbacks, separately from the earlier negative results.
The reserve estimate remains native test strategy, not a recovered damage or
admission formula. The full enabled-intro/pixel case is recorded separately below;
route interruption/resumption, complete lifetimes/save parsing and the12case
matrix remain independent pending work.

Build the opt-in native presentation regression explicitly before running its
four modes (or their CTest entries):

```bash
cmake --build working/build/qt-shell --target viewport-world-readiness-test --parallel 4
```

The recorded guard command supplies its private Xvfb display, software GL and
matching render-service path; it never uses the user's desktop as a test target.

## Enabled-intro full nominal acquisition

`target-nominal-victory-full-01.json` succeeds without skip-intros or checkpoint
entry, with full bounded command/viewport and World-source capture. The living
Wizard reaches the original victory/settled Realm path at43HP; actual Celtic
regions1and2, normal Main Quit, source/player stability, immutable-input checks
and complete retirement pass. Viewport capture has26675rows and36907398000raw
bytes under its64GiB raw bound, about515MiB stored under1GiB. Command capture
retains8482rows/232397712raw bytes. The acquisition itself is separate from pixel acceptance. Physical outputs
remained available until the independent comparison completed, then passed
dual-store preservation and exact restoration before retirement.

The independent original replay has completed3042Worlds/2477427draw calls,
without reading native-player pixels or original destination captures.
The separately recorded comparison below checks every captured first/later
World and native paint.
The other matrix branches, route interruption/resumption, populated lifetimes
and native save parsing remain outside this one branch.

## Independent full nominal comparison passes

`target-nominal-victory-full-01-comparison.json` passes with zero differences:
3042independent original Worlds,2477427original draw calls,10491native World
presentations and9227724000compared pixels. Original World1 and region2World3034
are both independently captured and compared, including their first swaps.
All command surfaces retire; the reference process group stops and original
manifests/source stability and complete3046-artifact retention pass. Neither
native-player pixels nor original destination captures feed the original oracle.

`HOST.campaign-nominal-world-history` and
`nominal-world-history-binding-02.json` bind this enabled-intro, full-capture
victory/software640 branch. It is1of12current-source matrix branches; historical
results keep their old fingerprints. No complete matrix, route suspension,
populated object/vptr/generation lifetime, physical scanout, whole native
simulation, save-parser or balance acceptance is inferred.

The native scenario policy keeps its original-behavior comparison status at
`none`; its narrow live pixel integration uses the verified original build and
explicit equivalence fields in binding02. Binding01 and the failed metadata
audit04 remain retained. This correction changes no execution, source hash or
measured result.
