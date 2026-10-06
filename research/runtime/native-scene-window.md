# Native window playback validation (NS17)

This extends `NP.scene-playback`, `NP.scene-picking` and `NP.presentation` with a
bounded native-window execution scenario. It is separate from original input,
ray/selection, pause/cadence and live replacement equivalence.

## Executed window and inputs

The `world-scene-window-test` target compiles the **same `main.cpp` entry point** as
`mnm-world-scene-preview`, linked to the production scene, movement controls,
playback and navigation libraries. Its test-only macro adds a bounded action driver,
a post-committed-tick observer and Qt widget file dialogs. The normal executable
has no script options, observer or test driver. Both entry-point builds retain the
same real QTimer/QElapsedTimer, session, decoded assets, draw/refresh callbacks,
mouse picking, commands and Save implementation. Object names enable reliable
widget discovery without changing their semantic actions.

Each accepted normal/sanitizer run opens eight windows: uninterrupted and fresh
checkpoint processes in each of four diagnostic orientations. Inputs are an 8x8
exported crop of installed Celtic Forest `CFsec01.map`, its installed SPR/TTD,
installed `Creatures/redcap.ani` movement sequence base 0 and paired `RedCap.spr`.
The owned frozen crop keeps the exporter’s sealed boundary and projected object
policy. Start `(3,4,4)`, initial goal `(4,4,4)` and clicked goal `(6,4,4)` are
explicit exported standing cells. Animation bytes are loaded from the installation
and embedded in native checkpoints, rather than generated fixture ANI programs.
This does not validate other crops, creatures, full installed worlds or live hooks.

## Interaction and comparisons

The driver sends actual Qt mouse press/release events to the canvas and native
buttons, rather than calling Orders/session actions. Independent forward decoded
SPR ownership selects visible actor and terrain pixels. Right-click queues a
full-generation subject move at the clicked standing cell without advancing it.
Play uses the production real timer. A test-only **after-tick observer** clicks
Pause at an exact committed boundary; it changes no world fields, routes, ANI,
clock timestamps or render queues. It prevents machine-load differences from
changing the chosen continuation boundary. Small test input batches deliver Stop
and pre-admission capture in one Qt dispatch; the next real admitted tick applies
Stop. Exact observer boundaries are instrumentation, not an assertion of human
pause latency or original scheduling.

The scenario pauses with positive partial fine progress and an active owned ANI
cursor. A 220 ms paused wait preserves checkpoint bytes and actual canvas PNGs.
Save is clicked while playing, pauses before the real QWidget file dialog opens,
and is accepted through that dialog’s native Qt slot. Its durable file equals the
paused snapshot. The next process starts paused without selection, reselects the
visible actor and reaches exactly the uninterrupted native checkpoint, canonical
queue/fine-coordinate JSON, RGB565 image and PNG after the same admitted ticks.
Stop is issued while playing and applied before further motion; pending subject,
route cancellation and committed-cell state are checked. Independent explicit-step
processes also match the continued and stopped window state/frames.

Each build compares **40 complete OpenGL RGB565 frames** with independently decoded
forward SPR rasterization, and performs **four exact fresh-window continuations**.
Actual canvas/window screenshots and native snapshots are retained. Canonical
frame comparisons exclude the transient selection outline; actual canvas equality
is checked across the paused wait and modal Save. Input, binary, source, script and
artifact hashes are pinned in the accepted reports. Original manifest verification
brackets each installed-artifact experiment.

## Corrected native map click policy

Execution exposed NS14’s ordinary MAP `z+1` target annotation. Native map export
and frozen navigation admit a tile’s standing coordinate on its **own layer**.
For this crop, the earlier policy queued layer 5 when the clicked standing cell was
layer 4. The main-window adapter now declares `(x,y,z)` for rendered MAP tiles
above zero; layer zero has no standing-cell annotation. Composition still checks
bounds and XY agreement. Reachability remains the planner’s decision. Four-view
clicks now agree with exported standing cells and start actual movement. This is a
corrected intentional native art-to-cell policy, not an original ray recovery.
Diagnostic fixture tiles retain their explicit caller-supplied layers.

## Confirmed remaining display gap

Native Stop clears fine/previous ANI cursors. The existing scene requires an owned
displayed cursor, so the stopped creature remains in state/selection but contributes
**no body draw**. The scenario asserts and records this branch; it does not claim
idle or stopped presentation is implemented. A persistent/static idle body policy
and original idle/action animation mapping remain separate next work. Rendering an
invented idle pose or adding checkpoint fields is not part of this validation.

## Reproduction and accounting

```bash
cmake --build BUILD --target world-scene-window-test mnm-world-scene-preview mnm-map-navigation-export mnm-world-sandbox
python3 tools/test-native-scene-window.py BUILD NEW_OUTPUT
```

The runner requires the prepared read-only `working/game-clean` installation and
Xvfb/software GL. It creates exclusive output directories; scripts are bounded to
64 actions, 32 ticks per wait, 500 ms per delay and a 45-second per-window deadline.
They can save only new named artifacts under the supplied output directory.
No manual input is needed. Visual smoothness/responsiveness can still benefit from
a separate subjective check; the tests assert no original frame-rate equivalence.

[Accepted evidence](native-scene-window.json) retains reports and validation logs.
[Committed-history review](native-scene-window-history-review.json) checks exact
intermediate receipts separately without claiming a past gate or new execution.
Its [extension](native-scene-window-history-extension.json) retains the additional
intervening commit review. Historical evidence retains its hashes/statuses, including affected source
staleness. The new bounded window result does not refresh older broad oracle or
original comparison claims. Other installed maps/ANI profiles, groups/factions,
commander/summoned gameplay and live replacement remain outstanding.
