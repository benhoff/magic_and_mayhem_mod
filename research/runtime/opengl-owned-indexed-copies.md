# Owned indexed blits and two-buffer Flips

Implemented 2026-10-03 after [indexed primary/palette capture](opengl-owned-indexed.md).
This extends `runtime/render/` and the existing `MNMCMD01` command format. No game
session or original artifact was used for implementation or tests.

## Native index propagation

Supported Blt/BltFast now copy 8-bit owned indices between distinct tracked
surfaces with matching native formats. The previous bounds, unscaled rectangles,
clipper rules, flags, epoch/generation checks, failed-call preservation and
complete opaque initialization requirements still apply. Source keys compare
native index values, not resolved colors; indexed keys outside 0..255 are rejected.
Two indices with equal palette RGB values remain distinct key values.

Native copies do not require observed source colors or destination colors. A
complete opaque overwrite can initialize a never-Locked indexed destination
using valid application metadata. Partial or keyed writes cannot initialize
unknown pixels. A later complete palette observation can publish retained
indices even when their earlier draw could not publish a frame.

The destination's observed palette controls publication. Source palette colors
are not copied, translated or substituted. A nested color update does not change
raw indices, so the completed draw uses the current observed destination colors.
A nested surface palette reassignment changes its generation and invalidates the
pending copy, requiring a fresh complete checkpoint or overwrite.

This is a scoped native-index copy model, tested against an independent synthetic
engine; real-driver equivalence is not established. API context: BltFast supports
source copies and source-keyed transparent copies, while SetPalette assigns the
palette used by a surface. References: [Microsoft BltFast](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-bltfast),
[Microsoft SetPalette](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-setpalette).

## Indexed Flip routing

The [observed two-buffer Flip contract](opengl-owned-flips.md) now accepts matching
indexed layouts. Only owned native pixel snapshots rotate. Palette assignments,
caps, keys and interface identities remain attached to their surfaces. The new
front uses its own current observed palette, even when front and back palettes
differ. A failed Flip leaves cache/frame state unchanged; an unsupported or
nested-invalidated successful Flip still invalidates the capture epoch.

Unknown front pixels become unknown back pixels after rotation. The first Flip
can still publish a known back image, but subsequent partial back writes cannot
establish its missing pixels. Larger chains and unsupported Flip flags remain
outside this contract. Keeping palette assignment on the surface identity is the
model’s inference from Flip’s documented memory exchange and SetPalette’s surface
assignment, not independent driver verification. Reference: [Microsoft Flip](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-flip).

## Replay evidence and bounds

Indexed `blit-N.bin` files now contain native CREATE source, CREATE destination
before, destination PALETTE, BLIT, native CHECK, PRESENT, DESTROY both, END. The
source has no palette record because it is never presented. A source palette is
not required to replay a native byte copy. Destination colors come only from a
complete observed palette at successful draw completion. If colors are unknown,
the tracker commits native pixels without writing a replay file or publishing a
frame; `blit_palette_unobserved` explains the missing file. Operation serials can
therefore have gaps. Bootstrap destination-before bytes remain synthetic zeros
whose entire extent is overwritten by the opaque draw, not driver evidence.

Successful indexed Flips with both old buffers known and complete observed front
colors additionally write `flip-N.bin`: CREATE old front with its front palette,
CREATE old back without a palette, SWAP (`11`), native CHECK new front, PRESENT
front, DESTROY both, END. Palette identity does not rotate in native OpenGL
replay. After the swap, old front bytes remain owned by the cached back and old
back bytes by the cached front, so writing this evidence needs no extra Lock or
native heap clone. An unknown old front or unknown front colors skip replay;
they do not prevent otherwise supported native rotation.

No protocol version or opcode was added. CHECK is reconstructed output, not
original-driver readback. Synthetic fixture dumps supply independent original
evidence. Replay uses the existing native integer OpenGL copy/swap/palette path.

All prior cache/Lock/blit/frame limits remain. Indexed blit payload accounting
includes palette records. Flip replay has a separate 64 MiB cumulative payload
bound; the existing 16 accepted-Flip limit also bounds its files. Indexed live
publication still stops at 16 frames per launch, including palette updates.
The original application calls remain intact when capture limits are reached.
Additional diagnostics are `blit_palette_unobserved`, `flip_recorded` and
`flip_file_failed`. Identical lifecycle records are deduplicated.

## Offline validation and confidence

Run `./tools/test-render-indexed-copies.py`. Synthetic PE32 i386 surfaces maintain
independent native backing pixels and distinct per-surface palette arrays. Source
Lock storage and palette-write input buffers are poisoned after original calls.
Palette A deliberately assigns equal colors to indices 0 and 1; index 0 alone is
transparent. Exact original arguments/results/LastError, call counts and palette
reference counts are asserted; there are no observer COM calls.

Every replay CHECK/output is compared with independent original-operation bytes
and a Python native-index oracle. Actual OpenGL command replay must reproduce
those indices and the independent destination-resolved RGBA hash. Each command
file has exactly one palette upload, belonging to the presented destination/front.
Per-operation stream counters and full RGBA snapshots are likewise compared with
original pixels/colors; Qt framebuffer readback checks final presented frames.

Cases include opaque/full initialization, BltFast, keyed incremental updates,
aliases, legacy Unlock, negative pitch, failed draws/Flips and retry, delayed or
missing palette colors, offscreen copies, rejected partial/keyed initialization,
out-of-range keys, nested color updates/reassignment, distinct-palette buffer
rotation, explicit/aliased Flip targets, unobserved chains and unknown old-front
pixels. Palette reassignment follows identity across repeated rotations.

Confidence: confirmed scoped synthetic x86 -> owned indices -> CPU/native OpenGL
copy and swap replay -> palette-resolved Qt presentation. Real-game API coverage,
driver equivalence and continuous gameplay remain unvalidated. Partial CPU Locks,
complex clipping/effects, format conversion, larger chains and uninterrupted
replacement presentation remain separate work.

Evidence: all 24 indexed copy/Flip cases passed in
`working/tests/render-indexed-copies/run-crk8oo83/report.json`. Reports record
per-operation frame counts, native GPU results, palette-resolved RGBA hashes
and the exact PE32 DLL hash.

Focused cases with distinct front/back patterns, visible incremental index
changes, delayed palettes and unknown old-front rejection passed in
`working/tests/render-indexed-copies/run-6_erb8uy/report.json`. All 22 previous
owned-palette cases passed in
`working/tests/render-owned-palettes/run-etnl1f02/report.json`, and all 14
Lock/Unlock lifecycle cases passed in
`working/tests/render-lock-lifecycle/run-0794jf7u/report.json`.

RGB chain/keyed/alias regressions passed in
`working/tests/render-lock-blits/run-t6xmtaxu/report.json`.

RGB rotation/alias/budget cases passed in
`working/tests/render-owned-flips/run-5e0hi__1/report.json`, and all 11 legacy
palette history/replay cases passed in
`working/tests/render-palettes/run-c67tq5at/report.json`. The production DLL
rebuilt as PE32 i386; hashes are in `working/build/render/manifest.json`.
