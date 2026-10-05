# Native single-creature movement and checkpoint continuation

NS02 implemented 2026-10-04. This is an offline move request → recovered route
→ native tick position progression → checkpoint → map rebind → continued
movement milestone. It does not establish original motion/animation equivalence
or live replacement. The original game retains live gameplay ownership.

## Implementation and ownership

`game/simulation/movement` owns typed actions/routes/cursors, a native navigation
interface and `MovementSession`, without Qt, Wine, hook or reconstruction
dependencies. `apps/world-sandbox/frozen_navigation` composes that interface with
`snapshot_neighbors` and `replay_route_world`: existing cell/footprint/validity/
support/boundary/clearance/creature-acceptance, scalar, neighbor costs and search.
The adapter fingerprints the exact owned buffer passed to the new byte overload
of the existing `MNMWLD01` decoder. Captured pointers remain tokens.

Admission is bounded to one moving creature/profile and 4,096 frozen map cells.
Per-query owned inputs replace the selected creature's XYZ; other captured
profile/world inputs remain fixed. There is no installed MAP world assembly or
dynamic legacy occupancy update in this slice.

## Explicit native policies

Orders enter the tick command boundary. Decisions plan; subsequent ordinary
maintenance advances one waypoint after another recovered edge-acceptance check.
The exact destination must be reached by search; an unreachable closest-node
fallback does not move the actor. Budget exhaustion produces `searchLimited`,
without retaining an unsaved heap search queue. The per-order budget defaults
to 300; original shared-budget debit/resume/stagger selection is absent.

The recovered sixteen-point output cap is retained. A consumed prefix ending
short of the destination triggers a fresh plan from the new position. Order
replacement prevents old-route advancement after admission. Alternate mode skips
movement; search suppression defers planning. Cleanup cancels motion and release
removes it. Optional goal handles capture target coordinates on admission and
cancel an active move on release; they do not continuously chase a moving target.

Action names, exact-target admission, generation guards and one-waypoint-per-tick
pacing are native policies. [Original creature findings](creature-ai-combat-spells.md)
map order/destination dispatch and action/route decisions, but complete original
order eligibility and motion action `0x005104b0` remain unimplemented. Fine
coordinates, speed integration, animation, health/death gates and live cadence
are not modeled. Stored scalar metadata is not used as a native movement clock.

The recovered destination special case can bypass occupancy with object `+0xd03`
zero. The blocked fixture uses confirmed cell-validity bit `0x4000`, preserving
that special case rather than inventing blanket occupancy admission.

## Persistence and resource commit

[Native snapshot v2](../formats/native-world-snapshot.md) stores map identity/
dimensions/content fingerprint, motion origin/destination/route/cursor/budget/
goal, and pending moves. Lifecycle-only v1 wire bytes remain unchanged. The
larger typed records increase resource accounting to 256 bytes per slot/command,
plus exact blob/route storage; this is a native limit policy, not a host layout.

`MovementSession::restore` decodes independent state, resolves an owned map,
compares binding/profile, validates every saved route edge, then commits map and
world together. Missing/changed resources, invalid progression or rejected
edges preserve the running world/map. Numeric structural validation alone does
not establish recovered direction/category/edge agreement. Pending internal
motion updates are forbidden; future updates derive from saved route/cursor.
FNV identity checks detect corruption and are not authentication.

## Evidence and confidence

Final normal Debug and ASan/UBSan CTest runs each pass **22 tests**: four native
world/movement tests and eighteen reconstruction/tooling regressions. Leak
detection is explicitly disabled because LeakSanitizer cannot operate under
environment tracing. Confidence is high within tested native/synthetic scope.

The native C++ fixture checks typed actions, queue replacement, phases, admission,
alternate/search suppression, goal/actor release and slot reuse, blocked/current-
edge rejection, limited searches, zero-distance, prefix replanning, callback
rollback and atomic map restoration. Its mock navigation is separate from
recovered behavior.

The independent Python test runs the real frozen reconstructed adapter, checks
complete expected v2 bytes before planning/mid-route/after arrival, imports a
Python-generated moving checkpoint, and compares every remaining position/action
trace. Final bytes agree for 8 uninterrupted versus 2+6 resumed ticks. A 17-step
route crosses the sixteen-point cap and agrees for 20 versus 8+12 ticks. Other
checks cover seam directions, zero-distance, invalid terrain, missing/changed
maps, every incomplete prefix, a bit flipped at each byte position, malformed
progress/count and wrong direction with recomputed checksums. Failed resumption
publishes no output. These fixtures consume no original artifacts.

Fresh unchanged original i386 comparisons passed **120,842** movement-helper
cases (51,204 any-terrain, 51,206 tall-terrain, 18,432 full movement chain) and
**149,070** scalar/table/modifier/acceleration cases matching both DWORDs.
Two one-second original cyclic-scan probes preserve the existing host guard
boundary. Original No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The hash-pinned runner uses a disposable copy and launches no game. All **2,927**
original files verified before/after. The first clearance attempt stopped with
SIGSYS inside the sandbox; retry outside passed. Failed and successful attempts,
source/build hashes, CTest logs and retained demo checkpoints/traces are in
[the companion report](native-creature-movement.json).

Helper agreement does not establish original whole-route, move-action or live
motion equivalence. The new pipeline has synthetic integration evidence and
explicit native execution policies.

Reproduce native checks/demo using [game instructions](../../game/README.md).
Sanitizer commands in [foundation evidence](native-world-foundation.md) now also
build/check the movement adapter and reconstruction targets. For original helper
checks on a host permitting isolated i386 execution:

```sh
./tools/original-manifest.sh verify
python3 tests/test-native-movement.py --component clearance
python3 tests/test-native-movement.py --component scalar
./tools/original-manifest.sh verify
```

Preserve the post-experiment manifest check if a comparison fails.

## Remaining boundary

No balance change, live hook or rendering replacement is introduced. Autonomous
AI, full original motion/action/animation and order eligibility, dynamic multi-
creature occupancy, shared scheduling/search continuation, combat/spells,
campaign triggers, legacy saves and general resources remain separate milestones.

Next: recover/independently compare original motion-action fine-position, speed
and route-consumption transitions before replacing native waypoint pacing. Then
integrate dynamic occupancy and additional creatures with checkpointed scheduling.
