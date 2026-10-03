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

The game was not launched during these tests. Pending manual checks:

1. Open the shell in a graphical X11/XWayland session and click Launch game.
2. Confirm the Wine desktop embeds and a map renders normally.
3. Confirm mouse coordinates, keyboard focus, edge scrolling and game dialogs.
4. Resize the shell, detach and reattach, and check presentation/input again.
5. Exit the game, verify the launcher finishes, then close the shell.

The first version blocks shell closure while its launcher is running. It does
not terminate Wine or use broad process-kill commands. Application instructions
and current limitations are in [the shell README](../../apps/qt-shell/README.md).

Related layout change: the unchanged logging bridge moved from
`reconstruction/shadow/` to `runtime/shadow/`. Rebuilding from the new path
produced the same SHA-256 as the already staged DLL, and the three offline
shadow staging/comparison tests passed. No staged game files were rewritten.
