# DRAW-02 full session comparison

The target is independent reference acquisition and native comparison across all
fourteen drawing journeys of the pinned No-CD build. This extends the bounded
[component corpus](draw02-independent-comparison.md); original simulation inputs,
native scene implementation, continuous delivery and live suppression retain
their separate milestones.

`tests/fixtures/drawing-journeys/suite.json` freezes explicit journey acceptance
requirements and the operation/failure denominator. `tools/drawing_journeys.py`
separates original-reference coverage from native parity and refuses to close a
journey from operation, sampled World or pre-display composition checkpoints.
Eight guard tests exercise missing requirements, independent provenance, temporal
and action identity, incomplete composition, native refusal and full denominator
closure using synthetic accounting observations. They establish no original
execution or full session comparison.

The initial suite binds the existing nine component outputs and six complete
startup composition checkpoints. Both have independent retained original output;
none establishes a complete presented journey. Every journey and operation gate
remains pending until appropriately scoped captures and actual comparisons are
admitted. Existing historical comparison fingerprints are preserved unchanged.

Acquisition proceeds through complete startup presentation, contiguous camera and
combat with HUD/cursor activity, then menus, campaign, preferences, save/load,
map/results/shutdown transitions and drawing failures/recovery. Each sequence must
record original build and collector hashes, observed action boundaries, ordered
source input identities, all presented outputs and caller-visible drawing results.
Oracle pixels belong exclusively to the comparator. Source assets and owned
state must reconstruct native output without deriving resources from the oracle.

The suite schema and acceptance boundaries are documented in
[the format contract](../formats/drawing-journey-suite.md). Requirements are native
acceptance policy; no new original branch or source-state behavior is inferred
from writing the suite. Live takeover and whole-engine completion remain open.

## Complete retained startup sequence

`tests/fixtures/drawing-sequence/sequence.json` now retains all 46,647 producer
records through sixteen World returns and all 1,062 independently compared
original checkpoints, totaling 474,236,912 RGB565 pixels. Sixteen encoded source
assets supply real image/sprite bindings. The historical source capture includes
one admitted raster bypass that was independently compared with original
execution; retention preserves that limitation and does not relabel it as a
wholly original-active session or a new live capture.

The producer stream uses ordered bounded gzip chunks. Original checkpoints use
lossless gzip or XOR deltas against the immediately previous checkpoint of the
same canvas generation and extent. Every decoded plane must still match its
original raw hash and exact historical report identity. Delta bases cannot be
arbitrary paths, reordered identities or recursive graphs. Deduplication and
delta compression preserve full planes without retaining hundreds of megabytes
of repeated background pixels. Oracles remain outside native input directories.

```sh
cmake -S compat/legacy/drawing-scene -B working/build/draw02-session -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/draw02-session --target mnm-drawing-scene-replay --parallel 4
python3 -B tests/test-drawing-sequence.py
python3 tools/check-drawing-sequence.py --headless \
  --executable working/build/draw02-session/mnm-drawing-scene-replay \
  --report working/tests/draw02-sequence-new.json
```

This expands independent native comparison to the entire retained history;
producer-record continuity still cannot close a presented-frame journey. No
camera/input identities, original caller/driver semantics, general battle or
transition/recovery behavior are inferred. New acquisition for those gates
remains necessary.
