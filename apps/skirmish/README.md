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

## Cure in shared native battles

Select your wizard, choose **Cure**, then click a living friendly Sentinel.
Cure costs3 mana and heals8 up to that creature's combat maximum. Its vector halo
follows logical movement while the owned ANI player runs. Dead/removed targets
receive no healing or refund. Cure, melee, movement and bolts share one native
battle transaction and authoritative health/mana. Save/load retains pending Cure
orders and in-flight playback, including in the campaign window.

The asset-free arena uses a declared short ANI fixture; separate integration checks
load real installed Cure recipe74 into the same runtime. No original spell balance
or animation cadence equivalence is claimed. New campaigns bind this profile;
older campaign and skirmish checkpoints retain their original behavior.

CLI script rows accept `TICK cure WIZARD-SLOT TARGET-SLOT`. Campaign commands accept
`mnm-campaign-cli cast INPUT OUTPUT cure TARGET-SLOT`. Target generation is checked
against the active battle. See [scope and evidence](../../research/runtime/native-cure-battle.md).

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
`python3 tools/test-native-map-campaign.py ASSET_ROOT` records the installed-process
journey and core/asset regressions, with original-manifest verification before
and after. Run it under the main checkout's storage guard. Retained execution
reports are separate from the reusable `working/build/native-map-campaign`
build directory. See `research/runtime/native-map-campaign.md` for scope.

For the playable terrain window, use **New map campaign…** and select the
installed game directory, or start directly:

```sh
working/build/native-map-campaign/mnm-skirmish --asset-root working/game-clean --map-campaign
working/build/native-map-campaign/mnm-skirmish --asset-root working/game-clean --load-campaign working/plains-saved.mnmc
```

Summon on a visible standing-cell diamond. Select a Sentinel and right-click a
cell to move, or an enemy to attack. Select your wizard and use Arc Bolt on an
enemy. Save/Load campaign pauses play; victory enables Continue to the finale.
The terrain uses installed sprites with native cell/unit overlays. This view
uses logical cell positions and unshaded terrain; original lighting, creature
sprite animation and full original maps remain separate work.

Add `--window` to the map validation runner for the terrain marker/picking
contract, five-process map window journey and complete existing window suite.
Qt configuration is isolated per run. Source declarations and raw failed runs
remain retained; a new successful report never refreshes old evidence hashes.
See `research/runtime/native-map-campaign-window.md` for the interaction policy.

The combined map/Cure journey runs without manual playtesting:
`tools/test-native-map-campaign.py ASSET_ROOT --cure` (under the storage guard).
It exercises real Qt controls across separate processes and checks exact saved
state against headless continuation, including one heal and win/advance.
Use `mnm-map-campaign-cli ASSET_ROOT create-cure OUTPUT` for the optional Cure
profile; ordinary map creation retains existing checkpoint identities. See
`research/runtime/native-map-cure-automation.md` for bounds and validation scope.

New map campaigns now enable **Magic Sphere** and **Cure**. Select your wizard,
choose Magic Sphere, then click an enemy. Its straight projectile can miss a
moving target and stops at blocked/occupied terrain and the sealed perimeter.
Save/Load campaign retains simultaneous Sphere and Cure continuation. Existing
map checkpoints keep their previous spell profile and content identity.

`mnm-map-campaign-cli ASSET_ROOT create-sphere OUTPUT` creates that profile from
the CLI. The combined automated journey is
`tools/test-native-sphere-map.py ASSET_ROOT --output NEW_DIRECTORY`, run under
the main storage guard. It includes normal/sanitized contracts and actual Qt
move/cast/save/restart/win/advance checks. See `research/runtime/native-sphere-battle.md`.

Magic Sphere is available in newly created arenas/campaigns. Select your wizard,
choose **Magic Sphere · 8 mana**, then click an enemy. It travels along a straight
fine-unit trajectory and can hit creatures in its path; a hit can retain flight.
The declared arena profile uses20 damage/speed20/lifetime100 and an authored ANI
fixture. Installed configuration binding, original ricochet and physical sound/
sprites remain separate. Arc Bolt retains its existing authored behavior.

The CLI accepts `TICK sphere WIZARD-SLOT TARGET-SLOT`; campaign CLI accepts
`cast INPUT OUTPUT sphere TARGET-SLOT`. Cure-only and effects-free campaign saves
retain their exact earlier profiles. See
[native Sphere scope](../../research/runtime/native-sphere-battle.md).
