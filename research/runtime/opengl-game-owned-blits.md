# Blit propagation from game-owned Lock/Unlock checkpoints

Implemented 2026-10-03. This is replacement-renderer instrumentation under
`runtime/render/`, separate from the original engine reconstruction. No game
session was launched to implement or test this chunk.

## Contract and provenance

`--capture-locks` still copies full writable application locks before the original
Unlock and commits them only when that Unlock succeeds. It now retains accepted
RGB checkpoints in a bounded surface cache. A successful observed writable Lock
invalidates that surface's previous pixels immediately; failed Unlock cannot
publish the new copy. Indexed snapshots remain on disk but do not seed this RGB
propagation cache. Accepted native rows are tightly packed and independent of the
original pixel pointer, including padding and negative-pitch normalization.

A supported Blt/BltFast clones cached source and destination pixels before the
original call, without holding the tracker guard across the call. After success,
the tracker validates both generations and its capture epoch, performs an
independent native-pixel copy, and retains the new destination. This supports
chains of copies and source-key transparency while preserving untouched borders.
A failed original call leaves the destination checkpoint unchanged and frees the
pending copies. Original arguments, call counts, HRESULT and LastError are
preserved; no observer Lock, Unlock, GetColorKey, GetClipper or QueryInterface is
issued. Application QueryInterface results establish interface aliases.

Supported copies are in-bounds, unscaled, distinct tracked RGB identities with
identical bits and masks. Source and destination dimensions are at most 2048x2048 after the
[primary bootstrap extension](opengl-primary-bootstrap.md). Blt accepts WAIT (`0x01000000`) and KEYSRC (`0x00008000`);
BltFast accepts WAIT (`0x10`) and SRCCOLORKEY (`1`), with optional exact source keys.
Source keys must come from a successful observed SetColorKey(SRCBLT) call or a
successful application Lock/GetSurfaceDesc descriptor with DDSD_CKSRCBLT,
with matching low/high native values. Descriptor fields without that flag are
ignored; descriptor key ranges remove exact-key provenance. Failed property changes preserve old values;
key removal or unsupported ranges remove key provenance.

Blt requires observed knowledge that the destination has no clipper: successful
CreateSurface initializes that knowledge, and successful SetClipper updates it.
BltFast cannot clip and an attached clipper makes the real API fail, so successful
supported BltFast is sufficient for this boundary. API references:
[Microsoft BltFast](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-bltfast),
[Microsoft SetColorKey](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-setcolorkey).

## Conservative invalidation and bounds

### Colour fills and recovery (2026-10-04)

Application Blt with a NULL source and only COLORFILL (`0x400`)/WAIT flags
can fill an in-bounds destination rectangle. The destination must have validated
layout and known absence of a clipper. A readable PE32 100-byte DDBLTFX supplies
the native colour at byte `0x50`; colours beyond the native bit width are rejected.
A full fill establishes a complete checkpoint; a partial fill requires one.
Arguments are captured before the original call, and only success with unchanged
epoch/generation commits. Per-operation replay uses an explicitly derived
constant native source and the existing opaque-copy operation. Ordered sessions
emit a GAP for a fill while recording, rather than inventing a source identity.

Pixel and metadata invalidation are separate: missed Lock/Unlock/Blt tracking
discards uncertain pixels and pending work while retaining validated shape,
primary identity and properties. Unknown successful Unlocks likewise discard
pixels. Missed property/identity tracking and uncertain retirement still discard
metadata. Subsequent full overwrites can recover only when metadata remains valid.

A successful unsupported or untracked draw invalidates the destination. Later
copies cannot inherit stale pixels; a new complete Lock/Unlock checkpoint can
reseed it. Self-copy, effects other than the colour-fill case above, stretching,
conversion, destination keys, unknown
source keys and clipping are rejected. An unavailable complete destination can
now be initialized only by the full opaque overwrite described in the
[bootstrap contract](opengl-primary-bootstrap.md).
Successful BltBatch, GetDC and Restore also invalidate affected pixels and
in-flight Lock checkpoints. Accepted application-owned GDI contexts now recover
a complete checkpoint after successful ReleaseDC; see the
[GDI context boundary](opengl-game-owned-dc.md). A mutation nested inside original Unlock cannot
commit its earlier copy. Restore
removes source-key provenance. The subsequent [owned Flip chunk](opengl-owned-flips.md)
rotates pixels for observed two-buffer RGB chains and invalidates the capture
epoch for unsupported successful Flips;
[primary blit presentation](opengl-game-owned-primary.md) is implemented.
An unmatched successful Unlock on a known component invalidates its pixels and
pending locks while preserving unrelated primary metadata. Unknown pixel identities
invalidate the pixel epoch. Pixel contention instead queues only the affected
target for checkpoint and pending-Lock invalidation; a 128-entry queue overflow
retains the pixel-epoch fallback. Metadata uncertainty also invalidates the
metadata epoch. Uncontended final Release
retires only its observed component. CreateSurface clears old identity and
property records before accepting a reused address. Joining two independently
tracked aliases discards conflicting checkpoints and properties.

The cache has 128 surface records and at most 64 MiB of retained plus pending
native pixels. Full Lock snapshots retain their existing 16-file/64-MiB bound.
There are at most 16 propagated blit command files and a separate 64-MiB cumulative
command-payload budget. Exhausted limits stop capture, never the original draw.
The post-call generation/epoch/budget check also covers nested application calls;
no tracker guard spans an original API call. Cross-thread guard acquisition
now permits a bounded yield/retry (8-ms GetTickCount budget, subject to clock
resolution); same-thread entry fails immediately. Timeout retains the conservative
invalidation policy. `MNM_RENDER_TRACKER_WAIT_MS=0` restores immediate skipping;
accepted debug values are 0–50 ms. See the
[contention comparison](render-startup-black-screen.md#2026-10-04-low-qt-update-cadence-bounded-tracker-waiting).
 This is bounded evidence capture,
not a game-speed render replacement or a benchmark.

After the per-operation recording limit is reached, with ordered session recording
disabled, admitted copies and fills update owned destination storage in place.
Epoch and both surface generations are rechecked after the original call; failed
or reentrant operations cannot mutate the checkpoint. This avoids cloning entire
source and destination surfaces for small sprite operations. Recording and ordered
sessions retain before/after snapshots. The 64-MiB storage bound is unchanged.
Observed alias relationships now have a derived hash/union cache, invalidated on
all graph changes, and a 256-edge bound. No additional COM queries are issued.

Primary Blt/fill publication converts only the changed rectangle when the stream
contains the exact preceding complete checkpoint (same surface token, generation
and frame count). Publication retains the complete RGBA image and the existing
version-one sequence protocol; no partial image or new wire layout is exposed.
A bootstrap, full overwrite, changed identity/generation, or intervening producer
requires full conversion. Palette publication remains full-frame. This reduces
the capture guard's work for small sprites without dropping final updates.


## Output and evidence limits

`lock-capture/blit-00000001.bin`, etc. use the existing `MNMCMD01` protocol:
CREATE source, CREATE destination-before, BLIT, CHECK reconstructed destination,
PRESENT destination, DESTROY both, END. Inputs contain owned checkpoint
pixels, except that bootstrap uses a synthetic zero destination-before image
whose every pixel is overwritten by the opaque full copy. The CHECK is a reconstruction result, **not an original-driver readback**.
It establishes agreement of the CPU propagation and independent OpenGL replay;
real-game driver equivalence still requires separate evidence. PRESENT is an
offline replay instruction. The following [primary routing chunk](opengl-game-owned-primary.md)
now also publishes supported reconstructed destinations whose primary identity
was established by a complete game-owned Lock/Unlock checkpoint or validated
application metadata.

Replay a completed file without the game:

```bash
./tools/run-qt-shell.sh --commands PATH_TO_LOCK_CAPTURE/blit-00000001.bin
```

Additional bounded lifecycle reasons are `blit_untracked`, `blit_unsupported`,
`blit_limit`, `blit_memory`, `blit_ready`, `blit_failed`, `blit_invalidated`,
`blit_propagated` and `blit_file_failed`. Their argument field is the source
interface token. Tokens are valid only within a launch.

Run `./tools/test-render-lock-blits.py` for generated PE32 i386 surfaces under
Wine, independent Python native-pixel copies and OpenGL command replay. The fake
engine maintains its own backing pixels, exposes padded lock storage and poisons
that storage during Unlock. It dumps original-operation pixels independently of
the bridge; every accepted command is compared to those bytes as well as the
Python and GPU results. Fixtures assert original call counts and LastError,
including zero observer queries/locks, and reject offscreen live publication.

Coverage: chained Blt/BltFast, exact source keys, aliases, failed draw/retry,
changed CPU pixels, 16/24/32-bit RGB, reseeding, failed key changes, operation
limits, unsupported flags, self-copy, untracked source, attached clipper,
Restore, removed keys, final Release/reused tokens, invalid bounds, source
subrectangles, a nested CPU update that invalidates an in-flight copy, initial
CreateSurface properties and conflicting alias records.

Evidence: 20 fixtures in
`working/tests/render-lock-blits/run-8nnykch_/report.json`, plus the two focused
creation/conflicting-alias fixtures in
`working/tests/render-lock-blits/run-raxunwra/report.json`. A further focused run
`working/tests/render-lock-blits/run-rf7is369/report.json` adds a successful
Restore nested inside Unlock and rechecks nested CPU writes, updates and reseeding.
This brings coverage to 23 distinct propagation fixtures. All accepted outputs
matched the fake engine, Python and OpenGL (Mesa llvmpipe) byte for byte.
Confidence is high for this scoped synthetic contract. Real game coverage,
unobserved COM aliases, longer-lived/partial updates, indexed palettes,
primary surfaces initially seeded only by GPU draws and actual frame boundaries
remain unvalidated or unsupported.

The existing 14 Lock/Unlock fixtures also passed after this integration:
`working/tests/render-lock-lifecycle/run-zy86drrm/report.json`.

The existing Wine-to-Qt/OpenGL bridge regression passed:
`working/tests/render/run-3n0xvpyq/report.json`, including prior opaque/keyed
readback capture and independent native/OpenGL replay. The production bridge
rebuilds as PE32 i386; source hashes and binary hash are recorded by
`working/build/render/manifest.json`.
