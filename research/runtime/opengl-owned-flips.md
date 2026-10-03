# Flip routing from owned RGB backbuffers

Implemented 2026-10-03 after [primary initialization](opengl-primary-bootstrap.md).
This is bounded instrumentation in `runtime/render/`, tested without launching
the game or consuming original artifacts.

## Confirmed implementation scope

Application GetAttachedSurface (vtable slot 12) results establish a front-to-back
relationship only when the successful query requests exactly BACKBUFFER (`4`)
and no extended caps. Returned interfaces are hooked using the originating
interface version. The observer issues no extra COM calls or references.
The request is copied before forwarding, so original input mutations do not
change its interpretation. Failed queries establish no relationship.

Accepted Flip calls require a known RGB primary/front with COMPLEX and FLIP
caps, explicitly observed BACKBUFFERCOUNT of one, an observed attached RGB
backbuffer with BACKBUFFER/COMPLEX/FLIP caps, matching dimensions/bits/masks,
complete owned back pixels and no active Lock on either buffer. Both NULL and
an explicit target resolving to that backbuffer are supported. Aliases come
only from successful application QueryInterface calls. Flags are limited to zero
or WAIT (`1`); stereo, intervals, DONOTWAIT and larger chains are not modeled.

The API associates backbuffer memory with the front surface and cycles attached
buffers when the target is NULL. An explicit target selects a member of the
chain. Restricting this implementation to two buffers makes their exchange
unambiguous. References: [Microsoft Flip](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-flip),
[Microsoft GetAttachedSurface](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-getattachedsurface).

The pre-call checkpoint records the capture epoch and both surface generations.
No tracker guard spans the original API call. After success and revalidation,
the tracker swaps owned native pixel snapshots, advances both generations and
publishes the new front through the existing RGBA stream. Shape, caps, clipper,
key and attachment provenance remain associated with interface identities.
No driver pixel pointers are read and no allocation is needed for the swap.

An unknown old front becomes an unknown back after the first swap. It cannot
be presented again until a complete Lock/Unlock checkpoint or supported opaque
initialization establishes all its pixels. Once both buffers are complete,
repeated rotation and incremental backbuffer copies retain their native pixels.
Successful Lock checkpoints refresh caps/count provenance instead of retaining
older descriptor fields. RGB conversion and Qt publication use the same path
as direct primary Unlocks and reconstructed primary blits.

A failed original Flip changes neither owned pixels nor the frame counter.
An unsupported, untracked or nested-invalidated successful Flip conservatively
invalidates the capture epoch; the prior Qt frame remains visible. Successful
application AddAttachedSurface/DeleteAttachedSurface (slots 3/8) also invalidate
the epoch because the chain may have changed. Final Release, Restore and existing
contention/generation checks continue to invalidate stale provenance.

Capture accepts at most 16 successful tracked Flips per launch in addition to
the existing Lock/blit/memory bounds. It is not continuous replacement rendering.
Flip adds no new binary replay format: native fake-engine dumps and ordered RGBA
stream snapshots provide its offline evidence. Diagnostics are bounded and
deduplicate repeats, so their line count is not an operation count.

## Offline tests and confidence

Run `./tools/test-render-owned-flips.py`. Synthetic x86 surfaces maintain
independent native backing pixels and execute the original buffer-pointer swap.
The front is never Locked. Backbuffer Lock storage is poisoned at original
Unlock. The fixture asserts original arguments, HRESULT, LastError and exact
application query/Lock call counts; no observer calls are permitted.

Every stream snapshot is checked byte for byte against independent original
front pixels, including unchanged counters after failures and rejected capture.
The actual Qt/OpenGL viewport checks final frames under Xvfb/Mesa. Tests cover
rotation, explicit targets, observed target aliases, failed Flip/retry, unknown
old-front rejection, incremental backbuffer draws, longer chains, missing count,
unsupported flags, unobserved/failed attachment queries, wrong targets, nested
descriptor invalidation, operation limits and attachment mutation.

Confidence: confirmed scoped synthetic PE32 i386 -> native owned buffer swap ->
RGBA stream -> Qt/OpenGL framebuffer. Real-game chain discovery, indexed
palettes, larger flip chains and driver equivalence remain unvalidated.

Evidence: thirteen cases passed in
`working/tests/render-owned-flips/run-jjyig42x/report.json`; the attachment mutation
case passed in `working/tests/render-owned-flips/run-j5pyqrde/report.json`
(14 distinct cases). All 14 existing Lock/Unlock lifecycle cases passed in
`working/tests/render-lock-lifecycle/run-67nps5nh/report.json`.

The previous initialization and contradictory-Lock metadata cases passed in
`working/tests/render-bootstrap/run-mv3xx1o3/report.json`. All nine legacy Flip
history/replay cases passed in
`working/tests/render-flips/run-49egefcd/report.json`.

Final rotation, budget and attachment-mutation checks passed after restricting
active-Lock checks to the current capture epoch:
`working/tests/render-owned-flips/run-vzlfv60r/report.json`. The production DLL
rebuilt as PE32 i386; hashes are recorded in `working/build/render/manifest.json`.
