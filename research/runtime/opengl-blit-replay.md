# Native integer OpenGL blit replay

Implemented 2026-10-03 after the [drawing inventory/capture
step](render-drawing-inventory.md). This is new replacement-renderer code under
`renderer/`, not a reconstruction of a new original function. The original
game's renderer and the PE32 capture hook remain unchanged by this step.

## Implemented drawing contract

`GlBlitter::draw` takes source pixels, destination-before pixels, a source
rectangle, destination origin, native bit depth and optional exact source key.
It uploads native unsigned pixel values to separate `GL_R32UI` textures, binds
the destination as an integer framebuffer attachment, and draws a full-screen
triangle restricted to the destination rectangle by the scissor box.

The OpenGL 3.3 fragment shader reads source texels with integer coordinates,
discards pixels matching an enabled source key, and writes every other native
pixel value. Blending, dithering, depth/stencil tests and sRGB conversion are
disabled. The original destination upload preserves transparent pixels and
the untouched border. Source/destination textures are distinct, avoiding a
read/write feedback dependency. Source keys use an unsigned uniform, including
values above `INT32_MAX`.

Logical top row is texture row zero for upload, command coordinates and native
readback. No row reversal or RGB conversion occurs in the blitter. This keeps
palette indices, duplicate-color indices and all 32 native bits intact. The
existing preview converter applies palette/RGB interpretation afterward.

Bounds match `MNMBLT01`: source up to 256x256, destination up to 2048x2048,
8/16/24/32-bit native values, unscaled in-bounds rectangles and optional single
exact source key. Blt and BltFast normalize to this same offline command;
their scheduling flags have no offline drawing effect. Unsupported input is
rejected before graphics submission. Color-space keys, clipping, effects,
format conversion, alpha and self-copy remain outside the contract.

Qt owns a dedicated `QOpenGLContext` and `QOffscreenSurface`. The surface uses
the created context's actual format. Initialization and destruction occur on
the GUI thread. The caller's previous context is restored after construction,
drawing and destruction; no existing Qt viewport's GL state is repurposed.
Context/FBO/shader failures produce an error and no successful comparison.

## Capture and comparison boundaries

`renderer/capture.cpp` independently validates the existing binary capture
format and extracts only source and destination-before pixels for the command.
The captured-after payload and palettes are skipped as renderer inputs. The
CLI writes new native output bytes and reports the OpenGL implementation and
input/output hashes. It refuses to overwrite existing output files.

`tools/replay-render-capture.py --backend opengl` runs the independent Python
CPU reference and the native shader renderer. Its report includes:

- `cpu_comparison`: CPU result versus captured output.
- `comparison`: OpenGL result versus captured output.
- `opengl_vs_cpu`: OpenGL result versus CPU result.
- `opengl`: driver/vendor/version and capture/output/renderer-binary hashes.

The capture hash must agree across both reads. Every destination pixel is
compared, including pixels outside the copied rectangle. `replayed.ppm` is the
OpenGL result, `cpu.ppm` the reference, and `captured.ppm` the original recorded
output. `opengl-native.bin` preserves exact native bytes; previews are not used
to decide equality. A missing/failed graphics backend returns inconclusive,
even when the CPU reference matches. No game is launched for replay.

## Validation and confidence

The native executable builds as Linux x86-64 ELF on this host; the capture DLL
remains Windows PE32 i386. No pointer sharing or ABI matching is needed between
them: they exchange the explicit little-endian capture file.

All four renderer CTests passed. `render-blit-test` checked 196 shader draws
against a separate C++ copy reference: all native bit sizes, unsigned high-bit
keys, opaque/keyed/all-transparent operations, varied subrectangles and edge
placements, maximum-size source/destination, one-pixel surfaces, repeated use
and drawing after rejected commands. It also checked preservation of a caller's
context and viewport state.

Capture integration evidence:
`working/tests/render-opengl/run-ilgw3k16/report.json`. Nine valid files matched
the Python reference; 21 malformed files were rejected consistently by both
parsers. Tests deliberately poisoned captured-after pixels: rendering still
matched the CPU reference, while the comparison correctly failed against the
recorded output. Existing-file refusal and unavailable-backend handling passed.

Fresh synthetic x86 Wine producer evidence:
`working/tests/render/run-rin2peem/report.json`. Both opaque Blt and source-keyed
BltFast captures matched the CPU and OpenGL results exactly. Reports:
`working/experiments/render-replay/run-uh6363t1/report.json` (keyed) and
`working/experiments/render-replay/run-achriwyz/report.json` (opaque).
The existing mapped-frame-to-Qt display readback also passed.

These tests used **Mesa llvmpipe**, reporting OpenGL 4.6 core / Mesa
26.2.1-arch1.1, with the renderer requesting desktop OpenGL 3.3. Confidence is
high for the tested shader operation and file contracts on that implementation.
Physical GPU drivers and real game captures remain unvalidated. Synthetic
DirectDraw interfaces are not evidence of every original engine drawing path.

## Next implementation boundary

The following [persistent-surface milestone](opengl-persistent-surfaces.md)
now retains renderer-owned textures, updates rectangular pixel regions, resolves
palettes/RGB masks on the GPU and presents them in a standalone Qt demo. The
single-command API described above remains a convenience wrapper for capture
comparison. The lifecycle described below applies to that wrapper.

This component uploads both input surfaces and reads the entire destination
back for each call. That deliberately simple lifecycle provides comparable
output, not game-speed rendering. The next independent chunk is persistent
renderer-owned surfaces and palette-aware presentation, with ordered copy/update
commands tested against the same native-pixel reference. Routing live engine
drawing into those surfaces requires surface identity/lifetime tracking, CPU
pixel-update capture and the pending real-game validation.

Qt references:
[QOffscreenSurface](https://doc.qt.io/qt-6/qoffscreensurface.html),
[QOpenGLContext](https://doc.qt.io/qt-6/qopenglcontext.html).
