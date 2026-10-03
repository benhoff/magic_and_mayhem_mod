# DirectDraw frame bridge

A freestanding PE32 DLL captures completed primary-surface frames for the
native Qt/OpenGL viewport. Build: `./tools/build-render-bridge.py`.
Synthetic Wine-to-Qt readback: `./tools/test-render-bridge.py`.

The bridge intercepts the pinned executable's DirectDrawCreate IAT slot, follows
recognized DirectDraw and surface QueryInterface results, and forwards original
calls. Successful primary-surface Blt/BltFast/Flip calls trigger a bounded, read-only,
nonblocking surface lock and RGBA conversion into a mapped frame file. Drawing
continues in the original engine. Palette indices and 16/24/32-bit RGB masks
are supported; unsupported surfaces and lock failures skip capture.

This process-lifetime instrumentation uses shared vtable hooks and must not be
unloaded during execution. It has passed synthetic interface tests; actual game
presentation remains unvalidated. Its purpose is the first presentation stage,
not a complete DirectDraw emulation layer or reconstructed drawing engine.

See [scope and evidence](../../research/runtime/opengl-presentation.md) and
[frame format](../../research/formats/render-frame-stream.md).

With `./tools/run-qt-shell.sh --capture-draws`, the bridge also records a bounded
draw-call inventory and one small opaque/source-keyed blit, including native
source and destination-before/after pixels. Replay with
`./tools/replay-render-capture.py CAPTURE_DIRECTORY`. The shell waits for
**Launch game**; opening it alone does not run the game. See
[drawing evidence and procedure](../../research/runtime/render-drawing-inventory.md)
and [capture format](../../research/formats/render-draw-capture.md).

Each completed draw capture also exports `commands-0001.bin`, an ordered
checkpoint session for the persistent native renderer. Preview it with
`./tools/run-qt-shell.sh --commands CAPTURE_DIRECTORY/commands-0001.bin`.
No extra observer locks or COM calls are needed. These are replay-owned surface
lifetimes. The separate opt-in native history below adds writable locks and
observed final Release; indexed palette hooks are documented below. See [command format](../../research/formats/render-surface-commands.md).

`./tools/run-qt-shell.sh --capture-history` opts into a separate bounded indexed/RGB
history after the first accepted draw. It records continuing copies, full-surface
writable Lock/Unlock updates and final Release lifetimes, sharing IDs across
recognized aliases and assigning fresh IDs after object-address reuse. Unsupported
operations/gaps invalidate replay. Shared palette assignment/entry updates are supported; flip replacement remains
pending. Synthetic x86 validation: `./tools/test-render-history.py`.
See [scope and evidence](../../research/runtime/opengl-surface-history.md).

Indexed history validation: `./tools/test-render-palettes.py`. Shared palette
updates and surface reassignment emit palette records without converting native
indices; CHECK_RGBA verifies original displayed colors independently. See
[palette scope/evidence](../../research/runtime/opengl-indexed-palettes.md).
