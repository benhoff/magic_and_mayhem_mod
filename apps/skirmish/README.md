# Native skirmish

A small playable, entirely native arena. No Wine, game binary, installed media,
or original assets are required. This is an authored rules fixture, not a claim
that the original game's combat or balance has been recovered.

Build and launch from the repository root (Qt 6.8+ Widgets, C++17, CMake):

```sh
python3 tools/working-retention.py run --reserve-gib 0.5 -- \
  sh -c 'cmake -S apps/skirmish -B working/build/skirmish -DCMAKE_BUILD_TYPE=Debug && cmake --build working/build/skirmish -j 6'
working/build/skirmish/mnm-skirmish
```

Select your wizard. **Summon Sentinel** then click nearby empty ground;
**Arc Bolt** then click an enemy. Select your Sentinel and right-click ground
for Move or an enemy for Attack. **Stop creature** holds position until another
order. A creature resumes automatic targeting when its Move finishes.

**Play/Pause**, Space and **Step** control simulation time. **Save skirmish**
and **Load skirmish** pause playback. Checkpoints retain pending orders, creature
movement/attacks, mana timers, cast windup, projectiles and terminal outcomes.
Selection and cursor mode are window state and reset on load. **New skirmish**
restarts the arena. To load at startup:

```sh
working/build/skirmish/mnm-skirmish --load checkpoint.mnms
```

The CLI creates, inspects and resumes the same state:

```sh
working/build/skirmish/mnm-skirmish-cli create initial.mnms
working/build/skirmish/mnm-skirmish-cli resume initial.mnms result.mnms 100 orders.txt
```

Script rows use absolute ticks **before admission**: `0 summon 0 2 5`,
`3 bolt 0 1`, `4 move 2 1 3`, `5 stop 2`, `6 attack 2 1`.
Slots in this bounded fixture have generation 1; GUI players never see these IDs.
CLI output files must be new. A checkpoint with pending orders already contains
those inputs: omit them from a continuation script to avoid submitting them twice.

Run tests with `ctest --test-dir working/build/skirmish --output-on-failure`.
Window tests require Xvfb. The test target uses the production entry point and
window, plus a bounded driver; the production executable has no test scripting.
Research and remaining limitations: [native-skirmish.md](../../research/runtime/native-skirmish.md).

New sessions now use the validated ordinary No-CD numerical health backend.
Spell damage values, AI, mana, creature profiles and vector graphics remain
fixture policies. Native healing is available as an engine API with cap/dead
refusal tests; this increment adds no healing spell or balance change. New
checkpoints bind the health backend; older saves retain their original fixture
backend. See [ordinary health research](../../research/runtime/ordinary-health.md).

## Authored native campaign

`mnm-campaign-cli` adds two declared duels over this same arena. It uses the
campaign coordinator to preserve pending input, active battle resources and a
single result per attempt. Victory selects the next region; defeat/draw allow
retry. No new rewards or combat balance changes are added.

```sh
working/build/skirmish/mnm-campaign-cli create campaign.mnc
working/build/skirmish/mnm-campaign-cli start campaign.mnc battle.mnc
working/build/skirmish/mnm-campaign-cli cast battle.mnc pending.mnc summon 2 5
working/build/skirmish/mnm-campaign-cli resume pending.mnc summoned.mnc 3
working/build/skirmish/mnm-campaign-cli cast summoned.mnc bolt.mnc bolt
working/build/skirmish/mnm-campaign-cli resume bolt.mnc resumed.mnc 3
working/build/skirmish/mnm-campaign-cli inspect resumed.mnc
```

Outputs must be new paths. Reuse `cast` and `resume` to play through the
duel; `start` admits the next mission or retry after a result. JSON includes
`last_result` and a separate `settlements` count: reopening a settled result
emits no new settlement. The campaign is currently a headless authored slice;
the existing skirmish window and raw skirmish saves retain their separate flow.
[Content binding, execution scope and limits](../../research/runtime/native-campaign-arena.md).
