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
camera/combat/alternate producers, picking and other consumer state, World
destination/extent changes and recovery remain pending. It adds no live capture,
continuous transport or original-body suppression. Complete DRAW-03 requires
the full journeys above with independent pixels and required side effects;
a passing startup-prefix comparison cannot satisfy that claim.
