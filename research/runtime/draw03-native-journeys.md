# DRAW-03 complete single-player journeys

The target scope is two reproducible No-CD single-player journeys at800x600
RGB565: Quick Battle and a fresh campaign mission, from launch through normal
shutdown. DRAW-02 comparison expansion and DRAW-03 implementation proceed
together. A native scene needs independent original pixels and all required
observable rendering state before it can count toward completion.

Required journey stages include enabled intros, transitions, Main/setup/loading,
active terrain and animated creatures, combat/effects/lighting, camera pan and
rotation, HUD/minimap/markers/windows/tooltips/text/cursor, selection/picking,
Mini/Preferences, save/load, mission/result/menu transitions, resource changes,
refusal/recovery and shutdown. Campaign coverage includes Realm return/map
transitions. Quick Battle coverage includes results and subsequent menu entry.
Gameplay balance and original drawing bypass are outside this milestone.

## Increment: persistent source-owned frame composition

`DrawingScene::publish` validates a complete boundary without sealing the native
history. Subsequent producer records retain the reconstructed canvases and World
resource state, but invalidate the service's current presentation. Previously
returned GPU leases keep their complete frame. Sequence/queue/extent errors poison
the service; it cannot expose a stale frame as a new completion. `seal` makes the
terminal boundary immutable and does not republish an already completed frame.
Queue-zero menu/loading canvases are permitted.

The schema2 offline runner takes ordered explicit frame boundaries and emits
each full native frame. It does not select scenes from diagnostic checkpoint
numbers. The comparator independently checks those boundary identities against
original completion records, including completed queue counts. Original planes
and their paths stay outside the native process. Large exact source histories
use bounded XZ fragments with packed/raw/concatenation SHA-256 checks, a128MiB
decoder memory limit,32MiB per-fragment output limit and128MiB total input limit.
Trailing or concatenated XZ streams refuse.

The first retained multi-frame fixture extends the unchanged original-active
map2 startup comparison through sequence44927. It includes two complete queue-zero
checkpoints and fifteen successive complete canvases after World, HUD, minimap,
markers and cursor. Each boundary has an independent full800x600 original plane.
These are pre-display-copy canvases, not physical presentation evidence or an
active-battle journey. The fixture retains16 actual source assets and exact
contiguous producer bytes; installed game files are unnecessary for replay.

Native tests cover menu-only publication, retained World/HUD history across
multiple queues, old GPU lease lifetime, surface generation changes, terminal
sealing, duplicate boundaries, stale output refusal, poisoning and cleanup.
Portable guards cover bounded fragment admission, ordering/queue/oracle identities
and command-level oracle separation. Simulated command outputs in the isolation
test establish argument separation only; the separate native replay performs
the actual pixel comparison.

## Increment: World destination and generation changes

The outer adapter validates global original queue ordinals independently of the
private World resource generation. Releasing the World canvas, changing its ID
or changing its extent creates a new owned World resource context. Native CPU
canvas history stays authoritative; cumulative queue/raster identities survive.
The new context uses the existing rolling World interface with a local ordinal.
Repeated or skipped outer ordinals refuse and poison presentation.

Native tests exercise release/recreation using the same ID, a new ID with a new
extent, and switching to another fully defined canvas without releasing the old
one. They check retained pixels, global counts and zero remaining surfaces.
The original startup fixture still exercises only a single World destination;
these tests do not establish original map-transition equivalence.

## Increment: externally checked raster returns

`DrawingScene::rasterResult` exposes the most recent World raster's sequence,
outer queue, canvas, backend and optional low16 AX. The result comes from the
existing recovered horizontal-refusal rule and sampled zero-return backend
domain; original EAX observations do not supply it. Backend8 explicitly has an
unknown return contract. Non-raster operations clear the result and poisoned
instances refuse queries. This does not implement node state, picking, hotspots,
flags, scratch globals or high EAX.

The comparator removes field21 from every raster record before launching the
native process. Exact wire/source bytes otherwise remain unchanged. The native
runner refuses nonzero observed raster returns and writes a separate bounded
result transcript. The comparator verifies count/order/queue/canvas/backend,
low16 values, transcript hash and unknown-result count against original return
observations held externally. Pixel publication and result comparison have
separate outcomes; neither substitutes for the other.

Historical provenance matters: the full c4ps704z capture contains one selected
native bypass at sequence45634 in queue16. Its recorded EAX is0xfffffff8; this
is not an original raster return observation. The frozen independent comparator
records that exact selected bypass in `bypass_checks`, while its private original
raster replay establishes pixel equality separately. The retained six-canvas and
17-frame prefixes end at20458 and44927, respectively, before suppression. The
new state comparator refuses any prefix reaching the first recorded bypass and
refuses unknown bypass provenance. Historical reports and hashes stay unchanged.

Portable guards cover return removal, bypass provenance, result mismatches,
unknown returns and malformed/reordered output. The command-isolation mocks use
simulated outputs only to check separation; fresh native execution supplies the
actual startup result comparison. The full consumer-state contract remains open.

## Reproduce

```sh
cmake -S compat/legacy/drawing-scene -B working/build/draw03 -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/draw03 --target mnm-drawing-scene-replay drawing-scene-test -j4
python3 -B tests/test-drawing-journey.py
python3 tools/check-drawing-scene.py --headless \
  --corpus tests/fixtures/drawing-journey/scene.json \
  --retention NR.drawing-journey.retention-20261010 \
  --executable working/build/draw03/mnm-drawing-scene-replay \
  --report working/tests/draw03/new-journey-comparison.json
```

## Remaining boundaries

The recorded native comparison matches8,160,000 checkpoint pixels across all17
full canvases, with zero differences; all17 separate frame publications also match.
The six-canvas startup regression matches2,880,000 pixels after the service
extension. Both results retain prospective scope claims and stable source/input/
binary hashes. The native suite and eight scene/four journey guard tests pass.
Evidence IDs are `NR.drawing-scene.persistent-native-20261010` and
`NR.drawing-journey.persistent-native-20261010`.

This increment supports finite ordered frame publication. Full Quick Battle and
campaign capture, enabled movie-to-canvas integration, physical presentation,
camera/combat/alternate producers, picking and other consumer state, independent original World
destination/extent transition comparison and recovery remain pending. It adds no live capture,
continuous transport or original-body suppression. Complete DRAW-03 requires
the full journeys above with independent pixels and required side effects;
a passing startup-prefix comparison cannot satisfy that claim.


## Integration with the complete DRAW-02 startup corpus

The committed DRAW-02 increments faa862a and e4ff085 are integrated into the
isolated native branch, preserving their complete1062-checkpoint source/oracle
corpus,14journey/21operation acceptance policy, historical results and append-only
receipts. The full-sequence adapter now strips observed raster EAX before using
the stricter native runner. Stored capture bytes and original hashes stay unchanged.
Its pixel comparison spans all16queues and46,647records; it does not compare full
original return-state history across the capture's recorded native bypass. The
separate17-frame result comparison retains its original-active cutoff.

The current original collector still lacks physical publication and complete
session action identities. Full presented Quick Battle/campaign journeys require
new collector contracts and observed captures; startup composition cannot close
those gates. The other contributor's uncommitted session tools remain in their
own worktree.
