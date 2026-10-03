# Persistent OpenGL surfaces and palette presentation

Implemented 2026-10-03 after [single-blit replay](opengl-blit-replay.md). This is
new renderer behavior under `renderer/`; no new original-engine function has
been reconstructed by this step.

## Surface ownership and ordered commands

`GlBlitter` now retains native `GL_R32UI` textures by opaque `SurfaceId` handles.
IDs are globally monotonic within the host process and never reused; each
renderer validates ownership. They are unrelated to game pointers or DirectDraw
interface addresses. Destroyed/foreign handles fail before drawing. Limits are
64 live surfaces and 16,777,216 native pixels, each surface at most 2048x2048.
The OpenGL texture-size limit is also checked.

`create` validates native pixels and format, uploads once and records dimensions.
`update` uploads an in-bounds native rectangle with `glTexSubImage2D`. `copy`
submits the existing integer copy shader using persistent source/destination
textures; it performs neither a native upload nor readback. Calls are ordered
on the renderer's single Qt GUI-thread context. Copies require distinct surfaces
with equal native bit depths and RGB masks. Indexed surfaces may have different
palettes because copy semantics preserve indices.

`read` explicitly returns full native pixels. `destroy` deletes all textures
associated with that surface and releases its pixel budget. Renderer destruction
cleans remaining surfaces. The existing `draw(Blit)` API remains a convenience
wrapper using temporary surfaces, preserving the bounded capture/replay contract.
The shared framebuffer, shader programs and vertex array are retained between
commands. `RenderStats` reports native uploads, copies, palette updates, native
readbacks, presentations and current live surface/pixel counts.

## Palette and RGB presentation

Each indexed surface owns a 256-entry `GL_RGBA8UI` palette texture, initially
opaque black. `setPalette` updates any nonempty, in-bounds range. Its API takes
RGB colors; output alpha is always 255. Capture `PALETTEENTRY` flags are ignored
as flags, never interpreted as alpha. Updates affect the next presentation
without rewriting native indices. Duplicate RGB palette entries remain distinct
indices for exact source-key tests.

`present` allocates a surface's RGBA integer texture on first use and reuses it.
The presentation shader reads native pixels and either fetches the palette entry
or extracts contiguous RGB mask fields. Masked channels use integer
`channel * 255 / maximum`, matching the existing CPU conversion's truncation.
Masks must be nonzero, disjoint, within the native bit depth, contiguous, and
at most eight bits per channel. Unused native bits remain intact in the native
surface; display output is opaque RGBA. Indexed formats use zero masks.

The presentation target is `GL_RGBA8UI`, read with `GL_RGBA_INTEGER`/
`GL_UNSIGNED_BYTE` into `QImage::Format_RGBA8888`. Upload, drawing and readback
use the same logical row-zero convention established in single-blit replay.
Every operation restores the caller's context. The Qt viewport consumes this
image through its existing texture presentation path.

This still entails RGBA readback and Qt texture upload per displayed image.
Shared-texture presentation and live engine command routing remain unimplemented.
It is not yet a complete DirectDraw renderer or a performance equivalence claim.

## Capture replay and visible Qt demo

`decodeCaptureData` extracts the validated command, native format and destination
palette. Captured-after pixels are still comparison evidence only. Replay creates
source and destination surfaces, submits a copy, reads the destination, and
optionally presents it. `mnm-render-replay ... --preview FILE.png` writes a new
PNG and reports `presentation_rgba_sha256` plus `surface_stats`. The Python
front end requests this preview for OpenGL replay and independently compares its
raw RGBA hash with CPU palette/RGB conversion. Native comparison remains separate.

```bash
./tools/replay-render-capture.py CAPTURE_DIRECTORY --backend opengl --headless
./tools/run-qt-shell.sh --surface-demo
```

The demo creates indexed surfaces, performs updates/copies, destroys the source,
and presents the retained destination. It cycles one palette entry every 500ms.
It launches no game. `--surface-test` runs the same setup and verifies the Qt
framebuffer's colors, orientation and letterboxing before exiting.

## Validation and confidence

The original 196 OpenGL copy cases still match their CPU reference. The new
persistent-surface test checks 192 ordered updates/copies across indexed 8-bit,
RGB565, RGB24 and RGB32. It compares both retained surfaces after each command,
verifies copy calls do not increment upload/readback counters, and checks GPU
presentation against a separate CPU converter. Additional cases cover palette
cycling, duplicate RGB colors, destination palette ownership, RGB555/BGR masks,
unused native bits, destroyed/foreign handles, surface-count limits, malformed
commands and unchanged destination contents after rejected commands.

Capture integration tests also compare GPU RGBA hashes with the CPU conversion,
check PNG output and exercise palette flags that must remain opaque. Existing
poisoned-output, malformed-capture and missing-backend checks remain intact.
The Qt surface display test passes alongside existing embedding/presentation
checks. All graphics tests run on Mesa llvmpipe, OpenGL 4.6 core, while requesting
the OpenGL 3.3 API.

All five renderer CTests pass. Capture integration evidence is at
`working/tests/render-opengl/run-g5ttl8tw/report.json` (ten valid captures,
21 malformed inputs rejected). All five Qt shell GUI/startup/help tests pass,
including the new persistent-surface display test.

Fresh synthetic PE32 Wine capture pipeline:
`working/tests/render/run-cng7033o/report.json`. Both captured copy paths match
native CPU/OpenGL output and GPU-presented RGB565 colors. Replay reports are
`working/experiments/render-replay/run-brrp17rp/report.json` and
`working/experiments/render-replay/run-_c_l3qom/report.json`.
Confidence is high for these synthetic contracts on Mesa software rendering.
Real game captures, physical GPU drivers and runtime surface mapping remain
pending. Original artifact files were not consumed or modified by this work.

The next integration chunk is an ordered surface-command stream from the PE32
bridge, including creation/destruction, native pixel updates, copy operations
and palette changes. It must map interface aliases/lifetimes to stable stream
IDs and report unsupported/missed drawing operations before live replacement.
