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

The current public shell rejects that combination with exit 2. The test therefore
exits nonzero at `combined-mode-admission`, before launching the game. This is the
first integration blocker, not a passing campaign test. The click-through driver
is compiled but cannot execute through the public route until menu/rendering
integration exists. Merely removing argument rejection will not suffice: live
menus currently select Wine embedding and their game runner stages the menu
bridge, separately from the rendering bridge/command channels.

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
