# DirectDraw frame bridge

A freestanding PE32 DLL captures completed primary-surface frames for the
native Qt/OpenGL viewport. Build: `./tools/build-render-bridge.py`.
Synthetic Wine-to-Qt readback: `./tools/test-render-bridge.py`.

The bridge intercepts the pinned executable's DirectDrawCreate IAT slot, follows
recognized DirectDraw and surface QueryInterface results, and forwards original
calls. Successful primary-surface Blt/Flip calls trigger a bounded, read-only,
nonblocking surface lock and RGBA conversion into a mapped frame file. Drawing
continues in the original engine. Palette indices and 16/24/32-bit RGB masks
are supported; unsupported surfaces and lock failures skip capture.

This process-lifetime instrumentation uses shared vtable hooks and must not be
unloaded during execution. It has passed synthetic interface tests; actual game
presentation remains unvalidated. Its purpose is the first presentation stage,
not a complete DirectDraw emulation layer or reconstructed drawing engine.

See [scope and evidence](../../research/runtime/opengl-presentation.md) and
[frame format](../../research/formats/render-frame-stream.md).
