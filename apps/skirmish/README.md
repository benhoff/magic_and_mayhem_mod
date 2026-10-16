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
working/build/skirmish/mnm-campaign-cli create campaign.mnmc
working/build/skirmish/mnm-campaign-cli start campaign.mnmc battle.mnmc
working/build/skirmish/mnm-campaign-cli cast battle.mnmc pending.mnmc summon 2 5
working/build/skirmish/mnm-campaign-cli resume pending.mnmc summoned.mnmc 3
working/build/skirmish/mnm-campaign-cli cast summoned.mnmc bolt.mnmc bolt
working/build/skirmish/mnm-campaign-cli resume bolt.mnmc resumed.mnmc 3
working/build/skirmish/mnm-campaign-cli inspect resumed.mnmc
```

Outputs must be new paths. Reuse `cast` and `resume` to play through the
duel; `start` admits the next mission or retry after a result. JSON includes
`last_result` and a separate `settlements` count: reopening a settled result
emits no new settlement. The campaign window below uses these same checkpoints;
raw standalone skirmish saves retain their separate flow.
[Content binding, execution scope and limits](../../research/runtime/native-campaign-arena.md).

## Native campaign window

Run `mnm-skirmish --campaign` for the two-duel campaign, or
`mnm-skirmish --load-campaign CHECKPOINT.mnmc` to restore it paused.
The window also has New campaign, Save campaign and Load campaign buttons.
After a duel, Continue advances on victory and Retry starts another attempt after
defeat/draw. Save between duels or during play; pending commands, casts and
projectiles resume with the campaign. Final completion disables Continue.
Standalone skirmish save/load remains available as a separate mode.

## Installed Plains campaign

The map campaign plays an authored first mission on a sealed8x8 crop of the
installed Plains terrain, then advances to the existing arena finale. The
synthetic one-cell movement profile and native combat rules are unchanged.
MAP, terrain catalog and sprite bytes bind the campaign content revision; select
the same installed content when reopening a checkpoint.

```sh
working/build/native-map-campaign/mnm-map-campaign-cli working/game-clean create working/plains-seed.mnmc
working/build/native-map-campaign/mnm-map-campaign-cli working/game-clean start working/plains-seed.mnmc working/plains-active.mnmc
working/build/native-map-campaign/mnm-map-campaign-cli working/game-clean cast working/plains-active.mnmc working/plains-summon.mnmc summon 2 5 3
```

The map CLI takes `ASSET_ROOT` before the existing create/start/cast/resume/inspect
commands. Summon accepts optional Z (default1 for the arena); `move INPUT OUTPUT
X Y Z` orders the first friendly Sentinel. All output files must be new.
`tools/test-native-map-campaign.py ASSET_ROOT` records the installed-process
journey and core/asset regressions, with original-manifest verification before
and after. Run it under the main checkout's storage guard. Retained execution
reports are separate from the reusable `working/build/native-map-campaign`
build directory. See `research/runtime/native-map-campaign.md` for scope.
