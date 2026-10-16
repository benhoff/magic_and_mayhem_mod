# EGL destination and software-upload witnesses

These are observer-owned little-endian files, not recovered original formats.
All envelopes start with eight magic bytes, uint32 version and exact byte size.
Common identity is EGL call/PID/TID. Object IDs are run-local labels; no host
pointer or C/C++ layout crosses the files. Unknown associations stay explicit.

## Destination before real swap

`MNMEDB01`, version1/128bytes, is paired with
`egl-destination-before-swap-<call>.565`. Word indices after the magic are:

| uint32 words | Meaning |
| --- | --- |
| 2–8 | Version, size, call, PID, TID, nominal EGL width/height |
| 9–15 | Draw FBO0, prior read FBO/buffer, prior default read buffer, selected draw buffer, exact byte count, restoration flag1 |
| 16–21 | Pack alignment, row length, skip rows/pixels, swap bytes, pack-buffer binding |
| 22–25 | Monotonic begin/end, low/high words |
| 26–27 | Same context and before-real-swap flags1 |
| 28–31 | Reserved zero |

The raw plane is RGB565 in GL lower-left row order. The external comparator
flips rows; it never feeds these pixels to a native process. Pack layout must
permit a tight plane, extent≤2048 and total per-process destination budget512MiB.
The reader validates exact hashes, identity/extent and completion timing. Nominal
EGL dimensions do not establish the actual underlying allocation dimensions.

## Context rebind experiments

`MNMERB01` uses96bytes. Version1 records call/PID/TID, applied count (including
explicit zero), first-context/extent-change reason, old/new logical extents and
success/state-preservation flags. Version2 additionally records mode1 (same-current
rebind) or2 (release then restore), and verified-release flag. Remaining words
are zero. The reader distinguishes a no-op from an attempted successful call.
These experiments did not correct the recorded physical resize failure.

## XCB image packets inside the real swap

`MNMEXI01` records checked/unchecked SHM PutImage (method1/2) and PutImage
(method3/4). Limits are1024packets per swap and65536per process. Exact cookie
sequence and errno are forwarded; metadata does not read image bytes or establish
XCB error delivery, SHM ownership or complete dispatch coverage.

Version1 is128bytes. Words2–10 contain version/size, parent call/PID/TID, packet
index, method and nominal EGL width/height. Words11–26 are this vector:

`drawable, total_width, total_height, source_x, source_y, source_width,
source_height, destination_x, destination_y, depth, format, send_event,
left_pad, data_bytes, shm_segment, shm_offset`.

Destination coordinates are sign-extended int16 values. Word27 is the original
cookie sequence; words28–31 are monotonic begin/end, low/high words. No connection,
GC or data pointer is exported. A cookie is not a server acknowledgement.

Version2 is224bytes. Words32–35 add the preceding successful post-return EGL
width/height and observed native X11 window/generation. Words36–39 are zero.
Words40–55 repeat the unchanged original argument vector.

Version3 is240bytes. Words36–37 record crop-applied and height delta;38–39 remain
zero. Words11–26 are the forwarded vector and40–55 retain the original vector.
Words56–59 add preceding full-upload image width/height, supported full-GL-blit
predicate and policy-enabled flag. A crop may change only source_y/width/height;
the reader recomputes the admitted geometry and rejects any other difference.
Unapplied packets must remain identical. Legacy versions remain readable.

All selected packet identities/extents must agree with their EGL frame; packet
indices are contiguous, end precedes post-return completion and begin follows the
destination witness. Immutable filenames and hashes remain in executed reports.

The crop is an explicit policy for the observed Wine/llvmpipe stale-size image
path: require a known window/generation, preceding successful logical extent and
full-upload image geometry, one supported full default-framebuffer blit, and a
full zero-origin depth24 ZPixmap SHM upload. On a shrink with the preceding image
geometry still submitted, select source_y=`old_height-new_height`, width/height
of the new nominal extent. The crop is inside the original full image and keeps
all destination, SHM, event and cookie fields unchanged. No pixel value chooses it.

Padding alone is insufficient. Proper fresh-sized buffers, repeated extents and
growth remain unchanged. An arbitrary driver retaining oversized storage with an
already-correct image origin is outside this intentional policy's scope. The
flag stays opt-in; no universal driver, allocation-lifetime or rendering-replacement
claim follows. See [findings and controls](../runtime/drawing-egl-swap-completion.md).
