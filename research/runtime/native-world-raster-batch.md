# Native World queue batching

This opt-in experiment retains owned source requests and immediate low16AX
fallback decisions while deferring native canvas writeback until World return.
The original destination is protected against access during admitted traversal.
Unexpected original reads/writes, source aliasing, other producers, changed
destination identity, malformed replies and incomplete queues refuse.

The V3 transport is an intentional native policy. It does not recover a new
original batching mechanism. Original traversal, simulation, HUD/menu outside
World and non-drawing lazy wave preparation remain active. Evidence for the
earlier per-raster contract remains immutable and may become stale after shared
source edits. New batching validation must declare its own scope before running.

The current V7 guarded queue pilot and complete prefix pass independent
original comparison and synthetic ABI/refusal checks. The domain is the pinned
800x600 format0 Quick Battle map2 startup, one queue followed by queues1..16.
Unknown callers, unsampled generic entries, vertical top-clipped displacement,
volatile output state/scratch globals, other formats and general gameplay remain
gaps. Canvas protection rounds to pages and may conservatively refuse access to
adjacent memory; no claim follows for such rejected allocations.


## Entry observation boundary

The v3/v4 live pilots refused before any admitted raster (queue1/count0).
The fault instruction was `0x7bcdaaf3`, writing `0x034a0004` and
`0x034c0004` respectively; rounded protection covered `0xeb000` bytes.
The addresses are launch-specific. Source review found `scene_observe` still
allocating/freeing its process-heap snapshot after the producer queue-entry
callback had enabled protection. A separate producer heap alone did not resolve
that later observation. This diagnoses adapter ordering, not an original pixel
reader or a successful original comparison. The retained
[failed pilots](native-world-raster-batch-entry-refusals-20261008.json) preserve
the raw fault hashes and declarations.

`canvas_producers_protect` now runs after all entry observation snapshots and
temporary process-heap buffers finish, immediately before original traversal
resumes. Producer records use a private heap created before the game queue;
protection stays active during their capture and original traversal. The return
checks every covered region is still committed/PAGE_NOACCESS, then verifies the
protection being restored was PAGE_NOACCESS. Unexpected protection changes
refuse completion. The synthetic fixtures invoke this same final entry boundary.

Native original-comparison input is a closed producer stream plus installed
assets. The native process receives no captured destination oracles, request
files or live replies. A bounded FIFO streams its independent CPU intermediate
canvases/AX to a separate PE32 comparator; the comparator executes the actual
original entry on its own evolving canvas. At each queue, its entry state is the
source-only native startup/HUD history, which is separately checked against
observed original completions outside the replaced traversal. Native GPU final
parity, live reply parity and per-raster original pixel/AX parity are separate
checks. Compiler dependency paths are resolved before comparing to the explicit
133-source declaration; the initial harness normalization refusal is retained
under `working/tests/world-raster-batch/run-zby0302t/report.json`.


The next harness run rejected an intermediate after eleven off-screen matches:
`Image` stores UInt32 native words, while the proof FIFO requires tight UInt16
RGB565. The live V3 reply already converted properly; the independent proof
writer did not. The writer now creates explicit UInt16 words before transmission.
The [retained harness refusals](native-world-raster-batch-proof-refusals-20261008.json)
keep both failures, source declarations and raw hashes. Failed proof output
never supports an original-comparison or replacement promotion.


## Reproduce the bounded experiment

Build `mnm-canvas-producers-live` from `compat/legacy/canvas-producers` into
`working/build/world-raster-batch`. Prepare a new declaration before executing:

```sh
python3 tools/draft-coverage-claims.py \
  --behavior RS.world-raster-batch-return \
  --behavior NR.world-raster-batch-transport \
  --scenario world-raster-batch-startup-20261008 \
  --scenario world-raster-batch-abi-20261008 \
  --output working/tests/world-batch-new-claims.json
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/capture-scene-game.py --samples 16 --map 2 \
  --canvas-producers \
  --producer-live working/build/world-raster-batch/mnm-canvas-producers-live \
  --world-producer-handoff --world-raster-prefix 16 --world-raster-batch \
  --claims working/tests/world-batch-new-claims.json
```

Use a fresh declaration filename and the printed experiment report path for
`tools/test-world-raster-batch.py --capture-report PATH --claims CLAIMS`. That
comparison rebuilds frozen sources, runs five focused native tests and compares
every raster canvas/AX to its precise original entry. The capture and comparison
verify original manifests independently before/after. Closed post-bypass
checkpoints alone do not establish original equivalence.

Batching is opt-in in this bounded producer/raster experiment. The public
interactive shadow launcher and the separate startup-history launcher do not
select this raster replacement automatically. General gameplay/native input,
longer sessions and performance profiling remain separate integration work.


## Current bounded evidence, V7 (2026-10-08)

The [queue1 pilot](native-world-raster-batch-pilot-replacement-v7-20261008.json)
compares1,839native intermediate canvases/AX exactly to their actual original
entries. The [complete prefix](native-world-raster-batch-prefix16-replacement-v7-20261008.json)
then compares all23,480admitted calls in queues1..16 over11,270,400,000pixels.
All16native CPU/GPU finals/live replies and1,062observed checkpoints match.
The original destination remains protected during each traversal; every return
verifies protection and performs exactly one native writeback before the
checkpoint/HUD boundary. There are16World GPU readbacks and16queue handshakes,
with zero per-raster handshakes. Other diagnostic checkpoint readbacks remain.

Full-canvas reply pixels drop structurally from22,540,800,000bytes for one
reply per raster to15,360,000bytes for one reply per queue. The diagnostic live
capture elapsed39.27seconds including startup/menu work; this is not a frame
latency or real-time benchmark. Independent FIFO comparison elapsed196.12seconds.
All133declared source fingerprints and captured inputs remain stable;80normalized
compiler inputs are within that declaration. Five focused native CTests pass.
The [V7 ABI fixture](native-world-raster-batch-abi-v7-20261008.json) proves13
synthetic body skips with prescribed AX/cleanup/state,49original forwards, four
guard/source-alias/producer refusals and three bad-reply refusals before
writeback. The native packet fixture rejects27malformed manifests. Original
manifests verify2,927files before/after capture and comparison.

Ten admitted entries were observed: black, clipped fallback, shadow, three
blend wrappers, wave, indexed-copy`0x59521a`, terrain`0x59603e` and destination
displacement`0x596490`. The other admitted entries`0x5947b2`,`0x595b47`,
`0x5968a4` remain synthetic-only in live suppression. Understanding remains
partial. The recovered return contract has scoped implementation, recorded
original comparison, live-equivalence integration and scoped live replacement.
The intentional transport has scoped implementation/live-equivalence integration,
with no recovered original batching-policy comparison/replacement claim. Original traversal, simulation, menu/HUD outside World
and non-drawing lazy wave preparation remain active. Caller/global-state and
top-clipped displacement gaps stay open. This evidence does not renew the
earlier scalar replacement or public launcher domains after shared-source edits.
