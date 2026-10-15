# Terrain-derived navigation graphs (NS73)

Build `nocd-40209ca7` only. These are battlefield navigation nodes, not campaign
or rendering regions. Original comparison, headless integration and live
replacement remain separate. The execution declaration is prepared before the
source-bound run. Dynamic graph repair, World danger updates, full World/campaign
construction, other species' coupled execution and live replacement are excluded.

## Construction recovered from the original

The World startup caller470950 calls53ab20(1),53d710,53bb40(1,0) when regional
navigation is enabled and the first record is inactive. This establishes the
static construction sequence; it does not recover the rest of World startup.
`tools/export-regional-graph.py` exports pinned disassembly and optional Ghidra
pseudocode without modifying the input executable/project. The inventory is separately re-exported after identifying53d6f0; the historical
inventory is retained and the discovery delta is reviewed before publication.

- **53ab20 placement:** scan X and Y from2 in steps of5, then Z upward. The
  first-cell predicate is direct4f3440 with height1 and special-byte0. A column
  resumes scanning at nonempty terrain, adjusts admitted points with53b600,
  and may suppress a redundant empty point when a width2 free probe can join
  the previous point. The last retained point in a completed column gets
  parameter1. A full-capacity return occurs before that column-final write.
  After sorting, scan every column for cells not represented by lookup53d7d0;
  prefer a height4 ground probe, then height1 ground or movement-kind3. A new
  column's remaining vertical runs are scanned and the array is sorted again.
- **53b600 adjustment:** choose the first legal height/mode combination in
  height4 then height1 order, each trying ground, kind3, then flag104 free
  movement. Keep a width2-legal point; otherwise try the26 original offsets
  in order, requiring movement and width1/2 legality. This helper rejects XY
  offsets outside the map rather than wrapping them.
- **53d710/53d6f0 sorting:** active key is `X*(width*height)+Y*width+Z`.
  Unused records use `(width-1)*(width*height)+(height-1)*width+layers`.
  Sort the full capacity, including unused records, with the recovered MSVC
  shortsort/quicksort ordering. Duplicate active keys retain its resulting tie
  order; native `std::sort` is not substituted.
- **53bb40 static connections:** clear the18 connection slots, enumerate each
  pair once, and use wrapped XY distance `(2*max(dx,dy)+min(dx,dy))/2`.
  Distance>=9 refuses; distance>5 also refuses endpoints whose kind3 legality
  differs.53c320 computes one descriptor for both directions. Existing matches
  take priority over empty slots; publish only when both endpoints have room.
  The incremental repair path using53c070/53f710 remains separate.
- **53cd30 height selection:** collect distinct configured heights, optionally
  filtered by width and movement kind; creature22 is skipped. The minimum is
  first, maximum second, and remaining values retain their discovery/swap order.
  Storage holds six entries. Native CFG import uses the existing recovered
  height1..5/width1..2 normalization, so valid imported tables fit this storage.
- **53c320/53ce50 clearance:** try width2 then width1, cache successful limits,
  reuse ground limits for free/water alternatives, and take the recovered
  maximum-height ground fast path when possible. Nonzero map boundaries enable
  additional kind3 and kind1 searches, including kind1 lower limits initially6.
  The exact20-byte result is retained; no invented creature-category schema
  replaces it. Comparisons cover all slots, including zero-cost/unused bytes.
- **542220 and fallback:** greedy traversal scans ordered adjacent cells and
  accepts strict heuristic improvements, excluding its previous point. The
  greedy ranking doubles Z; the accumulated edge metric doubles XY and is
  multiplied by4. Movement probes consume1/26 of a budget unit, rounded by
  signed division. An overly indirect success falls back to the original
  generic mode-one search. Ground attempts start at125 units, alternatives62;
  the scoped comparison uses the original adjustment global at zero. The
  generic search preserves FIFO equal priorities and discards the final budget
  expansion's candidates. Connection cost uses the first successful complete
  path, independently of the later clearance cache.

Static findings are supported by the pinned function bodies and bounded
original-backed development comparisons. Final status/freshness is determined
by the immutable source-bound execution report, not by the presence of this code.
Full heap/actor bytes, statistics and invalid original pointer domains are not
claimed equivalent.

## Native ownership and admission

`reconstruction/creature-ai/regional_graph.*` owns the algorithms with explicit
terrain/movement/search services. `apps/command-session/native_regional_graph.*`
binds owned map geometry and a generic descriptor search, independent of actor
movement speeds, Qt and Wine. Construction builds privately and publishes only
a completed graph. Nonempty graphs, initialized regional searches or retained
regional routes refuse reconstruction; dynamic repair is not silently simulated.
The existing4096-region bound and CMD1/v7 byte limit remain native policies.

`apps/native-encounter/navigation.cpp` imports only height, width and movement
kind from the retained encrypted CFG. This is not complete creature construction.
The explicit CLI entry is:

```sh
native-encounter run-regional INSTALL X Y WIDTH HEIGHT SEED TICKS NEW_SNAPSHOT
native-encounter resume SNAPSHOT TICKS NEW_SNAPSHOT
```

Generated nodes/connections and all search state use existing CMD1/v7 storage.
A fresh native process resumes from embedded inputs/checkpoint bytes without
reopening installed files. CLI construction and portable ownership are native
policies, separately registered from recovered original algorithms.

## Fifty-entry displacement tail

At cursor49,51b300 reads the next DWORD at actor+dcf. That address is also the
regional cursor field itself, so the next region ID is49. Native displacement
models this field alias explicitly without an out-of-bounds array access.
The coupled case requires region49 to be active and the current cursor to be
within0..49. Invalid/inactive references and larger cursors retain their native
admission boundaries. Success-dependent pending/cursor publication and saved
actor-route restoration remain unchanged.

## Validation protocol

`tools/test-regional-graph.py` retains fresh original32/native32 comparisons,
native64 and ASan/UBSan/LSan64 outputs, compile/source/input fingerprints,
static exports, traces, snapshots and immutable manifest checks. Original text
is guarded after each original-backed process. Original construction helpers
are not redirected; only isolated allocation and presentation endpoints are
supplied. Graphs are independently generated from each implementation's terrain.

Eleven helper fixtures cover installed32x32x23 Celtic Plains terrain, a wall with
an opening, a sealed partition, an overhead obstruction, boundary0/6 and
capacity1/17/52/512 exits. Each runs configured-height selection,512 ordered
adjustments,512 direct-probe return/cost/budget comparisons, full generated
node/parameter and connection/clearance comparison, and portable publication
checks. Full-capacity duplicate-key sorting is compared separately where32
records fit.

Five journey fixtures cover ordinary travel, a wall opening, an unreachable
partition, friendly yielding and cursor49 alias lookahead. Both sides build
and compare graphs before advancing. Endpoint selection requires legal ordinary
creature positions and admitted graph connections; it does not change gameplay
balance. Per-tick actor/ANI/motion/routes/shared budget/R250 state, all23552
terrain/occupancy cells and graph traffic counters compare. Native64/sanitizer
traces and final snapshots compare against that pair, with every16th-tick
restoration and separate fresh-process chains. Existing NS72 declared-graph,
regional helper, command and installed encounter regressions run separately.

Prospective declarations and final execution outcomes are retained under
`working/tests/regional-graph-bound-ns73-20261015-*`; a successful result is
published under a new durable evidence ID. Historical evidence is not refreshed.

An initial development journey selected endpoints using every connection,
including free-movement-only edges. Both implementations stalled identically;
that run (`working/tests/regional-graph-journey-dev-v1`) is retained and is not
arrival validation. The final fixtures require ordinary-creature legal endpoints
and clearance-admitted edges, then independently compare actual arrival. This
changes fixture selection only, not the recovered connection rules.

## Recorded execution

The [immutable NS73 report](regional-graph-bound-ns73-20261015.json) passes250067
construction/helper checks across11 fixtures,534 generated nodes and3652 directed
connections. Independent original32/native32 journeys pass10240 ticks across five
scenarios and two seeds. Native64 and ASan/UBSan/LSan64 traces and final snapshots
match. There are1315 checkpoint restores:1280 in-process and35 fresh processes.
The CLI separately generates52 regions, completes1024 ticks and resumes from an
initial checkpoint in a fresh process to an identical final snapshot.

The258440-check NS72 helper regression, four1024-tick declared-graph original
comparisons, both five-test command suites and both3072-restored-tick installed
encounter regressions pass. All247 prospectively declared sources,239 compiler/
configure inputs and installed artifacts stay unchanged. Original executable
text is guarded and all2927 immutable originals verify before/after execution.
Only the three declared NS73 contracts receive current scope-bound validation;
broader historical contracts retain their independent pending boundaries.
