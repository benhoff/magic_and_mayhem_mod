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
operations/gaps invalidate replay. Shared palette assignment/entry updates are supported; verified double-buffer flips are supported within seeded histories. Synthetic x86 validation: `./tools/test-render-history.py`.
See [scope and evidence](../../research/runtime/opengl-surface-history.md).

Indexed history validation: `./tools/test-render-palettes.py`. Shared palette
updates and surface reassignment emit palette records without converting native
indices; CHECK_RGBA verifies original displayed colors independently. See
[palette scope/evidence](../../research/runtime/opengl-indexed-palettes.md).

Double-buffer flip validation: `./tools/test-render-flips.py`. Front/back native
storage swaps retain logical palettes and surface identities. Native checks on
both sides and Qt presentation checks cover repeated flips and failed retries.
Longer chains, stereo/field flips and incomplete readback invalidate history.
See [flip scope/evidence](../../research/runtime/opengl-double-buffer-flips.md).

For a black startup screen, use
`./tools/run-qt-shell.sh --software-rendering --capture-history`. This opts into
Mesa software rendering for the shell and its Wine child. Startup diagnostics
now distinguish the hook stage, DirectDraw initialization and an HRESULT failure.
See [real-game startup investigation](../../research/runtime/render-startup-black-screen.md).

The loader/import integration fixture `./tools/test-render-import.py` verifies
the pinned PE layout and both adapter enumeration paths before DirectDrawCreate.
It replaces the original entry point and exits after the API calls; it does not
run the game loop. Startup status now also identifies adapter enumeration.

For a post-startup movie/surface ownership failure, retry with
`./tools/run-qt-shell.sh --software-rendering --skip-movies` and leave draw/history
capture off initially. Movie preferences change only in the disposable experiment
copy; both plaintext and encrypted preferences are handled. See the startup
investigation for evidence and limitations. This is an isolated diagnostic
workaround, not a confirmed movie decoder or surface ownership fix.

For surface-busy isolation, `./tools/run-qt-shell.sh --capture-draws --no-readback`
keeps game-call logging but disables all primary/history/snapshot readback locks.
Use the Wine window: Qt frames and pixel checkpoints are intentionally disabled.
This mode tests whether observer readback is needed to reproduce the error.
Application Lock calls are still forwarded unchanged.

`./tools/run-qt-shell.sh --capture-locks` opts into bounded game-owned Lock/Unlock
capture and automatically disables observer readback. It copies writable full
surfaces before the game's Unlock and commits only on successful Unlock. Native
snapshots go to the experiment's `lock-capture/`. Known primary RGB destinations
now produce Qt frames after successful Unlock and supported reconstructed blits.
Offscreen RGB Blt/BltFast copies also produce offline replay files. A complete opaque copy can also initialize a never-Locked destination using
observed application descriptors. Observed two-buffer RGB Flip chains now publish owned backbuffer pixels.
Complete indexed primary checkpoints now publish with observed 256-color palettes,
and application palette updates recolor their retained indices. Indexed blits,
indexed Flips and broader chains remain outstanding. See
[format and lifecycle](../../research/formats/render-game-lock-capture.md).
Run `./tools/test-render-lock-lifecycle.py` for the PE32 Wine lifecycle fixtures
and `./tools/test-render-lock-blits.py` for independent CPU/OpenGL propagation
checks. See [blit contract and evidence](../../research/runtime/opengl-game-owned-blits.md).

Disposable render installations disable CD music in staged `[SOUND]` preferences
to avoid the confirmed pre-DirectDraw `CDROM ERROR !` driver dialog. The manifest
records this edit and its hashes; plaintext/encrypted copies are both handled.
This does not change the source installation's preferences.

For reconstructed-primary publication, run `./tools/test-render-lock-blits.py --primary`;
`--unknown-primary-caps` checks rejection without returned caps provenance.
See [primary scope and evidence](../../research/runtime/opengl-game-owned-primary.md).

Run `./tools/test-render-bootstrap.py` to test 800x600 initialization without
Locking the destination, then incremental copies through Qt/OpenGL. See
[metadata provenance and evidence](../../research/runtime/opengl-primary-bootstrap.md).

Run `./tools/test-render-owned-flips.py` for two-buffer routing from application
GetAttachedSurface and descriptor observations, without observer surface calls.
See [Flip scope and evidence](../../research/runtime/opengl-owned-flips.md).

Run `./tools/test-render-owned-palettes.py` for indexed primary snapshots and
observed palette changes without observer calls. See
[palette provenance and tests](../../research/runtime/opengl-owned-indexed.md).
