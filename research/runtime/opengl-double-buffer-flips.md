# Bounded double-buffer flip capture and replay

## Confirmed scope

`runtime/render/flip_history.h` extends the already seeded indexed/RGB history.
A successful supported Flip emits opcode 11 SWAP, checks both resulting native
surfaces against original readbacks, and presents the primary/front buffer.
Indexed surfaces also emit independent RGBA checks for both logical surfaces.
Expected pixels/colors are comparisons, never GPU output uploads.

DirectDraw exchanges the front/back surface memory associations. The native
renderer mirrors that by swapping the two integer texture handles; it leaves
resource IDs, palettes and cached presentation textures with their logical
surfaces. Presentation always regenerates colors from current native storage.
This follows Microsoft's [Flip contract](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-flip).
Scheduling is not benchmarked or reconstructed.

## Guarded discovery and forwarding

The front must report PRIMARYSURFACE, FRONTBUFFER, FLIP and COMPLEX; its descriptor
must explicitly report exactly one backbuffer. GetAttachedSurface(BACKBUFFER),
called through the saved original slot 12, discovers that buffer on every flip.
The returned interface is intercepted using the receiver's surface ABI. Its
canonical identity must differ from the front; dimensions, bits and masks must
match. Both surfaces must be unlocked. Modern extended caps must be zero.
Aliases resolve through original QueryInterface with balanced observer references.

Default target NULL and an alias of the verified backbuffer are accepted. WAIT,
NOVSYNC and DONOTWAIT are supported individually or where not contradictory.
Stereo/field/interval flags, longer chains, mismatched layouts and unknown target
identities are outside this scope. Unsupported successful calls emit GAP; the
original API still receives its exact arguments and original entry LastError.
Original HRESULT and final LastError are restored after instrumentation.

Observer locks finish before the application Flip. One temporary attachment
reference keeps the observed backbuffer alive through the call and is released
before returning; no reference persists across application calls. No surface
pointer or underlying pixel address is assumed stable across calls or launches.

Before Flip, previously untracked members get native CREATE checkpoints; tracked
members get CHECK instead of reuploads. A failed Flip emits no SWAP or PRESENT.
Successful flips obtain nonblocking post-call snapshots; failed readback produces
GAP 6. Unexpected pixel rotation produces an offline CHECK mismatch rather than
being repaired by uploads. The existing 16-operation/240-record/64-MiB producer
bounds remain in force.

## Validation and confidence

Run `./tools/test-render-flips.py` for generated PE32 i386 fixtures under Wine,
then independent Python native-pixel reconstruction, OpenGL replay and Qt viewport
readback. Successful cases cover Surface2/Surface4 unlock ABIs, three successive
flips with backbuffer writes, target aliases, balanced QI/attachment references,
and a failed Flip followed by retry. Rejections cover a longer chain, stereo,
changed masks, post-call lock failure and an original implementation that reports
success without exchanging pixels. Both original return values and LastError
forwarding are asserted by the fixture.

`tests/test-render-commands.py` separately verifies repeated swaps and both native
and RGBA checks for 8/16/24/32-bit surfaces. Indexed surfaces have different
palettes, proving palettes stay with logical IDs. Malformed swaps cover identical,
unknown/destroyed IDs, short/extra payloads and mismatched dimensions/formats.
Existing RGB history, palette history, frame bridge, renderer and Qt tests remain
regression requirements. The flip evidence is
`working/tests/render-flips/run-ql9ndu7q/report.json`: nine fixtures, four accepted
and five rejected. The command tests passed eight sessions and 35 malformed
streams; all six renderer and seven Qt shell regression CTests passed.
Regression evidence also includes RGB histories at
`working/tests/render-history/run-t28xh969/report.json`, palette histories at
`working/tests/render-palettes/run-1_zx2apv/report.json`, and the frame bridge at
`working/tests/render/run-ryhyi68x/report.json`. The production PE32 i386 bridge
built successfully with SHA-256
`4d7c41f9a75dc906fa898b78181a9e19dc7df3c69b52089773518424c6f3e6d5`.

Confidence: high for the scoped synthetic behavior, unvalidated in real gameplay.
The indexed flip producer uses existing tested palette/readback logic, but the
integrated flip fixtures themselves use RGB565; indexed flip capture still needs
real-game or dedicated indexed-chain evidence.

## Ready capture and remaining work

Prepare `./tools/run-qt-shell.sh --capture-history`; it waits for **Launch game**.
Load a map, collect a history, then open
`./tools/run-qt-shell.sh --commands CAPTURE_DIRECTORY/history-0001.bin`.
No game was automatically launched for this chunk.

History still starts at an eligible draw, not an initial standalone Flip. This
is a bounded offline reconstruction, not live routing of the game's draw commands.
Actual game descriptor/caps coverage and pixel checks, longer-chain rotation,
additional draw operations and complete frame boundaries remain to validate.
