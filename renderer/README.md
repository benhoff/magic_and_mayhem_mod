# Native OpenGL renderer

New rendering code, separate from original-engine reconstructions, the PE32
capture hooks in `runtime/render/`, and the Qt shell in `apps/qt-shell/`.
The renderer owns persistent native-pixel surfaces, accepts rectangular updates
and opaque/exact-keyed copies, and resolves indexed palettes or RGB masks with
a presentation shader. Replay compares native output and displayed colors with
the CPU reference. Live game command routing remains pending.

## Replay a capture

```bash
./tools/replay-render-capture.py CAPTURE_DIRECTORY --backend opengl --headless
```

`CAPTURE_DIRECTORY` contains `blit-0001.bin` from the existing capture bridge.
The command builds `working/build/renderer/mnm-render-replay`, runs the OpenGL
blitter, and writes a JSON report, native output bytes and PPM previews under
`working/experiments/render-replay/`. The report compares OpenGL against both
the captured output and the independent CPU reference. All three must agree
for exit status 0; mismatches return 1, unavailable/failed rendering returns 2.
There is no automatic CPU fallback when OpenGL was requested.

`--headless` uses isolated Xvfb and Mesa software rendering. Omit it to use the
current display and driver. Driver/vendor/version and capture/output/executable
hashes are recorded. A GPU-generated `opengl-presented.png` and its raw RGBA
hash are checked against independent CPU palette/mask conversion.
`--gl-executable PATH` reuses an explicitly selected build.
Without `--backend opengl`, the existing CPU-only replay behavior remains available.

A known synthetic example from development:

```bash
./tools/replay-render-capture.py working/tests/render/run-rin2peem/fast \
  --backend opengl --headless
```

This does not launch Wine or the game. To collect a new game sample, use
`./tools/run-qt-shell.sh --capture-draws`, click **Launch game**, and follow the
[capture procedure](../research/runtime/render-drawing-inventory.md).

## Build and test

Requires C++17, CMake, Qt 6.5+ Gui/OpenGL, desktop OpenGL 3.3, and Python 3 for
the tooling/tests. Xvfb enables isolated graphics tests.

```bash
cmake -S renderer -B working/build/renderer
cmake --build working/build/renderer --parallel 4
ctest --test-dir working/build/renderer --output-on-failure
./tools/test-render-bridge.py
```

The six CTests cover CLI help, the existing CPU reference tests, 196 OpenGL
draws, 192 ordered persistent-surface updates/copies, palette cycling/lifetimes,
capture/CLI integration, and ordered command sessions. The integrated Wine bridge test generates
fresh x86 captures, compares both copy paths through CPU/OpenGL, then checks
the existing Qt frame presentation. All game-like interfaces in that test are
synthetic; no map session is started.

`mnm-renderer` is a CMake library. Its `Blit` command contains source pixels,
destination-before pixels, a source rectangle, destination origin, pixel size,
and optional exact key. `GlBlitter::draw` returns native pixels and has no
captured-after input. A `QGuiApplication` must exist, and the blitter must be
created, used and destroyed on the GUI thread. It owns a separate offscreen
context and restores the caller's context around operations.

Persistent API: `create`, `destroy`, `update`, `copy`, `setPalette`, `read`,
and `present`. Surface handles are unique process-local IDs; destroyed or foreign
handles are rejected. `copy` retains both textures and performs no native upload
or readback. `update` uploads only a changed rectangle. `read` explicitly returns
native pixels. `present` resolves colors on the GPU into a reused RGBA texture
and returns a Qt image. Indexed palettes start as opaque black and support
partial updates; palette changes never alter native indices.

```cpp
mnm::render::GlBlitter renderer; // Requires QGuiApplication on the GUI thread.
auto surface = renderer.create({2, 1, {0, 1}}, {8, {}});
renderer.setPalette(surface, 0, {{255, 0, 0}, {0, 255, 0}});
QImage frame = renderer.present(surface);
renderer.destroy(surface);
```

The Qt shell links this module. Open its independent palette-cycling demo with:

```bash
./tools/run-qt-shell.sh --surface-demo
```

The demo performs copies and updates on persistent surfaces, then cycles a
palette entry every half second. It opens a viewport without launching the game.

Persistent surfaces support up to 2048x2048, with at most 64 surfaces and
16,777,216 native pixels per renderer. Formats are indexed 8-bit or masked
16/24/32-bit RGB. Copies require identical formats and distinct surfaces;
clipping/stretching/effects and conversion remain unsupported. Existing single
capture replay keeps its original 256x256 source bound. `draw` is a compatibility
wrapper that creates temporary surfaces, copies, reads and destroys them.
Presentation currently reads RGBA back to Qt for upload by its viewport;
direct shared-texture presentation remains later work.

See [implementation and validation evidence](../research/runtime/opengl-blit-replay.md)
and [capture format](../research/formats/render-draw-capture.md).
Persistent API and evidence: [surfaces and presentation](../research/runtime/opengl-persistent-surfaces.md).

## Replay ordered surface commands

The capture bridge now writes `commands-0001.bin` alongside an accepted blit.
It describes replay-owned surface creation, palettes, a copy, an original-pixel
check, presentation and destruction. The format also accepts explicit rectangular
updates and incremental palette changes. This is a bounded checkpoint session;
continuous game surface/COM lifecycle capture remains pending.

```bash
./tools/run-qt-shell.sh --commands CAPTURE_DIRECTORY/commands-0001.bin
working/build/renderer/mnm-render-commands CAPTURE_DIRECTORY/commands-0001.bin \
  --output NEW_NATIVE_FILE --preview NEW_PNG_FILE
```

Both commands replay without launching the game. The CLI emits JSON statistics
and hashes; output paths must be new. Invalid ordering, unsupported operations,
and mismatching original-pixel checks fail. Expected pixels are never GPU inputs.
See [protocol](../research/formats/render-surface-commands.md) and
[evidence and remaining hooks](../research/runtime/opengl-surface-command-replay.md).

For a bounded sequence rather than a single draw, prepare capture with
`./tools/run-qt-shell.sh --capture-history`, then replay
`history-0001.bin` using the same `--commands` option. Supported RGB copies and
full-surface CPU updates retain surface IDs across recognized COM aliases until
Release or the recorder boundary. A capture GAP or native CHECK mismatch fails
replay. See [history coverage](../research/runtime/opengl-surface-history.md);
synthetic x86 tests run with `./tools/test-render-history.py`.
