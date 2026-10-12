# DRAW-02 full session comparison

The target is independent reference acquisition and native comparison across all
fourteen drawing journeys of the pinned No-CD build. This extends the bounded
[component corpus](draw02-independent-comparison.md); original simulation inputs,
native scene implementation, continuous delivery and live suppression retain
their separate milestones.

The ordered [end-to-end execution record](draw02-end-to-end.md) and prospective
`tests/fixtures/drawing-journeys/acceptance.json` specify actions, observations,
owned inputs and dependencies for every existing requirement. Validate this plan
with `tools/check-drawing-acceptance.py --report <new-path>`. Its static inventory
never contributes acquisition or native parity; the original execution and
comparison gates below remain unchanged.

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

## One suite command and presented sessions

`tools/check-drawing-journeys.py` runs all four portable catalogs and derives
reference/parity coverage from admitted outputs. The current suite includes nine
component planes, six selected startup composition planes and all1,062retained
history planes plus103observed primary publication planes. Duplicate historical
outputs intentionally remain distinct tests
of the separate adapters; totals count comparisons, not unique game frames.

```sh
cmake -S compat/legacy -B working/build/draw02-world -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/draw02-world --target mnm-world-frame-preview --parallel 4
cmake --build working/build/draw02-session --target mnm-render-replay --parallel 4
python3 tools/check-drawing-journeys.py --headless --run-guards \
  --surface-executable working/build/draw02-session/producers/renderer/mnm-render-replay \
  --world-executable working/build/draw02-world/mnm-world-frame-preview \
  --scene-executable working/build/draw02-session/mnm-drawing-scene-replay \
  --report working/tests/draw02-suite-new.json
```

`--inventory` reports the fixed acceptance denominator without claiming execution.
`--require-reference-complete` requires original acquisition coverage;
`--require-complete` requires full native parity. Both completion checks fail for
the current corpus. Missing native backends are unsupported outcomes; invalid
original provenance is an error and contributes no reference coverage. A native
failure preserves independently admitted reference coverage. Exit0means admitted
comparisons pass (or a requested static inventory was written), exit1means
mismatch/unsupported/requested incomplete coverage, and exit2means invalid inputs
or execution failure. New immutable reports retain per-case and per-frame results.

`tools/drawing_session.py` defines a versioned file ABI for a future presented
session backend. It admits only independently registered original execution with
verified original manifest checks, exact session/build/provenance and no oracle
input dependency. Frames must be contiguous with no drops or sampling, action
counts and owned-input bindings must agree, and each operation claim needs an
independent caller-visible result. RGB565 and RGBA comparisons check complete
planes, including borders. The native process receives only owned input artifacts,
actions, frame identities and effect identities; expected pixels/hashes/results
remain in the comparator. Pixel equality cannot hide wrong API results. Backend
exit64reports unsupported; partial/reordered output and leaked surfaces refuse.

`tools/retain-drawing-session.py --evidence <registered-original-id> --output
 tests/fixtures/<new-directory>` validates and losslessly retains a new original
session. Portable artifact paths/codecs may change; original raw hashes, action/
frame/effect identities and acquisition scope remain fixed. Original captures are
never acquired by a native replay or manufactured by retention. Synthetic guard
fixtures are temporary and contribute no real reference coverage.

The [publication collector](draw02-publication.md) now independently reads actual
primary output at the guarded opaque-copy COM sites, records original HRESULTs,
and brackets five applied original menu dispatches with same-thread entry/return
phases bound to call/frame ordinals. The sixteen-return camera capture observes
all four World orientations and effective camera-center changes. A portable
`publication` suite adapter retains the owned266MiB V2 producer journal, seventeen
encoded assets, exact original call/action traces and all103primary planes.
Native source-only comparison passes49,440,000pixels with zero differences.

This is`observed_primary_blt` coverage. Alternate presentation routes, complete
input outcomes, viewport boundaries, active battles, transitions and native
failure/caller-result equivalence remain unadmitted. The fourteen full presented
journeys and twenty-one operation/failure gates remain open. Producer-only
collectors still use their earlier World-return boundary; generic complete
`presented_frame` session admission keeps its stricter contract. Native scene
expansion continues under DRAW-03; continuous delivery and original suppression
remain DRAW-04/05/06 work.

## Replaying the retained primary capture

`tests/fixtures/drawing-publication/publication.json` pins the original capture
and its separately recorded source asset bindings. Nine bounded lossless gzip
chunks preserve the complete V2 journal; source resources and primary oracles
retain independent raw hashes. Primary output planes/hashes and the original COM/menu result traces never
enter the native input directory. The inherited source journal retains its
minimap return-offset admission guard; broader caller-result equivalence remains
separate. Exact raw call/action data preserves the
observed route and phase identities instead of upgrading them to complete
session input coverage. The existing suite command includes this case through
`--scene-executable` and needs no installed game or Wine for portable replay.
The61synthetic suite/session/publication/corpus guards include an explicit refusal
to close full journeys from the newly observed partial publication boundary.
