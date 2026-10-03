# Native OpenGL renderer

New rendering code, separate from original-engine reconstructions, the PE32
capture hooks in `runtime/render/`, and the Qt shell in `apps/qt-shell/`.
The first implemented operation is an unscaled opaque or exact source-keyed
rectangle copy. It executes a fragment shader on native integer pixels and
compares readback with a captured DirectDraw operation and the CPU reference.
It is an offline replacement-renderer component; game drawing is not routed
through it yet.

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
hashes are recorded. `--gl-executable PATH` reuses an explicitly selected build.
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

The four CTests cover CLI help, the existing CPU reference tests, 196 OpenGL
draws, and capture/CLI integration. The integrated Wine bridge test generates
fresh x86 captures, compares both copy paths through CPU/OpenGL, then checks
the existing Qt frame presentation. All game-like interfaces in that test are
synthetic; no map session is started.

`mnm-renderer` is a CMake library. Its `Blit` command contains source pixels,
destination-before pixels, a source rectangle, destination origin, pixel size,
and optional exact key. `GlBlitter::draw` returns native pixels and has no
captured-after input. A `QGuiApplication` must exist, and the blitter must be
created, used and destroyed on the GUI thread. It owns a separate offscreen
context and restores the caller's context around operations.

Current limits match the capture subset: source at most 256x256, destination
at most 2048x2048, 8/16/24/32-bit native pixels, no clipping/stretching/effects
or conversion. Textures are uploaded and read back for each command; this is
a correctness baseline, not the final live rendering pipeline. Palette indices
stay as indices during drawing. Palettes/RGB masks are used by the Python
preview generator, not by the copy shader.

See [implementation and validation evidence](../research/runtime/opengl-blit-replay.md)
and [capture format](../research/formats/render-draw-capture.md).
