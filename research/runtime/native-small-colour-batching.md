# Ordered small colour effects

The 2026-11-12 live combat diagnosis measured 9.16575 source updates/second
across 275 consecutive intervals. An overlapping native-report range averaged
85.8426 ms native work, including 81.4210 ms submission, with approximately
995 submitted draws and 188 texture uploads per World. These are CPU wall
measurements including driver waits, not GPU busy-time attribution.

The stopped session `run-s8xj4cgr` retired normally with 3196 balanced Worlds.
Its retained completed generation3196 contains359 owned 2x2 colour rectangles,
with124 distinct dimension/mode/source combinations. Those small sources compete
with the existing64-entry colour atlas. Generation3197 is an unpublished WRITING
slot and is explicitly excluded. Raw timing headers, the exact completed source
and request, final reports and executables are retained in
`working/slowdown-20261112/observations.sqlite3` in the owned task checkout.

`GlBlitter::smallColourRects` accepts at most 512 fully owned clipped 1..2 by
1..2 RGB565/RGB555 writes. A direct batch requires fully defined destination
storage and validates the entire list before mutation. The current implementation
partitions at most 2048 pixel operations into destination-pixel chains with a
stable sort. Operations at a given pixel retain their exact original order.
One snapshot supplies the initial destination words; one instanced draw evaluates
each chain, including every intermediate integer rounding step. No other pixel
is read by these copy/half/quarter blends, so grouping distinct pixels does not
change their result. Sprite, larger-rectangle, hidden-draw and caller-budget
barriers remain unchanged.

The original implementation froze a separate destination envelope for each
consecutive disjoint group. That implementation and its measurements below are
retained historical evidence. The overlapping-pixel change is measured separately
in [large-battle attribution](large-battle-attribution-20261112.md).

The operation buffer contains at most 2048 packed source/mode words (8 KiB); the
instance buffer contains at most 2048 pixel/span records (32 KiB). Native pixels
remain integer storage. There is no ordinary readback, source-image upload,
interpolation, original output seed or algebraic approximation of chained blends.
The source words are uploaded through a bounded OpenGL 3.3 texture buffer, not a
source image atlas. Renderer-owned scratch and buffer objects are released with
the context; cache release does not discard pending destination state.
The isolated renderer retains serial colour operations and its existing wire and
recovery protocol.

`SceneRenderer` collects only consecutive small colour requests within the
caller's admitted draw/time budget. Sprites, larger rectangles and hidden draws
form barriers. Incremental draw counts, previous-frame refusal and completion
rules remain in force. The path is enabled only with the existing colour-cache
policy; generic uncached scenes keep the serial primitive. CanvasWorld defaults
to it; `MNM_WORLD_SMALL_COLOUR_BASELINE=1` selects the existing atlas path for
comparison. Larger rectangles retain the64-entry atlas and its storage bounds.

The new scalar/serial controls check twelve complete outputs per backend,
includingRGB555/RGB565, clipped1x1/1x2/2x1/2x2 sources, four blend modes,
overlap, intervening larger writes, invisible draws, budgets1/3/32/12320,
cache release, invalid geometry/words/modes/counts, unchanged pixels after
refusal and undefined destination refusal.256 disjoint effects require one
submission and no source-texture uploads. Current llvmpipe and Apple M1 executions
pass; existing colour-cache, indexed atlas and92 retained-image repeat controls
also pass.

A separate replay binds the retained source sprites and colour requests to
native resources and an authored nonconstant800x600 RGB565 background. It does
not reconstruct the original history. Both policies execute the same1626 ordered
primitives, with50 full-frame comparisons and identical output hash
`b675bbce8b4ba8d0afa3749b0ab80b53369db1839d9def1b87eab3424f0f66ed`.
On Apple M1, warm mean submission decreases69.6308 to34.2004 ms; mean completion
including explicit verification synchronization decreases70.9205 to35.5616 ms.
Submitted draws decrease658 to372, and warm source-texture uploads31 to0.
The exact retained frame and warmed cache differ from the live diagnosis range;
these counts must not be substituted for its188 uploads/World. Timings are a
bounded same-input native comparison, not live FPS or original-pixel equivalence.

Fresh crowded-session validation is recorded separately. All-map20FPS, arbitrary
combat loads, physical scanout and original-driver equivalence remain pending.
Historical shared-renderer evidence keeps its original hashes and source phases;
these controls do not refresh unrelated claims or repair unsupported raster paths.

## Moving combat and the reported floating base

`COMBAT12.live-diagnostics01` retains seven owned probes, including failed
instrumentation and source refusals. None is a normal-Quit acceptance pass.
The first crowded run reached eight accepted casts but did not require creature
motion. The next attempt is invalid because its instrumentation contended on
SQLite. The corrected moving run observes eight distinct moving zombies in its
first phase; consecutive source rates across five phases range11.9633–14.7739Hz.
It ends at the existing FAILURE43/tag14/caller`0x54a563` guard in unpublished
WRITING generation1335. This is the historically documented unsupported caller,
not a newly admitted native path. Original caller guards are unchanged.

A maximum-capacity-core prototype reaches approximately19.95Hz in a mainly
stationary scene but fails its requirement for three distinct moving zombies.
The engaged fiery run reaches several moving zombies and falls to a cumulative
16.3674Hz before input refuses an inactive viewport. The guard prevents test
input from reaching another desktop window. Scene differences and failed motion
criteria prevent a controlled CPU-affinity performance conclusion. The prototype
source is retained in SQLite; CPU-placement changes were removed from the public
launcher batch.

The user reports a floating zombie carrying a rectangular patch in that fiery
run (`run-1w9tyfs_`). Its original spell journal records spell80, Levitate,
cast by enemy slot2/type25/owner2 against zombie31, with mana13312→10240.
Passive actor rows then show rising height and horizontal drift. The installed
Grimoire describes suspension and wind-driven drift. These observations support
Levitate as the movement explanation; they do not identify the exact object in
the user's report or establish that its rectangular base is correct. A short
window recording shows a pale platform under a zombie near the stone circle.
The base still needs correlation to the source requests and original output at
that moment. Source sprite inspection finds no terrain platform in the zombie
frames. Replaying a later completed source packet on an authored background
matches both native policies, but does not validate the earlier reported frame.

Two subsequent bounded collectors retain complete observational source prefixes:
444 and1625 consecutive completed Worlds, each starting at1, with matching
source/request CRC32C and stable original-returned/free slot identities. Both
runs then fail the existing caller`0x54a563` guard before a captured Levitate
case. Unpublished failure prefixes remain negative evidence. The optional
lossless viewport capture, source compression and short window recording add
observation overhead; those rates are not recording-free FPS proofs.

The verified result is a bounded rendering-cost reduction with exact native
scalar/serial comparisons. Moving-combat19–20FPS, the reported floating base,
long-session stability and original/physical output acceptance remain pending.
All raw successful/failed trials, source versions, first-party spell text,
videos, exact binaries and source-history databases are pinned under
`working/slowdown-20261112/` with verified SQLite backups.
