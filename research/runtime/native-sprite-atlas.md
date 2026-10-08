# Native sprite atlas and palette reuse

NR.sprite-atlas is an intentional native storage policy, registered before its
implementation. Native World enables it automatically. Other resource clients
retain the original per-frame cache by default for direct comparison. No wire
contract, producer hook or supported original draw-kind set changes.

The cache packs owned frame indices or RGB565 words and separate coverage into
integer texture pages. Indexed keys contain resource identity, revision and frame,
not palette variants. A copied 256-word RGB565 table is supplied to the integer
shader independently; the renderer retains one value table per copy/composition
program and updates uniforms only when values change. Embedded palettes use the
same RGB >> 3/2/3 conversion as the previous CPU expansion. The coverage plane
continues to distinguish opaque palette zero from transparency. There is no
palette pointer identity, RGB round-trip or filtering.

Copies, half/quarter blends and displacement use the existing integer arithmetic.
Displacement still snapshots the preceding native destination. Derived projected
shadow masks keep their row/clip shape identity and pack as ordinary word/mask
entries. Signed origins and clipped source coordinates include the atlas offset.

Pages start at 1024 square, grow only for frames up to 2048, and shrink for explicit
smaller budgets. The default eight-million-texel local budget permits four 1024
pages, charged as two planes each. Actual native textures are R32UI, so this is
32 MiB of plane storage plus bounded validity maps and indexed presentation palette
textures; it is not an eight-MiB allocation promise. The renderer-wide 64-surface
and 16-million-texel caps still apply. Metadata has a separate 4,096-frame default
and 16,384 hard maximum, rather than increasing the old per-frame surface limit.
Captured replay needed one page and 250 resident frame/shape entries.

Shelf allocation leaves released holes until a whole least-recently-drawn page is
evicted. Eviction retires all entries on that page; it never retains references
across admission or changes draw order. Resource revisions invalidate old entries,
release frees empty pages, and cleanup frees pending CPU planes and GPU storage.
Excessive dimensions, malformed planes, duplicate/stale completions, invalid
indices/coverage and insufficient local/global storage refuse admission/drawing.
An oversized frame is rejected before retiring valid entries.

Owned CPU preparation retains indices for indexed atlas requests. The existing
worker handoff and scene completion checks remain in force. GPU transfers write
contiguous rows directly into the page region; an advance writes at most one
plane patch and obeys the same byte budget. Unwritten page regions remain undefined
and cannot be sampled or presented. A 512x256 fixture transfers one MiB in 128
advances of at most 8,192 bytes. Palette uniforms are separate bounded draw state,
not pixel-plane uploads; upload byte counters therefore count only plane data.
Driver calls/allocation latency and total process RSS are not hard time guarantees.

## Validation

The reproducible runner is `python3 tools/test-sprite-atlas.py --full --capture
working/experiments/scene-observer/run-19ynq2s1`. Prospective behavior/scenario claims
and source hashes are saved before compilation. Reports retain unsuccessful runs;
source/contract stability is checked after execution and originals are verified
before/after captured asset consumption.

Focused regression execution `working/tests/sprite-atlas/run-vwau819d/report.json`
passed seven tests, including Qt heartbeat/cancellation and existing non-atlas
regressions. Pixel checks cover owned/embedded palettes, 256 palette variants,
opaque black, word frames, nonzero atlas offsets, signed clipping, copy, half and
both quarter blends, displacement, projected shadows, row validity, page/metadata
pressure, resource revisions, release, external handle refusal and cleanup.
The responsiveness fixture now requests a different uncached frame rather than
expecting a palette change to expand/upload pixels.

On the same closed 16-queue corpus, replaying each renderer twice from explicit
native zero initialization rendered 30,720,000 pixel instances: a reference pass and three passes compared
against it (23,040,000 pixel comparisons). First-pass expanded
uploads were 43,988 versus atlas 500; second-pass expanded uploads were 43,988 versus
atlas zero. Drawing plus normal GPU presentation submission averaged 601.8 versus
285.8 ms cold and 585.0 versus 252.3 ms warm on llvmpipe in Debug. Separate test-only
readbacks synchronized and compared every result. Normal replay presentation used
zero native readbacks; no original destination was a rendering input. Timing here
includes synchronous cache preparation/drawing, not Wine startup or live worker
and timer scheduling. The final source-stable full execution is retained at
`working/tests/sprite-atlas/run-t7x04nmz/report.json` and copied verbatim to
`native-sprite-atlas-validation-20261008.json`: all 70 CTests passed. Its final
measurements were 528.4 versus 238.5 ms cold and 526.6 versus 205.7 ms warm; upload
counts, pixel equality and storage bounds were unchanged. Timing variation between
runs is retained rather than replacing earlier measurements. The registered
evidence ID is NR.sprite-atlas.native-20261008; historical fingerprints are preserved.

## Remaining boundaries

This compares native storage paths, not newly recovered original raster behavior
or original startup canvas equivalence. Existing comparison evidence affected by
shared renderer edits becomes source-stale and remains historical. Live startup
performance on the user's driver remains unmeasured. Page fragmentation, unusually
large working sets, context-loss recovery, palette batching and destination snapshot
reuse are further work. Unsupported kind 8 still requires its concrete virtual
method/object writers to be reconstructed; this change does not admit it.
