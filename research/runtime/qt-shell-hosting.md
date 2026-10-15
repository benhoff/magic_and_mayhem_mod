# Qt shell window hosting

Native embedding backend implemented 2026-10-03 in `apps/qt-shell/`, independent
of the engine models. Select it with `--renderer native`; the default is now
the [OpenGL presentation backend](opengl-presentation.md).
Launch entry: `tools/run-qt-shell.sh`. Generated native executable:
`working/build/qt-shell/mnm-qt-shell`.

Confirmed on this host: Qt 6.11.2 Widgets / XCB compilation, offscreen startup,
and an Xvfb integration test with an external Qt fixture process. The test
verifies discovery of the visible `MagicMayhem` desktop title, ancestry beneath
the shell's native window after embedding, and survival outside that ancestry
after detachment. Both CTest cases pass. Confidence is high for this fixture
contract, not for original game presentation or input.

In native mode, the shell's Launch game action invokes the existing `tools/run-game.sh` with
`launch --no-gamescope --window-size 800x600` and the working x86-64 Wine prefix.
It snapshots X11 window IDs before launching, then polls for one newly visible
Wine desktop with title `MagicMayhem` (or its title suffix). Preexisting windows
are excluded, ambiguous matches are not automatically attached, and missing
windows leave the log and retry controls available. Window IDs are discovered
on every launch; none are hard-coded or persisted.

A separate XCB connection reads window properties/liveness only. Reparenting
uses Qt's [foreign-window interface](https://doc.qt.io/qt-6/qwindow.html#fromWinId)
and [widget container](https://doc.qt.io/qt-6/qwidget.html#createWindowContainer).
The game remains a separate Wine process using its original rendering path.
Native Wayland hosting and capture/display of pixel frames are not implemented.

The game was not launched during these synthetic tests. Manual checklist:

1. Open the shell in a graphical X11/XWayland session and click Launch game.
2. Confirm the Wine desktop embeds and a map renders normally.
3. Confirm mouse coordinates, keyboard focus, edge scrolling and game dialogs.
4. Resize the shell, detach and reattach, and check presentation/input again.
5. Exit the game, verify the launcher finishes, then close the shell.

Hands-on evidence, 2026-10-04: after being asked to test the native embedding
backend, the user reports that everything seems to be working fine. Retained
launcher log: `working/logs/run-20261004T160537Z.8Kl2VI/`. Its environment records
the pinned No-CD executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
`/usr/bin/wine`, display `:1`, an 800x600 Wine desktop, gamescope disabled and
`working/wineprefix-x86_64`. Confidence: live user observation supports successful
operation in this run. Individual checklist actions, map, duration and audible
movie behavior were not enumerated, so complete hosting/lifecycle equivalence
remains unverified. The recurring Mesa/pixel-format/Quartz diagnostics coexist
with the reported success. This evidence concerns native foreign-window hosting;
it does not validate OpenGL frame streaming or replace original engine drawing.

Shell closure follows the original Quit path while its launcher runs. Since
2026-10-12, the explicit Hard restart toolbar action / Ctrl+Shift+R also provides
forced recovery. `game_process.hpp` gives each launcher a new Unix session and
kills its process group with SIGKILL. For an active game, it runs `wineserver -k`
then `wineserver -w` with the exact selected WINEPREFIX. Wine processes outside
that prefix are outside the shutdown scope; other processes sharing it stop too.
Staging cancellation kills only its launcher group. Shutdown is asynchronous,
has a ten-second deadline, and refuses relaunch after errors or timeout.

The shell suspends input and presentation polling, stops native playback, retires
the menu bridge, cancels render consumers, discards displayed frames and creates
fresh channels through the existing launch path. Forced termination does not
claim producer END or graceful engine cleanup. Unsaved progress is lost; accepted
preferences keep the existing persistence policy. Launch options remain selected,
and any pending close-after-Quit is cleared. The action is disabled while checking
the installation or waiting for shutdown. It assumes the Qt event loop remains
responsive; a frozen shell itself cannot dispatch the action.

`game_restart_action.hpp` reserves the restart chord in an application event
filter before the viewport forwards input, including when fullscreen hides the
toolbar. The synthetic widget test checks input-filter priority, disabled state,
auto-repeat refusal, hidden-button activation and unrelated-window isolation.

`tests/game-hard-restart-test.cpp` exercises frozen Unix descendants, continued
Qt timer progress, exact prefix kill/wait ordering, unrelated-process survival,
duplicate requests, launcher reuse, missing/failed/hung wineserver refusal, and
both staging and active menu-session cancellation. These are synthetic host
checks; actual-game freeze recovery, foreign-window cleanup on force-stop and
current rendering/media/input equivalence after relaunch remain pending. Historical
execution fingerprints remain unchanged. Application instructions are in
[the shell README](../../apps/qt-shell/README.md).

Related layout change: the unchanged logging bridge moved from
`reconstruction/shadow/` to `runtime/shadow/`. Rebuilding from the new path
produced the same SHA-256 as the already staged DLL, and the three offline
shadow staging/comparison tests passed. No staged game files were rewritten.

Validation, 2026-10-12: the new recovery test and six host regressions passed.
The [immutable synthetic result](qt-hard-restart-synthetic-retry-20261012.json)
records prospective scope claims and verified stable source hashes. The initial
[result](qt-hard-restart-synthetic-20261012.json) preserves the sandbox-only Xvfb
socket failure; permitting local display sockets passed the same fixture. The
repository storage guard interrupted the first test link below its free-space
reserve, so the final test executable was linked in RAM-backed `/tmp` under
the same guard. The updated public shell executable was built successfully.
No historical hashes or original artifacts were changed.
