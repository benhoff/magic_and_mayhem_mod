# Search reconstruction and caller coverage

2026-10-02. Exact no-CD build SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses are preferred VAs, not assertions about stable live addresses.
No executable patch or live hook was installed for this reconstruction.

## Reproducible evidence

```bash
./tools/verify-decompilation-baseline.py --executable working/game-nocd/Chaos.exe
./tools/export-search-support.py
./tests/test-reconstruction.sh
python3 tests/test-search-support.py
```

The first command checks the pinned 14 raw artifacts and 1,686 assembly byte
chunks. The second verifies the executable hash before exporting support
assembly and reading the file-backed tables, records artifact checksums, and
checks the input hash afterward. Every export has its own directory; this run
is `working/decompiled/search-support-pchlja6y/`. Its manifest contains the
26 neighbor offsets, nine direction entries and 27 local-caller offsets, plus assembly for the helper
routines, constructor and two caller excerpts. The original raw exports remain
unchanged.

## Recovered search behavior

The buildable implementation is `reconstruction/pathfinding/route_search.cpp`
and its public header. This is a host model with 32-bit node tokens and separate
host containers, not a replacement DLL or the engine's memory layout.

| Finding | Evidence | Confidence |
| --- | --- | --- |
| Wrapped signed X/Y delta in `[-dimension/2, dimension/2]`, with an exact half-distance tie kept positive | Helpers `0x0040e4e0..0x0040e50a` and `0x0040eb70..0x0040eb9a` | High, static |
| Distance metric is `2*largest + middle + smallest` of the three signed absolute inputs | All six ordering branches at `0x004eac10..0x004eac9c` | High, static |
| Search heuristic is `20 * metric(2*wrapped_dx, 2*wrapped_dy, dz)` | Register and stack data flow at `0x004ec857..0x004ec8a4` | High, static; not live-validated |
| Priority queue permits duplicate scores, orders scores as signed integers, and inserts equal scores after existing equals | Constructor `0x004bf12f` sets queue `+8` to 1; insertion `0x00430d61..0x00430d78`, `0x00430d7a..0x00430d8b`, `0x00430f31..0x00430f3a` | High for comparisons/duplicate flag; medium-high for equal-priority order |
| Node lookup uses an unsigned pointer-token key and defaults to score `INT_MAX`, predecessor 0, category 5, remaining payload zero | `0x00431171..0x00431178`; `0x00440088..0x004400c4` | High, static |
| Start is queued with score zero, not with its heuristic | `0x0054b965..0x0054b9a3` | High, static |
| Candidate score is `front_score - current_h + edge_cost + neighbor_h`, using DWORD arithmetic and signed improvement checks | `0x0054bac7..0x0054baff` | High, static |
| Better scores copy predecessor and all 28 payload bytes, insert a new queue entry, and update closest node only on strictly smaller heuristic | `0x0054bb01..0x0054bb64` | High, static |
| Old queue entries are not discarded by a stale-score check before expansion | Search loop `0x0054b9c3..0x0054ba6b` | High within this routine |
| One budget unit is charged per expansion; if this reaches zero, discard that expansion's candidates and retain its queue entry | `0x004ec76a`, `0x0054ba70..0x0054ba95` | High, static |
| Budget exit leaves initialization byte zero; normal queue/bound exit sets it to one | `0x0054ba79` jumps past `0x0054bb8e` | High, static |
| Result uses the closest discovered node, follows predecessors through start, skips start on emission, and emits at most 16 waypoints | `0x0054bbb8..0x0054bc1d`, `0x0054bc91..0x0054bd5f` | High, static |
| A waypoint is 28 bytes: XYZ, direction, vertical delta, category-in-1..3 flag, category | `0x0054bd81..0x0054be22` | High for fields/stores; medium for direction/category labels |

The raw heuristic decompilation misleadingly displays only the Z-dependent
return because helper prototypes lose register arguments. The assembly shows
X in ECX and Y in EDX entering the metric. No raw inferred C was rewritten.

The host implementation preserves x86 wrapping score/metric arithmetic without
introducing C++ signed-overflow undefined behavior. It preserves the zero-score
seed, stale entries, partial fallback, budget boundary and output cap rather
than substituting a textbook A* algorithm. The scalar best-heuristic/node/score
fields at context `+0x211`, `+0x215`, `+0x265` are reflected; host container
addresses are never written into the engine's opaque container bytes.

Map dimensions and coordinates must be valid and node tokens nonzero. Missing
callbacks, uninitialized continuation, invalid/cyclic predecessor chains, and
nonadjacent emitted XY jumps are rejected by harness checks. These are not
assertions about engine error handling. Neighbor offsets wrap X/Y, bound Z,
retain table order and retain duplicates on narrow maps. Enumeration alone
does not establish legal moves. Direction emission uses the original nine-entry
table for adjacent XY deltas, retains direction for vertical moves, and writes
zero vertical delta on XY moves. Unused route bytes remain copied from the object.

## What remains unresolved

The [neighbor assembly review](pathfinding-neighbor-review.md) records the
expansion ABI, three control-flow paths, candidate payload fields, cost
arithmetic, and unresolved helpers as the next implementation reference.
The [creature acceptance model](pathfinding-creature-acceptance.md) now supplies
the recovered `0x514360` decision sequence and `0x4f5a40` cell wrapper, with lower
`0x4f3990`/`0x4f47d0` checks still supplied explicitly.
The [lower movement model](pathfinding-movement-test.md) now replaces the
`0x4f3990` callback with recovered control flow, validated against isolated
original i386 code; its deeper checks remain providers. The [cell-footprint model](pathfinding-cell-test.md)
now supplies `0x4f47d0`, with `0x4f3550` supplied by the [occupancy model](pathfinding-occupancy-test.md).

`SearchWorld::expand` must supply legal candidates, their exact edge costs and
their opaque movement payloads. Creature-specific collision checks, terrain,
speed computation and helper internals remain unresolved. The neighbor model
now reconstructs both standard expansion paths and the alternate six-link
branch at `0x004ebb9b..0x004ebd02`, including cost/payload arithmetic, with
explicit callbacks for unresolved helpers. The 26-cell enumeration alone
does not establish legal moves. Cell-token
mapping is supplied explicitly; the model does not assume live cell-table
addresses or fabricate a game map. No heuristic admissibility or optimality
claim is made.

The synthetic tests use explicit graphs to verify search transitions and
byte-level output. Existing live traces validate only the route wrapper with
measured search output supplied. They do not contain the expanded candidates,
queue history or collision inputs required to validate this search core live.
The reconstruction is therefore a tested static milestone, not a complete
live-equivalent pathfinder. The next search work is reconstructing the movement
providers and capturing their inputs/output in addition to the search boundary.

## Why the current traces may favor combat

**Confirmed:** the tracer watches wrapper `0x00512800` and its internal search
call, not every entry to search `0x0054b800`. Static call references show a
second direct search call at `0x005131b3`, with ECX context `0x006c4bd0`, whereas
the wrapper uses `0x00690148`. The direct caller compares cached target/start
state at `0x005130d4..0x00513132`, sets its context initialization byte when
inputs change (`0x00513136`), and uses
separate budget globals at `0x006df188` and `0x006df18c`. Its setup assigns
70 or 210 and subtracts consumed budget after returning. Confidence: high
for this separate cached/budgeted call path; gameplay role remains unconfirmed.

**Confirmed:** `run-99zlrevg` contains eight samples returning to `0x0051cc96`,
one to `0x0050bd13`, and one to `0x005213eb`. Every emitted count is one or two;
each consumes one or two expansion units from a configured budget of 300.
The dominant caller uses an offset table at `0x005e55c4`, adds an offset to the
object's current XYZ, wraps X/Y, and requests the resulting local destination
at `0x0051cc91`. Confidence: high for observed caller distribution and offset
construction; medium for the local-repositioning interpretation. File-backed
table inspection confirms all these offsets have components in -1..1: the
zero offset followed by the same ordered 26-neighbor stencil.

**Hypothesis:** combat generates frequent local reposition requests through
the watched wrapper, while some ordinary movement uses the separate cached
search path or follows an existing route without invoking another search.
This is consistent with the user's observation but is not proof that either
caller is exclusive to combat or player movement. Controlled idle/move/attack
captures at both search contexts are required to label them.

Two independent `SearchState` instances model those two scratch contexts. The
wrapper bridge reinitializes on each route request; direct calls can retain a
zero initialization byte and resume after budget exhaustion. The search core
does not assume that all callers share wrapper lifecycle behavior.
