# Multiple native movers and logical reservations (NS11)

Reviewed 2026-10-05. This is an opt-in native policy over the frozen navigation
adapter, not recovery of original shared scheduling or occupancy production.
`NP.multi-movement` owns this milestone; `MV.scheduler` and
`MV.dynamic-occupancy` retain their separate partial gaps and NS10 original
predicate evidence retains its historical fingerprints.

## Contract

The adapter admits up to 32 motion drivers sharing one captured creature profile.
Stationary occupants remain supported. Logical boxes retain NS10's bounded,
nonwrapping map domain, full-generation native handles and validated native slot
projection. Mixed movement profiles, effects and map-linked blockers are excluded.

Each maintenance phase builds owned reservations from its starting occupancy and
ongoing fine edges. Actors run in ascending slot order. Before moving, an actor
atomically reserves the axis-aligned union of its origin and adjacent destination,
expanded by its logical footprint. A conflicting logical box or reservation causes
waiting with route, segment, fine-motion and animation state preserved. Origins
and claims remain held for the entire phase even when an earlier actor completes
an edge. This deliberately prevents swaps, crossing diagonals and immediate entry
into a just-vacated cell. It is conservative grid admission, not physical fine-space
collision. Persistent conflicts can wait indefinitely; deadlock resolution and
automatic replanning are not implemented.

Decision priority rotates from the persisted tick. Admitted planners share the
world's 53-expansion allowance, debited by actual search expansions. A planner
receives at most its requested budget and the remaining allowance; fewer than two
remaining expansions defers planning. Budget exhaustion keeps a multi-mode actor
planning and retries a fresh search on a later tick. No open search heap is saved.
An over-budget provider result rolls back the whole tick, including prior movement.
Default single-mover and NS10 stationary mode retain their existing policies.

Claims are derived from logical positions and ongoing fine-edge records; no new
checkpoint wire fields or serialized reservation cache are introduced. Spawn and
restore reject overlapping logical boxes or conflicting saved fine claims before
publication. Cancellation, cleanup, release and generation reuse rebuild claims
from owned state. The adapter adds `multi-creature-occupancy-v1` to the resource
fingerprint, so incompatible single/stationary checkpoints are refused atomically.
Each creature retains its own ANI and segment controller.

## Reproduction and evidence

`mnm-world-sandbox move-pair` and `move-pair-fine` accept a frozen MAP, output
checkpoint, two XYZ starts and targets, then a tick count. `resume` resolves the
saved policy fingerprint. Native API callers can additionally supply owned ANI
inputs, as exercised by the synthetic runner.

Build the normal or ASan/UBSan world sandbox and run:

```bash
python3 tests/test-native-multi-movement.py \
  BUILD/world/mnm-world-sandbox BUILD/world/native-multi-movement-test OUTPUT
```

The runner uses owned synthetic MAP and ANI fixtures. Both accepted builds pass
six exact fresh-process checkpoint continuations, including mid-edge, ANI,
parallel movement and conflict waits. Model/adapter checks cover atomic claims,
full-generation identity, false origins, adjacent-edge bounds, footprint volumes,
converging goals, swaps, crossing diagonals, cleanup/release/cancellation, capacity,
restore refusal and complete tick rollback. Shared-budget checks verify rotation,
actual debit and exhaustion retry. Two staggered ANI actors each match their
independent solo simulation's complete projected checkpoint for 500 ticks
(1,000 per-actor tick comparisons, including idle ticks after arrival).

[Immutable accepted record](native-multi-creature-movement.json) contains report,
source and test-log hashes. The normal suite passes 102 tests; nine focused
sanitizer suites pass. [History review](native-multi-creature-movement-history-review.json)
accounts separately for committed intermediate versions. Historical evidence for
changed shared sources may remain stale; it is neither refreshed nor promoted by
this native result.

## Remaining boundaries

Original occupancy insertion/removal, pointer/flag lifecycle and original scheduler
semantics remain unrecovered. Fine-space collision, combat, deadlock handling,
mixed creature profiles, captured live entity inputs and live replacement remain
open. The Qt scene still presents the existing single-creature slice; multiple
moving creature presentation is a separate next milestone. This work changes no
original artifacts or gameplay balance.
