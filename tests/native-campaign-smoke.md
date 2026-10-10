# Public native campaign smoke test

Run from the repository root:

```sh
python3 tools/test-native-campaign.py
```

This is a strict test of **native Qt menus and native command presentation in
one campaign session**. It runs the same `tools/run-qt-shell.sh` and
`working/build/qt-shell/mnm-qt-shell` used interactively, requesting both
`--live-menus` and `--native-commands`. It never retries with either requirement
removed and never treats Wine-window fallback as a pass.

The combined route stages hash-checked `MnmMenu.dll` and `MnmRender.dll` imports
in the same disposable installation. The host creates versioned render, input,
command and recovery channels before launch. Qt menus own input during supported
menu screens; gameplay hands input to the native OpenGL viewport. The ordinary
menu-only route remains available. Cold Wine startup has a bounded two-minute
first-frame allowance; recovery retains the existing ten-second bound.

The previous public-mode rejection is preserved as historical admission evidence.
Combined gameplay/rendering behavior is under active validation; admission alone
is never a passing campaign result.

For just the build and mode check, without a game or display:

```sh
python3 tools/test-native-campaign.py --check-only
```

Once the combined route is supported, the same test will click the public Qt
Launch, New Game, Region Enter and campaign Mini Cancel controls. It preserves the
normal menu feature flags and default difficulty, waits for engine-confirmed menu
acknowledgements, requires at least three native command presentations during
campaign gameplay, sends Escape through XTest, and requires another three frames
after Cancel. An independent observer trace must confirm the original New
Game/Enter/Cancel callbacks, initialized World ticks and World resume on one
engine thread. The current v12 menu protocol is required. Missing screenshots,
retired menus, absent presentations and visible original gameplay all fail.

The test uses a private Wine prefix and preferences store, verifies the original
manifest before and after a game experiment, and terminates only its own process
group and Wine prefix. This is bounded test cleanup, not a normal Quit/save test.
By default it needs Xvfb for isolated keyboard input; Wine and libXtst are also
needed for live execution. To watch it on an existing X11/XWayland display:

```sh
python3 tools/test-native-campaign.py --display "$DISPLAY"
```

This explicit option changes focus and sends Escape on that display. Do not
interact with other windows during the short input sequence. `--timeout` bounds
the live run (default 330 seconds); the Qt driver has its own five-minute bound.

Each invocation retains a fresh `working/tests/native-campaign/run-*/report.json`,
build/mode-probe logs, and, if admitted, shell logs, screenshots and `flow.json`.
`success` is true only after the entire live flow and independent checks pass.
A successful `--check-only` returns zero with `success: false` and
`status: admitted-not-exercised`; that is admission evidence only.

The native frame count is sampled at the command renderer's completed-frame
callback, not from original-frame capture or command publication counters.
It is a presentation smoke check, not pixel equivalence, latency or GPU-driver
certification. Original drawing remains active in the existing native command
route. Complete original drawing replacement and actual character movement are
not claimed (`complete_drawing_replacement: false`, `movement_verified: false`).

Run the fail-closed report checks without Qt, Wine or game media:

```sh
python3 -B tests/test-native-campaign.py
```

Gameplay stress and throughput checks:

```sh
python3 tools/test-native-campaign.py --stress-seconds 20 --menu-cycles 3
```

The runner independently observes three original initialized World tick returns
before allowing Escape or other gameplay input. Native Region Entry/loading
frames cannot authorize gameplay. Stress injects physical XTest camera pans,
rotations, scene/HUD clicks and wheel events through the native viewport.
It repeats native Mini Cancel and resumes input after every handoff.

The default performance floor is 20 native completed frames and visible Qt
paints per second on average in each gameplay phase; one-second samples below
10 FPS and sampling stalls over two seconds fail. `--min-fps` makes the selected
floor explicit in the retained report. Private Xvfb runs select software Mesa;
performance there is scoped to that backend, not physical GPU certification.
Player movement, combat effects and pixel equivalence need separate evidence.

For active camera/gameplay stress, select Apprentice through the public native
Region Entry radio, avoiding Initiate's spell tutorial gating:

```sh
python3 tools/test-native-campaign.py --difficulty 1 --stress-seconds 20 --menu-cycles 3
```

Native framebuffer and independently copied original-owned primary images are
retained for each gameplay phase. They are unsynchronized diagnostic samples;
whole-window screenshots can include an overlapping original Wine desktop.
The shell lowers its identified original desktop during native presentation.
A read-only original-owned publication sampler is retained alongside separate
native presentation/Qt paint rates; none alone establishes simulation FPS.

Extended spell/creature selection and targeting attempt protocol:

```sh
python3 tools/test-native-campaign.py --difficulty 1 --stress-seconds 40 --menu-cycles 3 --timeout 600
```

The final retained software-Mesa run passes all four phases, averaging51–55 native
FPS/about 33visible FPS, worst 1-second window 14 FPS, zero fallback/recovery.
Spell/creature HUD clicks and source/target orders are attempted, not asserted
as successful casting or combat. Initiate tutorial gating remains a separate
negative route. Fresh source-bound reports and profiles are linked from
`research/runtime/native-campaign-rendering.md`; historical failures remain.

Successful casting and combat through the native viewport:

```sh
python3 tools/test-native-campaign.py --require-casting-combat --difficulty 3 --stress-seconds 60 --timeout 600
```

This mode also needs Tesseract and Pillow. It selects the normal highest
difficulty through native Region Entry; nearby live enemies make a bounded
combat journey possible. Apprentice (`--difficulty 1`) is retained as an
exploratory alternative, with early movement gating still unresolved. Initiate
and the other difficulty journeys remain separate tests.

The test selects the Zombie spell, right-clicks clear ground, and requires a
new living player-owned Zombie plus `0/15` then `1/15` in independently captured
native and original-owned images. It observes original creature state while
using public actor selection, camera centering, ground orders and another
normal summon near a living enemy. Combat requires player Zombie original melee
calls to reduce an opposing active target's positive health, followed by player
wizard or combat Zombie lethal melee HP depletion of that same enemy. Original slot
reuse is checked by observing the type/owner transition around each cast;
the attacker may be the later combat summon. Existing actors, order attempts,
scripted damage, nonplayer hits, altered images and incomplete traces cannot pass.

No simulation or health/mana writes, cheats or scenario edits are used. The
private observation hook verifies original melee entry bytes and forwards the
original function unchanged. A single worker compresses owned diagnostic images
after GL-thread readback, keeping PNG saves out of the UI event loop. Capture
overhead still counts toward the same strict performance floors. All evidence,
including failed placements, movement probes and slow runs, remains retained.

The private Xvfb software-Mesa backend uses an explicit four-worker pool;
`--software-threads` selects and records another size. This controls both native
and original Wine software-renderer workers and changes no game rules. The
four-worker rerun passed resumed camera/HUD stress but still failed briefly during
repeated post-hit wizard recentering. That run remains historical negative evidence. The portrait follow-up below
restores recentering during combat and passes the same timing floors. The
ordinary route without portrait stress still uses terrain hover.

The [retained passing result](../research/runtime/native-campaign-combat-passed-20261010.json)
verifies two player Zombie summons, 14 Zombie damage events and Cornelius's
`2 → 0` melee finishing blow on the same enemy, then native Mini Cancel/resume.
It presents 5,798 native frames and 3,958 visible Qt paints with no fallback or
recovery. Combat averages 27.21 native/20.57 paint FPS, worst 10.95 FPS; resumed
stress averages 55.06/35.40 FPS, worst 15.89 FPS. This is one bounded software-Mesa
journey. The later portrait route below validates the recentering correction.

## Portrait rendering stress

Run the same bounded native casting/combat route with restored portrait
reselection during combat and sixty seconds of post-victory recentering:

```bash
python3 tools/test-native-campaign.py --require-casting-combat --difficulty 3 \
  --stress-seconds 60 --portrait-stress-seconds 60 --software-threads 4 \
  --timeout 600
```

The portrait phase must separately pass 20 FPS native/paint averages and 10 FPS
one-second windows. The complete flow also includes both capture boundaries
and retains its two-second stall ceiling. A stable wizard face crop must match
independent original-owned pixels within one channel value; hashes, both image
saves and absence of fallback are required. This is not full-frame or animated
World equivalence. Simulation and balance remain original. Native staging
disables legacy draw skipping while preserving the selected game speed and
original pacing instructions; precise scheduling equivalence remains pending.

## Blocked casts and ranged damage

```bash
python3 tools/test-native-campaign.py --require-casting-combat --spell-cases \
  --difficulty 3 --stress-seconds 60 --portrait-stress-seconds 60 \
  --software-threads 4 --timeout 600
```

This optional route requires distant invalid-target and insufficient-mana Zombie
attempts with no new living Zombie or mana loss, plus original defended enemy HP
reduction during a player Fireball71 effect. It retains separate cast/impact/
damage diagnostics and original owner/identity checks. UI refusal differs from
internal cast admission. The starting green spell is Fireball, so successful Cure
healing is a separate pending loadout case. Ordinary retreat orders aim to keep
the wizard alive through portrait stress. Wizard death, unexpected World exit,
busy acquisition beyond its bounded retry and slow rendering fail the journey.
See [extended gameplay findings](../research/runtime/native-campaign-extended-gameplay.md)
for failures and the remaining scope.

## Normal Quit after gameplay

```bash
python3 tools/test-native-campaign.py --require-casting-combat --spell-cases \
  --normal-quit --difficulty 3 --stress-seconds 20 --software-threads 4 \
  --timeout 480
```

This route completes ordinary Mini Cancel/resume, then native Mini Quit. The
original confirmation remains rendered through native commands. Physical No
requires an observed original answer and World resume; repeated Yes requires the
native defeat report, Continue, fresh native Main and Main Quit. Both original
launcher and native shell must exit0. Confirmation images use the native
framebuffer and independent original-owned publication. Bounded private cleanup
is still available after failures, but cannot satisfy the normal Quit outcome.
The test now waits for late introductory Hermes prompts before non-tutorial spell
attempts. Initiate tutorial steps, Cure loadouts and modal pause equivalence remain
separate cases. [Extended research](../research/runtime/native-campaign-extended-gameplay.md)
preserves failed readiness/targeting attempts and the fresh Quit pass.
