# Ordered application DC checkpoint handoff

2026-10-05. Native policy `NR.owned-session-dc` addresses the tracked-surface
invalidation at GetDC found by the
[constant-fill startup observation](opengl-owned-session-fills.md).
This is a bounded bitmap checkpoint bridge. Original GDI drawing stays active;
individual GDI operations and fonts are not reconstructed or replayed natively.

## Ownership and command input

On a successful observed application GetDC with known surface layout, selected
bitmap and no existing borrowed context, the ordered surface becomes pending.
The native renderer retains its last texture, but the producer cannot check,
consume or complete a session with that pending surface. An overlapping writable
lock or duplicate acquisition is refused. The tracker drops obsolete owned CPU
pixels and records the application's HDC, HBITMAP and thread as before.

The existing pre-ReleaseDC path flushes GDI, checks the DIBSECTION against the
observed surface, and copies GetBitmapBits into independently owned storage.
Only RGB16/24/32 with admitted even native row sizes and matching masks qualify.
No observer GetDC, DirectDraw Lock, GetDIBits or bitmap replacement is added.

Only a successful original ReleaseDC with unchanged thread, bitmap, generation
and epoch commits that storage. An existing ordered identity resumes with one
full-surface v1 UPDATE, followed by CHECK and an eligible primary PRESENT. A
previously unregistered surface uses CREATE from the admitted checkpoint. The
UPDATE is bitmap input copied before release; CHECK bytes never drive rendering.
Existing geometry, resource, operation and transport bounds apply. No wire
version, opcode or native GPU renderer implementation changes.

A failed ReleaseDC discards its speculative copy and leaves ownership pending
for retry. Successful release with unsupported or changed ownership invalidates
the stream. Unreleased contexts prevent END. A completed bounded session does
not reopen on subsequent DC releases. The original call result and LastError
remain preserved by the existing wrappers.

## Validation protocol

```sh
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-session-dc.py working/build/live-render-channel
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-bootstrap.py --build working/build/fill-bootstrap \
  --case dc --case dc-bottom-up --case dc-retry --case dc-format \
  --case dc-unmatched --case dc-swapped --case dc-rgb24 --case dc-rgb32
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-live-render-game.py working/build/live-render-channel
```

The new ordered fixture first fills and records a primary texture, borrows a
real Wine DIBSECTION, changes an asymmetric native rectangle, then releases it.
The fake engine independently copies those pixels and poisons the borrowed
storage during original ReleaseDC. Each published native GPU frame and admitted
UPDATE is compared with independently maintained engine pixels. Explicit GPU
native-byte CHECK replay verifies final native storage, including RGB32 unused
high bits. A later ordinary blit proves the identity resumes through the stream.

Cases include RGB16/24/32, both bitmap orientations, failed GetDC, failed
ReleaseDC/retry, repeated acquisition/release, mismatched format, swapped bitmap,
metadata mutation, release on another thread, tracker contention and unreleased
ownership. Earlier unregistered offscreen DC fixtures also run through the live
ordered consumer. Diagnostic reads are explicit; ordinary native/RGBA readbacks,
viewport uploads and retained GPU resources must be zero at terminal cleanup.

## Recorded results and remaining boundary

The [current synthetic execution](opengl-owned-session-dc.json) passes all 18
cases: 12 complete sessions and six explicit refusals, with 38 independent full
GPU frame comparisons. Successful sessions also pass CPU replay and explicit
native-byte CHECK replay; the default live path retains zero ordinary readbacks,
viewport uploads and terminal GPU resources. Both incremental-consumer and
live-channel CTests pass with the required X11 display access. All eight legacy
DC bootstrap cases pass, and the [fill regression](opengl-owned-session-fills-dc-regression.json)
passes eight cases with 17 independent full GPU frame comparisons.

The [original startup observation](opengl-real-game-shadow-dc.json) verifies all
2,927 immutable files before and after the experiment. Its first 400×280 RGB565
DC handoff resumes existing identity 5 with full UPDATE sequence 16. The
lifecycle records dc_checkpoint before any session_gap. Original drawing remains
active and reaches the menu.

The next gap is the existing 4,062-record admission boundary: 21,576,532 published
bytes contain six CREATE, 4,039 UPDATE, 16 CHECK and one BLIT. Full/rectangular
unlock updates currently emit one command per native row, so the stream exhausts
its record budget before its first native PRESENT. The mirror terminates with
file GAP reason 2; mapped-channel failure is separately GAP reason 2. Consumer
cleanup passes, but the original-game native session remains incomplete with
zero native presentations. An independent real-driver pixel comparison,
continuous sessions and live rendering replacement remain unverified.

Next bounded work: pack admitted unlock rectangles into one tightly packed
UPDATE while preserving input ownership, row pitch, partial-base CHECK and
failure/epoch/generation checks. Retain record and byte limits and rerun original
startup before claiming a first native game frame.
Historical fill/startup evidence keeps its original hashes; shared-source edits
can leave older evidence stale without changing its historical result.
