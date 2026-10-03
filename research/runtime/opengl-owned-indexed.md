# Owned indexed pixels and observed palette updates

Implemented 2026-10-03 in `runtime/render/`, separate from the engine
reconstruction and the older observer-readback palette history. This chunk uses
only application calls and owned memory; no game launch or original artifact
was needed for implementation or tests.

## Pixel and palette provenance

Complete writable application Lock/Unlock checkpoints now retain 8-bit native
indices in the bounded surface cache. Copying still occurs before original
Unlock; only successful Unlock commits it. Negative pitch and padding are
normalized, and no original pixel pointer is read afterward. The existing
`MNMLOCK1` file contains indices only, with no new header or appended palette.
Supported indexed descriptors use pixel-format flags `0x60`, 8 bits and zero
RGB masks. Application metadata can preserve this layout but does not create
unknown initial pixels.

Surface SetPalette (slot 31) and successful application GetPalette (slot 20)
establish palette assignment. A failed call preserves prior assignment; successful
SetPalette(NULL) removes it. Palette interfaces are hooked without adding COM
queries, references or releases. Only successful application palette-IID
QueryInterface results establish aliases. Conflicting independently tracked
aliases discard palette provenance and require fresh observations.

Palette colors come from successful observed CreatePalette (factory slot 5),
GetEntries (palette slot 4) or SetEntries (slot 6). CreatePalette and SetEntries
inputs are copied before the original call; GetEntries output is copied after
success. Pending entry operations validate the palette generation and capture
epoch. A nested mutation rejects the pending update rather than merging stale
observations. Original calls, arguments, HRESULT, LastError and reference counts
are preserved.

Entry interpretation requires observed palette capabilities, from CreatePalette
flags or successful application GetCaps (palette slot 3). This contract requires
8BIT plus ALLOW256 (`0x44`), optionally PRIMARYSURFACE (`0x10`), and no other caps.
GetEntries/SetEntries require zero flags and a nonempty in-bounds range. Palette
entries are four bytes of red, green, blue and flags. Alpha output is always 255;
the fourth entry byte is retained but is not treated as alpha. Unsupported alpha,
8BITENTRIES, reserved-endpoint or smaller palettes are rejected.

CreatePalette initializes the palette from its supplied color array. The API also
supports one-byte entries for certain indexed palettes, so capabilities must be
known before assuming four-byte entries. SetEntries changes colors immediately.
References: [Microsoft CreatePalette](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdraw7-createpalette),
[Microsoft GetEntries](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawpalette-getentries),
[Microsoft SetEntries](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawpalette-setentries).

Each palette has a 256-bit coverage map. Partial observations accumulate, but
all 256 colors must be known before publication, even if a particular image uses
fewer indices. Failed entry calls preserve prior colors. Unsupported successful
entry calls invalidate coverage. Successful Initialize invalidates caps and
colors, requiring fresh observations. Final palette Release invalidates the
capture epoch, preventing address reuse from inheriting old colors or pixels.

## Presentation and limits

A complete owned indexed primary with an observed assignment and complete
supported palette publishes through the existing RGBA stream and Qt/OpenGL
viewport. Palette changes recolor retained indices without another surface Lock
or native pixel read. Successful palette reassignment can likewise recolor the
same indices; updates to the former palette do not publish a new frame.
Offscreen surfaces do not publish. Repeated diagnostic records are deduplicated.

The palette cache contains at most 32 records and 16 observed interfaces per
record. Static palette storage is bounded; the existing 32 surface slots and
64 MiB retained/pending native-pixel budget also apply to indexed checkpoints.
Indexed publication has a separate 16-frame bound per launch. Exhaustion skips
publication while preserving application calls. This is bounded evidence
capture, not continuous replacement rendering.

The subsequent [indexed copy/Flip chunk](opengl-owned-indexed-copies.md) extends
native propagation and destination palette resolution, and emits bounded
indexed copy/swap replay files. Partial CPU Locks, complex operations and
continuous presentation remain separate work.

## Offline validation and confidence

Run `./tools/test-render-owned-palettes.py`. Generated PE32 i386 surfaces retain
independent original native indices and palette arrays. Original Unlock poisons
exposed Lock storage. Original successful palette creation and writes poison
their caller input buffers after consumption, verifying that capture copied
inputs before forwarding. The fixture asserts exact API call counts, original
arguments/results/LastError and reference counts; no observer calls are allowed.

Per-operation stream snapshots are compared byte for byte with independently
resolved original indices/colors, including unchanged frames after failed or
rejected capture. The actual Qt viewport checks final colors under Xvfb/Mesa.
Cases cover full and partial entry observations, creation, aliases, reassignment,
GetPalette assignment, detach, failed writes/retry, unsupported flags/caps,
missing capabilities, failed reads/assignments, nested mutation, Initialize,
offscreen nonpublication, indexed descriptor queries, frame limits, palette setup
before Lock, final Release/reseeding, negative pitch and the legacy Unlock ABI.

Confidence: confirmed scoped synthetic x86 -> owned indices and palette state ->
RGBA stream -> Qt/OpenGL framebuffer. Real-game palette coverage, unobserved
aliases/changes, driver equivalence and continuous gameplay remain unvalidated.

Evidence: all 22 indexed cases passed in
`working/tests/render-owned-palettes/run-xd7245kn/report.json`. The report records
per-case frame counts, Qt readback, diagnostics and the exact PE32 DLL hash.

All 14 existing Lock/Unlock lifecycle fixtures passed in
`working/tests/render-lock-lifecycle/run-9gq27tzj/report.json`.

All 11 legacy indexed palette history/replay cases passed in
`working/tests/render-palettes/run-_djjgmvo/report.json`.

RGB rotation/alias/budget regressions passed in
`working/tests/render-owned-flips/run-5afp7j4n/report.json`. Primary initialization,
creation metadata and conflicting active-Lock regressions passed in
`working/tests/render-bootstrap/run-gl1apt42/report.json`. The production DLL
rebuilt as PE32 i386; hashes are in `working/build/render/manifest.json`.
