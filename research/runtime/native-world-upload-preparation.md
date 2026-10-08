# Native World upload preparation

## Contract and confidence

This is an intentional native scheduling policy, not recovered original engine
behavior. CPU ownership and synthetic pixel/lifecycle checks are separate from
installed-game latency, original raster comparison and live replacement.

`prepareSpriteFrame` expands owned native colour and coverage planes on a CPU
worker. Palette overrides are owned RGB565 values. Row checkpoints allow
cooperative cancellation. `prepareResourceUpload` also constructs projected
shadow planes from the selected source rows; the shape preserves the existing
horizontal-refusal, every-other-row and half-darkening rules.

The World worker retains immutable decoded copies of its indexed resources so
later GPU misses never borrow ResourceManager references or reopen input files.
That additional store is limited to 64 resources and 128 MiB of accounted decoded
storage. It is independent of the GUI manager's same-sized residency budget;
decoder scratch and an in-progress new copy are not total process reservations.
There is one worker job/completion at a time. One sprite payload is at most
2048x2048x2 uint32 planes (32 MiB), including projected shapes.

## Ordered admission and bounded data transfer

The GUI scene validates the complete draw list as before. In prepared-only mode,
it draws until the next uncached resource/frame/palette-value/projected-shape
variant. It then requests precisely that draw's worker preparation. A completion
must match the owned World frame, scene cursor, resource revision, frame, palette
values and shadow rows. Geometry is checked before GPU adoption. Retired
revisions, duplicate completions and wrong cursors are refused. Missing resources
cannot fall back to synchronous file loading or pixel conversion.

GPU storage is allocated without CPU image initialization. Undefined native
pixels cannot be read or sampled. Full-width row uploads validate only the rows
being transferred, use pointers into the owned planes, and update native validity.
The two sprite planes are drawn only after both finish. Pending planes and GPU
handles are released when the cache/scene closes.

The live host uses at most 32 draws, 256 KiB of colour/coverage data and a
cooperative 2 ms elapsed limit per drawing tick. The smallest supported byte
budget is 8192 bytes, one full row at maximum width. A patch never exceeds the
remaining byte budget. Each tick attempts at least one step even if context
acquisition has already consumed the time allowance; a slow context switch cannot
strand an otherwise ready upload/draw. Texture allocation/adoption occurs on its own poll, with
at most two native surfaces for that payload; there is no whole-frame preupload.
A frame can exceed the cache's 24 variants: LRU eviction and preparation resume
in exact draw order. Equal palette values and projected source-row shapes share
uploads; absolute shadow position is independent of the shape key.

Only a complete canvas is presented and acknowledged. The previous complete GPU
lease remains displayed during CPU preparation and GPU transfer. Producer ENDED
still drains its consumed final packet. Refusals, resize and close preserve their
existing lifecycle policy. Normal presentation performs no native pixel readback
or viewport image upload.

The byte limit is a hard bound on native colour/coverage row data. The elapsed
limit is cooperative: a driver call, context switch, allocation, validity-map
initialization, draw or destruction cannot be preempted. Background/canvas
creation and whole-list preflight remain GUI work. Mask palette initialization
and presentation textures are outside the row-data metric. This is not a hard
latency guarantee or a total GPU/RSS reservation.

## Validation and remaining work

Synthetic checks exercise a 512x256 bitmap requiring 1 MiB of plane transfer in
128 ticks at 8192 bytes/tick, independently expected output pixels, partial-frame
refusal, no normal readback, warm reuse, palette variants, projected shadow reuse,
a one-entry LRU with a three-variant ordered working set, stale revisions,
undefined storage and destruction during an unfinished upload. World worker tests
prepare planes after deleting the source file. Qt tests hold a warm variant's
actual worker preparation while the previous GPU lease and GUI heartbeat continue,
then cancel without joining worker I/O.

[The new synthetic execution](native-world-upload-preparation-20261008.json)
records all 69 CTests passing, prospective scope/scenario claims and stable source
fingerprints. The large frame transferred 1 MiB in 128 ticks at 8192 bytes/tick.
Held upload preparation delivered 46 GUI heartbeat callbacks over 50 ms; closing
took 3 ms with no remaining GPU surfaces. The retained run log and JUnit output
are under `working/tests/world-preparation/run-wjnc6268/`. Testing used Xvfb and
software OpenGL; no installed game or original artifacts were consumed. Older
CPU preparation and raster evidence retain their original hashes and do not
validate this code.

The [final progress execution](native-world-upload-preparation-progress-20261008.json)
reruns all 69 tests after guaranteeing an initial step even when context setup
consumes the time allowance. Warm drawing and partial cold uploads advance at a
1 microsecond budget. The large transfer still requires 128 ticks at 8192 bytes;
held worker preparation delivers 46 GUI heartbeats/50 ms and close takes 2 ms.
The final retained run is `working/tests/world-preparation/run-o9wg32z_/`. The
preceding upload report remains historical and has stale source fingerprints
after the progress fix; its hashes were preserved.

Remaining boundaries: measure cold/warm installed-game latency and drivers;
budget/move scene preflight and canvas allocation; eliminate the additional
worker decoded copy if shared immutable storage is introduced; retire CPU sources
in long sessions; bound cancellation inside decoder/I/O calls and process drain;
expand complete startup/canvas, HUD, input and raster-side-effect coverage. No
original work is bypassed by this change.
