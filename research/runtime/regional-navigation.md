# Regional navigation

NS72. Build `nocd-40209ca7` only. Static discovery, isolated comparison,
headless integration and live replacement are separate milestones. Prospective
execution declarations bind this scope before the final comparison.
These are battlefield navigation nodes, separate from campaign regions and
rendering terrain regions.

The recovered actor request `5128a0` delegates region lookup to `53d7d0`,
regional search to `53e5e0`, edge distance to `53ea50` and height candidate
selection to `513cf0`. Region records have stride 660, position at +4/+8/+12,
18 inline 28-byte connections at +16, and a search traffic counter at +528.
Connection admission is recovered; graph production remains separate.

`tools/export-regional-navigation.py` exports the pinned read-only bodies and,
optionally, pseudocode from the existing read-only Ghidra project. Confidence
is high within the compared domain after the linked execution succeeds.

## Recovered contracts

- `53e520` indexes regions sorted by X. It clamps the high endpoint to the last
  record; substituting a conventional lower_bound with an end iterator changes
  the seam lookup. `53d7d0` scans from wrapped X-4 through X+5, admitting at most
  fifteen legal records. Its asymmetric distance formula includes half the
  absolute height difference only when Y separation is at least X separation;
  the second wrapped prefix omits height. Directional hints change ranking
  after the initial distance<=10 filter. The original maximum-selection
  shortsort and middle-pivot quicksort preserve its equal-score ordering.
- Lookup tries `4f4900` terrain legality, then `4f4ba0` direct lines in ranked
  order. Failed direct lines fall back to independent twelve-expansion tile
  searches through `4ebae0`. The final expansion is discarded at budget zero.
  Movement-kind2 candidates below the boundary use boundary minus actor height.
  The statistical lookup globals are observational counters, not native state.
- `53a380` reads eighteen connections in order. Each 28-byte connection holds
  signed cost, destination index and twenty signed clearance bytes. Clearance
  groups at +8/+11/+14/+17/+20 model the alternative movement paths; group+22
  supplies the movement-kind1 lower bound. Size/height comparisons are signed.
- `523070` reads owner-indexed seven-byte danger rows at region+532, optionally
  subtracting the corresponding row at +588. Flag104 and region parameter
  choose the first component; physical state gates component2; components5/6
  use explicit power/bravery ratios. The final byte at +644 always contributes.
  Native risk powers and danger rows are supplied owned inputs; production of
  these inputs by the original World is excluded.
- `53e5e0` uses heuristic `4*distance_metric(2*wrapped_dx,2*wrapped_dy,dz)`.
  It seeds priority zero, permits duplicate queue entries and FIFO equal
  priorities, relaxes strict improvements, and tracks the closest discovered
  node. Danger weighting multiplies edge cost by `10*danger+1`; traffic weighting
  multiplies by `2*(source.traffic+1)`. Cumulative raw cost/danger payload words
  remain separate from weighted priority. Completed, unreachable and exhausted
  searches publish paths; exhausting a budget retains the current queue entry
  and discards that expansion's candidates. The return value reports reaching
  the goal even if the stored path truncates to fifty entries. Traffic counts
  increment only the stored prefix of a successful marked query.
- `513cf0` preserves the flight-clearance loop and candidate-height adjustment,
  followed by movement-kind2 boundary correction. `NativeCreatureType` carries
  explicit recovered +40 clearance; ordinary existing profiles retain zero.
  Nonzero values join the immutable profile stamp. General species/config
  import and complete flying movement remain outside the coupled journey scope.
- `4f4900` combines validity, clearance, support, movement-kind2 boundary and
  flag104 branches without occupancy. `4f4f90` first descends empty unsupported
  columns, then tests the original ordered offsets with a one-cell free probe
  where needed. That probe sets flag104=1 and therefore bypasses support;
  validity and clearance still apply. Region entry and endpoint adjustment use
  these providers.

## Native composition and continuation

The owned request binds lookup, search, edge distance and waypoint providers.
Segment completion receives actual region positions/parameters. The scheduler
uses native lookup when regional mode is enabled; preparation enables its
regional update flag. The recovered `51b300` displacement probe selects the
current or next regional waypoint and commits its pending/cursor changes only
under the original success branch while restoring all saved actor route/motion
words. Shared tile search and budgets retain their original continuation rules.

CMD1 version7 appends region records, the complete owned graph search queue and
records, and risk powers. Actors already carry fifty-entry paths and cursors;
the request metadata carries their final destinations. Version1..6 readers
retain an empty graph, and pre-v7 writes refuse to discard regional state.
Graph indices, sorted/canonical positions, positive nonzero edge costs, queue
roots and bounded storage are native admission policies. An enabled empty graph
can fall back to local routing. Original invalid pointers, arbitrary-width
clearance reads and noncanonical region coordinates are excluded.

The native map projection now admits XY dimensions4..64 with a65536-cell limit.
The command wire retains its independent one-MiB total limit. The integration
fixtures use32x32x23 installed Celtic Plains geometry with sealed boundaries.
Their declared navigation graphs derive connections from bounded tile searches
and are identical inputs to both implementations. This preparation is a test
policy; it does not establish original graph construction or full-map World
loading. Actor/grid projections and traffic counters are compared each tick;
native64/sanitized traces and final checkpoints agree with the original32 pair.

`tools/test-regional-navigation.py` retains original32/native64/sanitized helper
outputs, four installed journey scenarios, checkpoint traces, source/input
fingerprints, static exports and before/after immutable manifest verification.
The scenarios cover regional travel, a wall opening, an unreachable partition
and friendly displacement on a retained regional leg. The helper suite also
checks budget0/1, search resumption, the fifty-region truncation, endpoint
adjustment, v6 migration, malformed reference refusal and native unfinished
graph-search restoration. Existing nonregional native CTests and installed
encounter regression run separately in the same report. Graph production,
dynamic World danger updates, campaign and live replacement remain pending.

The regional displacement adapter requires candidate indices below50.
Alias-derived lookahead at the tail of a truncated actor path remains an
admission/integration boundary; the standalone graph search's fifty-entry
truncation is compared separately. Full original heap/actor bytes, observational
lookup statistics and dynamically produced World risk rows are not projected.

## NS72 recorded execution

The [final source-bound report](regional-navigation-bound-ns72-20261015.json)
passes258440 helper checks,8192 original comparison ticks,1052 checkpoint
restores (28 fresh native processes), native64/sanitized trace and final-snapshot
agreement, the five relevant command CTests and installed3072-restored-tick
regression. All238 declared source hashes remain unchanged. Original executable
text, installed inputs and immutable originals verify unchanged. This supports
only the two declared bounded regional contracts, preserving older evidence and
all documented World/graph-builder/alias-tail/live boundaries.

NS73 separately closes static graph construction and the active-region49
lookahead boundary within its declared installed-terrain domain. See the
[construction contract and execution](regional-graph-construction.md). NS72's
supplied-graph evidence above remains historical at its original scope; dynamic
graph repair, World danger production and full World/campaign/live integration
remain separate.
