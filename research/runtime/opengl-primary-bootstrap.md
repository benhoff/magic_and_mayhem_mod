# Complete RGB initialization from application descriptors

Implemented 2026-10-03 after [primary presentation](opengl-game-owned-primary.md).
This extends the bounded replacement-renderer capture under `runtime/render/`.
No game session or original artifact was used for implementation or testing.

## Metadata provenance

The tracker observes the application's successful GetSurfaceDesc calls (surface
vtable slot 22), and preserves the input descriptor before forwarding a successful
CreateSurface call. It issues no additional COM calls, Locks or descriptor queries.
The original arguments, call counts, HRESULT and LastError are preserved.
Metadata does not expose pixels: descriptor `lpSurface` is never read here.

Accepted descriptors have the correct 108/124-byte interface layout, explicitly
valid CAPS, WIDTH, HEIGHT and PIXELFORMAT flags, a 32-byte pixel-format structure,
RGB-only flags, 16/24/32-bit pixels and nonoverlapping supported channel masks.
Both dimensions must be between 1 and 2048. Extended caps must be zero in modern
descriptors. Primary identity requires the valid caps field and PRIMARYSURFACE
bit; other valid RGB descriptors establish offscreen shape only. Application
QueryInterface relationships remain the only source of interface aliasing.

GetSurfaceDesc reports current surface state and descriptor flags identify valid
members; references: [Microsoft GetSurfaceDesc](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-getsurfacedesc),
[Microsoft DDSURFACEDESC2](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/ns-ddraw-ddsurfacedesc2).
The stricter RGB/size limits above are this implementation's capture contract.
CreateSurface metadata is accepted only for explicitly supplied valid members;
implicit primary dimensions/format remain unknown until observed elsewhere.

A failed descriptor call leaves prior provenance unchanged. An unsupported
successful descriptor invalidates it. A successful descriptor that changes the
known layout invalidates retained pixels and active Lock checkpoints, including
a contradiction of an active Lock before its first successful Unlock. Every
accepted metadata observation advances the surface generation, so nested calls
cannot silently change an in-flight blit's interpretation.

## Complete initialization

An RGB destination without owned pixels can now be initialized by a supported,
successful opaque Blt/BltFast covering its entire known extent. The source must
already have a complete owned checkpoint. Source and destination bits/masks must
match, and the copy must be unscaled, in bounds and between distinct identities.
Blt still requires observed absence of a destination clipper. No source key,
effects or partial destination coverage is accepted for initialization.

The tracker allocates a temporary zero destination-before buffer, then overwrites
every pixel after the original call succeeds and generations/epoch still match.
This synthetic zero buffer is only a replay starting state. It is **not evidence
of original destination pixels** and is never published before the full copy.
Failed or invalidated calls produce no initialized pixels or new frame. Once
complete, the destination supports the existing incremental and keyed copy rules.
Supported initialized primaries publish through the existing owned-pixel RGBA
stream and Qt/OpenGL viewport; offscreen destinations produce replay evidence only.

The `MNMCMD01` format is unchanged. Bootstrap files contain synthetic zero CREATE
destination-before bytes followed by an opaque BLIT overwriting the whole surface.
CHECK remains reconstructed output, not original-driver readback. New diagnostic
reasons are documented in [the capture format](../formats/render-game-lock-capture.md).

Source dimensions now also permit up to 2048x2048. The existing 32 surface slots,
64 MiB retained/pending pixels, 16 Lock snapshots, 16 blit command files and
cumulative file budgets still bound capture. Exhaustion stops capture work and
leaves application calls intact. This is not continuous replacement rendering.

## Offline evidence

Run `./tools/test-render-bootstrap.py`. Its synthetic PE32 i386 engine maintains
independent native backing pixels. The 800x600 destination is never Locked and
starts poisoned. Source Locks expose padded, optionally negative-pitch storage,
which is poisoned during original Unlock. Only source/sprite Locks and explicitly
requested application descriptor calls are allowed; call counters enforce this.

Accepted full initialization and subsequent sprite updates are compared byte for
byte against independent original-operation output, a Python native-pixel oracle,
OpenGL command replay and per-operation RGBA stream snapshots. The actual Qt
viewport checks the resulting final frame using GPU framebuffer readback under
Xvfb/Mesa. Rejections must produce neither replay files nor primary frames.

Eighteen fixtures passed in
`working/tests/render-bootstrap/run-8ornn8k9/report.json`: opaque Blt, BltFast,
legacy interface, negative pitch, RGB24/RGB32, observed alias, creation metadata,
failed draw/retry, nested descriptor invalidation, offscreen destination, partial
or keyed initialization, missing caps/dimensions/format flags, invalid masks and
failed descriptor calls. The additional active-Lock descriptor contradiction
passed with full-copy and nested-description regressions in
`working/tests/render-bootstrap/run-a88ea9fd/report.json` (19 distinct cases).
All 14 existing Lock/Unlock lifecycle fixtures passed in
`working/tests/render-lock-lifecycle/run-hdatvbnr/report.json`. Existing primary
chain, destination-alias and reseeding cases passed, including Qt readback, in
`working/tests/render-lock-blits/run-qobontyr/report.json`.

The legacy Wine-to-Qt bridge regression passed in
`working/tests/render/run-pgj_n460/report.json`. The production DLL rebuilt as
PE32 i386; source and binary hashes are in `working/build/render/manifest.json`.

Confidence: confirmed scoped synthetic x86 -> owned RGB pixels -> OpenGL replay
and Qt presentation. Real-game API coverage and driver equivalence remain
unvalidated. The subsequent [Flip chunk](opengl-owned-flips.md) handles observed two-buffer
RGB rotation. Indexed palette propagation, implicit or unobserved descriptors,
broader chains and continuous frame boundaries remain separate work.
