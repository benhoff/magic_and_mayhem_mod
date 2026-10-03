# DirectDraw to Qt/OpenGL presentation

Implemented 2026-10-03 as the first rendering migration stage. Qt displays
finished engine frames through an OpenGL texture; the original engine still
performs drawing through DirectDraw. This is not a complete renderer rewrite.
Input continues in the separate Wine window. No actual game was launched during
implementation; live-map presentation and input integration remain pending.

## Confirmed interfaces and evidence

Static inspection of the pinned no-CD working executable (SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`)
shows DirectDrawCreate at IAT `0x005c5014`, thunk `0x0059755a`, and calls in
`0x0058a9b0` / `0x0058aa90`. QueryInterface uses GUID storage `0x005c7960`
(IDirectDraw2) and `0x005c7970` (IDirectDraw4). A 124-byte surface descriptor
is used near `0x0058f1e6`. Confidence is high for these static API observations;
they do not identify every in-game presentation path. Existing initial evidence
is in [hook candidates](hook-candidates.md).

`runtime/render/bridge.c` intercepts the guarded DirectDrawCreate import and
recognizes DirectDraw 1/2/4/7 and surface 1/2/4/7 interfaces. Shared vtable hooks
forward QueryInterface, CreateSurface, Blt, BltFast and Flip. Original return values and
last-error state are retained. A successfully presented primary surface triggers
GetSurfaceDesc and a nonblocking, read-only Lock with NOSYSLOCK. The bridge
converts valid 8-bit palettes or masked 16/24/32-bit RGB into opaque RGBA and
always Unlocks successful locks. Unreadable/unsupported layouts skip capture.
Only the first 33 surface methods / seven Draw methods are copied as originals;
the remainder of each vtable stays untouched. Hook records use vtable identity,
not unstable object addresses. Bounds are 32 tables and 2048x2048 frames.

Capture serializes with an atomic busy flag and production attempts are limited
to roughly 60 per second (minimum 16ms between attempts), without changing the
engine's own drawing rate. A rejected or busy surface does not replace the
original API result. The shared frame is published through the documented
[seqlock stream](../formats/render-frame-stream.md). Qt drops old frames and
uploads accepted RGBA data to an OpenGL 3.3 texture with nearest filtering and
aspect-preserving letterboxing. The native output remains present for validation.

The bridge is a process-lifetime import, not safely unloadable instrumentation.
It patches shared interface vtables inside its process. It assumes the pinned
image is loaded at `0x00400000`; mismatched image base/thunk skips interception.
It does not emulate DirectDraw or capture separate hardware-overlay planes.
Capturing and converting a primary surface can add CPU/GPU readback cost.
Actual engine formats, lock success rates and performance need a map session.

## Tests and confidence

Confirmed synthetic producer-to-consumer pipeline:
`working/tests/render/run-o7jradus/`. Its Wine harness invokes Blt and Flip on
a supplied PE32 surface, confirms original HRESULT / last-error values, verifies
lock/unlock pairing and published RGB565-to-RGBA bytes, then displays the exact
mapped bytes in Qt/OpenGL and checks framebuffer readback. Its report labels
`synthetic_wine_opengl`. Confidence is high for this tested synthetic contract,
not original-engine compatibility. The selftest disables production throttling
so both back-to-back hook calls are exercised.

All six Qt CTest cases pass. Qt CTest covers palette/RGB565/RGB24/RGB32 conversion, negative pitch, row
padding, malformed masks, dimensions and stream headers/sequence/stride,
headless shell startup, external native-window embedding, and known-pixel
OpenGL readback with correct orientation and letterboxing. A context-cleanup
crash found during initial readback was fixed by disconnecting the context's
cleanup callback before widget destruction; the integrated test then passed.

Staging uses the full executable hash before adding a `.mnmgl` data section /
RenderAnchor import in a fresh copy. It records EXE / DLL hashes and build
provenance, and the launcher adapter verifies both hashes before Wine starts.
No source-game code sections or original artifacts are modified. A staging-only
run was prepared without launching: `working/experiments/opengl-render/run-ele6k52e/`.
The default UI creates a fresh staging run when Launch game is clicked.

## Next validation and migration stages

Run `./tools/run-qt-shell.sh`, click Launch game, load a map and compare the Qt
viewport with the separate Wine output. Check colors, palettes, terrain, units,
UI, effects and map transitions. Inspect stream status when no frames arrive;
that may indicate primary-surface locking or a presentation path outside the
currently intercepted methods. Retain the original output until these checks
are complete.

The next implementation adds a [static drawing inventory and bounded native-pixel
blit capture/replay](render-drawing-inventory.md), enabled with `--capture-draws`.
It validates a small copy operation before moving drawing into OpenGL.

Then implement input forwarding/focus/cursor mapping and verify it before
hiding the original window. Replacing engine drawing requires separately
reconstructing surface composition, sprite/terrain formats, transparency,
palette updates, effects and animation; the current frame bridge provides a
visual reference for that work. None of those stages is claimed complete here.

Primary references: Microsoft's
[DirectDraw surface interface](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nn-ddraw-idirectdrawsurface7),
[surface Lock](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-lock),
and Qt's [QOpenGLWidget](https://doc.qt.io/qt-6/qopenglwidget.html).
