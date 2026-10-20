# Physical campaign summon/combat driver

The retained default resident attempt `working/tests/native-campaign/run-rbpf499y`
selected the spell without selecting the wizard. It created no living Zombie and
recorded no original player cast return or mana debit. The retained native image
shows the wizard without selection brackets. The helper selected/centered the
wizard only inside its optional `--spell-cases` branch. This is a confirmed driver
precondition omission; a fresh original run is required to establish its effect.

The separate `run-v0su8jdz` summoned successfully but fitted a camera transform
from short, obstacle-limited movement. For wizard `(12,73)` and enemy `(31,52)`,
that fitted approach points steeply upward. Ground navigation instead uses the
same existing isometric tile/height projection as the spell targets:
`sx=32*(dx-dy)`, `sy=16*(dx+dy-dz)`, scaled to at most `(170,110)` pixels from
centered ground `(400,300)`. Center first, then read fresh actor/target positions.
Independent original movement deltas remain mandatory movement evidence; they
do not establish a projection matrix or original pathfinding equivalence.

Implementation and validation are recorded separately under
`HOST.campaign-physical-combat-driver`. The positive initial cast remains one
physical order, with the existing twelve-second newly living owned-Zombie gate.
Control-count OCR, actual original mana debit, Zombie melee, same-enemy party
lethal and a living wizard remain required. No game health, mana, AI, positions,
timing limits or throughput floors change. The complete parent campaign may fail
its unchanged 19 FPS acceptance even when these physical-input outcomes pass.

Fresh validation passes six new driver controls and twenty-three existing
campaign controls. The default live route creates the initial owned Zombie and
native `0/15` to `1/15` count, debits 4,352 fixed-point mana units, observes two
independent movement deltas, and records sixteen original Zombie melee events.
The combat Zombie delivers the same enemy's lethal `2 -> 0` health transition;
the wizard is subsequently observed living with 86 health. Complete raw row
prefixes and capture hashes are retained, including a corrected metadata receipt
clarifying that the post-input prefix can contain subsequent observations.

The parent flow completes and quits normally without fallback or recovery, but
whole-session acceptance fails its unchanged 19 FPS gate at 8.671475 engine FPS.
Its raw failed report is retained independently. The scoped result supplies only
`HOST.campaign-physical-combat-driver`, not the wider
`HOST.native-campaign-casting-combat` throughput contract. Three isolated
prelaunch dependency failures remain retained with no execution credit.
Extended spell and portrait journeys remain outside this default-route proof.

Run the controls with `python3 -B tests/test-campaign-combat-input.py` and
`python3 -B tests/test-native-campaign.py`. The live reproduction uses
`tools/test-native-campaign.py --native-draw --resident-world --independent-world
--hardware --require-casting-combat --difficulty 1 --min-fps 19 --stress-seconds 60
--normal-quit --timeout 600 --skip-movies`, with `--source-game` pointing to the
verified owned reference installation. Declare the new driver's `.live` scenario
before execution and use the storage guard. An isolated checkout also needs its
local variant installation manifest and a verified music catalog physically
inside `working/`; cross-checkout catalog symlinks are refused. The retained
failed attempts establish these prerequisites without implying gameplay failure.

### Integration with Medium input admission

The composed helper preserves the newer distinct-spell/hover/press-receipt
protocol and summon reconciliation. Caster readiness precedes its selector;
navigation centers before fresh state reads. Eleven driver controls, 33
admission controls, seven observation controls and 26 campaign controls pass.
The earlier live proof remains historical after these source/dependency
changes. Next: run one new bounded default physical-input cohort with the
composed helper and register new fingerprints. This source merge does not
claim that live rerun or broader acceptance.

### Separate current composition controls and historical live proof

`HOST.campaign-physical-combat-driver` retains its original scope and
historical physical-input milestone. Its changed source needs a new bounded
default live cohort. `HOST.campaign-input-admission-composition` separately
records current headless callback/read-ordering and readiness/receipt controls;
it makes no physical execution or full frame-freshness claim. The first
`CINPUT.admission-merge-controls-20261020` declaration used an intermediate
draft scope and remains preserved as a retired claim. A separate current
composition declaration/result supplies the headless evidence. No old
execution hashes or live acceptance claims are refreshed.
