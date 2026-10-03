# Magic & Mayhem Qt shell

Native Qt application code, separate from reconstructed engine algorithms.
The default viewport presents captured engine frames through an OpenGL 3.3
texture shader. A native Wine-window embedding backend remains available.
The host can be x86-64 while the injected frame bridge and game are PE32 i386.

## Build and open

Requires CMake, a C++17 compiler, Qt 6.5+ Widgets / OpenGLWidgets / OpenGL
libraries, pkg-config and XCB development files. The frame bridge additionally
requires Clang, llvm-dlltool, lld-link and Wine. The current host supplies these
and Qt 6.11.2. No game is launched merely by opening the application.

```bash
./tools/run-qt-shell.sh
```

Click **Launch game** to build the render DLL, stage a hash-checked disposable
game copy, and run it through `tools/run-game.sh`. The game uses a separate
800x600 Wine desktop. Its primary DirectDraw surface is copied after successful
Blt/BltFast/Flip calls, converted to RGBA, and displayed in Qt via OpenGL with nearest
filtering and aspect-preserving letterboxing. The launch log and normal launcher
logs remain available. **Check installation** uses the existing launcher checks.

**This first migration changes final presentation, not engine drawing.** The
original DirectDraw renderer remains active. Use the separate Wine game window
for keyboard/mouse input: forwarding input from the OpenGL viewport is not yet
implemented. Live map rendering remains unvalidated; synthetic producer-to-Qt
readback passes. The native game output is retained during that validation.

Every launch gets a new shared frame file in `working/runtime/render/` and a
fresh game copy in `working/experiments/opengl-render/`, with EXE / DLL hashes
and build provenance. Source game files and `original/` are not modified.
Exit the game and wait for the launcher to finish before closing the shell;
closure is blocked while the launcher runs. The default persistent prefix is
`working/wineprefix-x86_64`.

For the original embedding backend:

```bash
./tools/run-qt-shell.sh --renderer native
```

That backend requires Qt and Wine to share an X11/XWayland display. It snapshots
window IDs before launch and embeds exactly one new visible `MagicMayhem`
desktop. **Detach game** restores a separate native window, and **Attach game**
retries discovery. Click that viewport to focus game input. Resizing that
container does not replace the game's renderer or change its internal resolution.
The launcher script defaults `QT_QPA_PLATFORM=xcb`; explicitly select a different
Qt backend only for the OpenGL stream viewport. Native Wayland embedding is
unsupported.

## Verification

```bash
cmake -S apps/qt-shell -B working/build/qt-shell
cmake --build working/build/qt-shell --parallel 4
ctest --test-dir working/build/qt-shell --output-on-failure
./tools/test-render-bridge.py
```

CTest checks pixel conversion (palette, RGB565, RGB24, RGB32, negative pitch,
padding and invalid masks), frame-stream guards, headless shell startup, external
window embedding/detachment and known-pixel OpenGL framebuffer readback.
The GUI tests use isolated Xvfb displays; OpenGL readback uses Mesa software
rendering when requested by the test. Local X server sockets must be permitted.

The Wine integration test calls the PE32 surface hooks using a synthetic
DirectDraw interface, verifies original return values / last-error state and
lock/unlock pairing, checks mapped RGBA bytes, then feeds those exact bytes
through Qt/OpenGL framebuffer readback. It uses a dedicated test prefix and
never launches the game. It does not prove that the original game's surface
presentation or input works.

For opt-in drawing evidence, open `./tools/run-qt-shell.sh --capture-draws` and
click **Launch game** when ready. It records a bounded API inventory and one
small opaque/source-keyed copy for offline pixel comparison. The launch log
prints its capture directory. This is a drawing reconstruction aid; see
[capture and replay procedure](../../research/runtime/render-drawing-inventory.md).

CLI: `--repo DIRECTORY`, `--renderer opengl|native`, `--capture-draws`, `--smoke-test` (no game),
`--opengl-test` (known pixels) and `--stream-test FILE` (synthetic readback).
Use `--renderer native --smoke-test` with the offscreen Qt backend.

## Boundaries and references

- `apps/qt-shell/`: UI, shared frame reader and OpenGL presentation.
- `runtime/render/`: PE32 DirectDraw frame capture.
- `renderer/`: native OpenGL drawing components and offline capture replay.
- `runtime/shadow/`: PE32 pathfinding instrumentation.
- `reconstruction/pathfinding/`: original engine models.
- `tools/`: builds, staging, launches and verification.

See [OpenGL migration scope and evidence](../../research/runtime/opengl-presentation.md)
and [shared frame format](../../research/formats/render-frame-stream.md).
Qt interfaces: [QOpenGLWidget](https://doc.qt.io/qt-6/qopenglwidget.html),
[foreign windows](https://doc.qt.io/qt-6/qwindow.html#fromWinId),
[QProcess](https://doc.qt.io/qt-6/qprocess.html).

The [native blit renderer](../../renderer/README.md) now executes captured
opaque/source-keyed copies through OpenGL and compares native pixels. Run
`./tools/replay-render-capture.py CAPTURE_DIRECTORY --backend opengl --headless`
for that offline check. The shell's live viewport still presents original
engine frames; routing live drawing through the new renderer is later work.
