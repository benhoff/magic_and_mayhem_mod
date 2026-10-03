# Pathfinding reconstruction

This directory contains buildable C++17 host models of route requests, search
control flow, both neighbor-generator branches, creature acceptance and all
movement decision helpers, creature scalar calculation and frozen world replay. They preserve recovered engine rules with explicit
map and object inputs; they are not original source or installed replacements.

The capture controller can now extract real map and creature inputs for offline
search replay. End-to-end live agreement still needs a fresh world capture.
No live hook or gameplay change is installed.

## Three layers

1. Raw C and assembly: `research/runtime/decompiled/nocd/`, checksum-pinned
   to no-CD SHA-256
   `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
2. Analysis annotations: `tools/ghidra/AnnotateRouteMilestone.java`, applied
   only to that exact PE32 build at preferred image base `0x00400000`.
3. Readable models: `route_request.*`, `route_search.*`, `route_neighbors.*`,
   `route_creature_acceptance.*`, `route_movement.*`, `route_cell.*`, `route_occupancy.*`, `route_validity.*`, `route_cell_validity.*`, `route_record_query.*`, `route_cell_support.*`, `route_boundary.*`, `route_clearance.*`, `route_scalar.*`, and `route_world.*`,
   with corresponding `tests/route-*-test.cpp` executables.

The generated Ghidra database stays under ignored `working/`. Scripts and
the readable model stay tracked. Raw exports are never edited or formatted.

## Reproduce

```bash
./tools/verify-decompilation-baseline.py
./tests/test-reconstruction.sh
./tools/decompile-game.py --annotated
```

Build the standalone host library and run all fifteen C++ test executables (plus Python world tooling checks):

```bash
cmake -S reconstruction/pathfinding -B working/build/pathfinding -DCMAKE_BUILD_TYPE=Release
cmake --build working/build/pathfinding --parallel 2
ctest --test-dir working/build/pathfinding --output-on-failure
```

For an x86 host executable, the same sources also build with `-m32`:

```bash
cmake -S reconstruction/pathfinding -B working/build/pathfinding-x86 -DCMAKE_CXX_FLAGS=-m32
cmake --build working/build/pathfinding-x86 --target route-replay --parallel 2
```

The i386 replay build passes the frozen-world integration tests on this system.

This creates `working/build/pathfinding/libmnm_pathfinding.a`. Use
`ReconstructedSearch` with explicit `SearchWorld` callbacks, and pass
`search_backend(search)` to `route_request`, or call `route_search` directly
with separate context/state for a resumable search. See
[search findings and caller coverage](../../research/runtime/pathfinding-search-model.md)
for evidence, fidelity boundaries and unresolved movement dependencies.

Use `NeighborHelpers` with `neighbor_search_world(helpers, target, object_token)`
to connect the recovered creature generator to `route_search`. Map and collision
helpers must be supplied explicitly. For generic descriptors, call
`generate_neighbors` directly; `expand_neighbors` additionally models the
generator's budget guard, append, and decrement. Do not use that budgeted entry
inside `SearchWorld::expand`, because the search core charges the unit itself.
See the [neighbor review](../../research/runtime/pathfinding-neighbor-review.md)
for descriptor offsets, recovered tables and unresolved helper contracts.

Use `with_creature_acceptance(neighbors, acceptance)` to install the recovered
mode-zero acceptance logic. `CreatureAcceptanceHelpers` supplies object state
and the two lower checks (`0x4f3990`, `0x4f47d0`); mode-nonzero requests
retain the previous callback. See the
[creature acceptance findings](../../research/runtime/pathfinding-creature-acceptance.md)
for intermediate-cell groups, bypass rules, and member layout.

`with_movement_test(acceptance, movement)` supplies the reconstructed `0x4f3990`
under creature acceptance. Install `with_standard_movement_test(neighbors, movement)`
before `with_creature_acceptance` to wire the other standard path too; its setup
`NeighborRecord::engine_token` is the source record override. Deeper record,
validity and clearance checks remain explicit `MovementHelpers` providers.
See [movement findings and native x86 comparison](../../research/runtime/pathfinding-movement-test.md).

`with_cell_test(acceptance, cells)` supplies reconstructed `0x4f47d0` through
`CellHelpers`. It preserves the one-cell or ordered wrapped 2x2 footprint checks;
use `with_occupancy_test(cells, map_view)` to supply its reconstructed `0x4f3550`
column scan. `OccupancyMapView` borrows stable cell storage and offset tables. See
[cell findings and native x86 comparison](../../research/runtime/pathfinding-cell-test.md).

See [occupancy rules and native comparison](../../research/runtime/pathfinding-occupancy-test.md)
for flags, occupant tokens, scan clipping and storage lifetime requirements.

`with_validity_test(movement, validity)` supplies the recovered `0x4f46b0`
footprint check and retains the other movement providers. Use `with_cell_validity_test(validity, cell_map)` to supply its recovered
`0x4f3440` rules using borrowed cell/terrain storage for the caller's map context. See
[validity findings and native comparison](../../research/runtime/pathfinding-validity-test.md).

See [cell validity and clearance findings](../../research/runtime/pathfinding-cell-validity.md)
for terrain record layout, flag rules, scan bounds and integrated native comparisons.

`with_record_query(movement, support)` supplies `0x4f4330` source/destination
queries. It preserves single-query forwarding and the four-column support
aggregation; use `with_cell_support(support, cell_map)` to supply the recovered `0x4f3320`
current/below-cell predicate using borrowed map and terrain data. See
[support aggregation findings](../../research/runtime/pathfinding-record-query.md).

See [cell support findings](../../research/runtime/pathfinding-cell-support.md)
for terrain flag `+0xb0`, current/below differences and the five-routine comparison.

Install `with_boundary_test(movement)` after the record and validity adapters to
supply the recovered `0x4f41a0` modes and retain the two clearance providers. It
captures the current support/validity callbacks. See
[boundary findings and integrated comparison](../../research/runtime/pathfinding-boundary-test.md).

`with_clearance_tests(movement, global_map)` supplies `0x4f44e0` and `0x4f45b0`
from the fixed global map. Install it before the validity/boundary adapters when
starting without placeholder checks. All movement decision helpers can now be
supplied by reconstructed rules; world data and creature costs remain inputs.
See [clearance findings and complete movement comparison](../../research/runtime/pathfinding-clearance-tests.md)
for the global/receiver distinction and host cycle guard.

The first two commands verify evidence and test the host-side reconstruction.
The third creates a fresh analyzed project and annotated C export; it does
not replace the raw snapshot. Normal `decompile-game.py` remains an unannotated
export, and `--all` still controls export scope. No commands overwrite earlier
runs. The test requires a C++17 compiler (`CXX` or `g++`), with ASan and UBSan.
Leak detection is disabled because it requires ptrace unavailable in the sandbox.

## Evidence and confidence

| Model | Source VA | Evidence / confidence |
| --- | --- | --- |
| `normalize_coordinate` (X) | `0x0040e290` | Signed-negative addition loop, unsigned-positive subtraction loop; high within positive-dimension domain |
| `normalize_coordinate` (Y) | `0x0040e8a0` | Same operation using a different dimension global; high |
| `copy_z` | `0x0040eeb0` | Copies one DWORD; high for operation, medium for Z label |
| `route_request` | `0x00512800` | Four stack DWORDs, ECX object, local budget, constructor stack tracing, snapshot copy and flag stores; high static evidence |
| Search core | `0x0054b800` | Queue/relaxation/budget/partial-output control flow reconstructed; movement providers available through frozen world replay; live agreement pending |
| Heuristic | `0x004ec780` | Wrapped XY deltas and register arguments into sorted weighted metric recovered from assembly; high static confidence |
| Neighbor generator | `0x004ebae0` | Both standard paths and six-link branch, ordered candidate bytes, limits and budget reconstructed; acceptance/scalar/record helpers supplied explicitly |
| Creature acceptance | `0x00514360`, `0x004f5a40` | Parameter construction, base movement test, destination/intermediate checks and category resets reconstructed; lower helpers remain explicit |
| Cell footprint | `0x004f47d0` | One-cell/2x2 branch and short-circuit order; original i386 routine matches 35,201 fixture cases; occupancy rules supplied by the reconstructed column scan |
| Column occupancy | `0x004f3550` | Low flag bits, occupant comparison, extent and layer clipping; original i386 code matches 9,509 direct and 576 integrated footprint fixtures |
| Validity footprint | `0x004f46b0` | Ordered footprint, caller receiver and parameter forwarding; 35,201 original i386 footprint and 3,072 integrated movement fixtures match; lower rules supplied by the reconstructed cell-validity model |
| Cell validity | `0x004f3440` | Base flags/classification, extent promotion and overhead scan; 101,180 direct/footprint/movement native fixtures match |
| Support aggregation | `0x004f4330` | Single query or balanced four-column support with requested-level hit; 64,522 direct/movement native fixtures match; lower rules supplied by the reconstructed cell-support model |
| Cell support | `0x004f3320` | Current/below terrain predicates, classification and flag eligibility; 253,456 direct/aggregation/movement native fixtures match |
| Boundary predicate | `0x004f41a0` | Mode dispatch, signed boundary arithmetic, support/upper validity calls and parameter rereads; 128,232 direct/map/movement native fixtures match |
| Clearance | `0x004f44e0`, `0x004f45b0` | Wrapped global-map scan, nonzero terrain or unsigned class >=8; 120,842 direct/complete-movement native fixtures match with no instruction redirects |
| Creature scalar | `0x005205b0`, `0x00505840`, `0x00505920` | Table sums, modifiers, signed/unsigned arithmetic, prior-scalar acceleration and x87 conversion; 149,070 untouched original x86 cases match both output DWORDs |
| Lower movement | `0x004f3990` | Categories, boundary restrictions, clearance and intermediate checks reconstructed; isolated original i386 routine matches results/categories/helper order for more than 25,000 cases |

Coordinate labels and object identity are inferred. Unknown argument/fields
retain explicit unknown names. The helper constructors return their destination
pointer in EAX. The annotation script records that return and marks the three
constructors inline **for analysis only**, to expose caller stack-memory effects.
This is not code inlining or binary rewriting.

`0x00690354` aliases `0x00690148 + 0x20c`: the wrapper's flag store affects
the first search-state byte in the shared route context. The model preserves
that alias rather than inventing an independent global. The wrapper does not
reset it after search; search itself can change it. This also means the search
initialization branch is entered when this wrapper calls it, not necessarily
a continuation of a previous search. Other callers remain to be investigated.

## Layout and behavior constraints

- `RouteSnapshot` is exactly `0x20c` bytes; count is at `+0x10`.
- The object's snapshot starts at `+0x96b`, giving count at `+0x97b`.
- Object flags at `+0xb8b` and `+0xd03` retain their byte offsets.
- Search state at context `+0x20c` is outside the copied snapshot.
- All structures are packed, with compile-time offset/size assertions.
  Object/context types are **minimum accessed prefixes**, not complete classes.
- All opaque snapshot bytes are copied, not just known fields.
- Budget is a stack-local copy; changing it does not mutate the configured limit.
- Any nonzero count reports success; the wrapper itself does not cap count at 16.
- The host callback ABI must never be cast to the game's 32-bit thiscall ABI.
  Real hooks need a separate x86 bridge and verified live object lifetimes.

Positive signed dimensions and a non-null backend are host-model preconditions.
Invalid setup throws before changing modeled state. Those guards are NOT
discovered engine branches; this model does not claim equivalence for corrupt
dimensions or a missing backend. No search algorithm, allocation strategy,
priority ordering, or movement rule is changed.

## Validation and remaining work

Tests cover 63,519 coordinate cases, signed extremes with large dimensions,
Z copying, exact callback arguments, budget isolation, aliasing, nonzero/zero
counts, opaque-byte copying and untouched surrounding memory. Compile-time
assertions check binary offsets; ASan/UBSan check the host model.

These tests validate a reconstruction against independently asserted static
observations. They do **not** prove live-game equivalence. Remaining validation:
capture arguments and state for a simple route, blocked target, budget
exhaustion and several simultaneous orders; compare those traces to the model.
The opt-in trace controller below is available; no live-equivalence claim or
replacement hook is included in this milestone.

Actual Ghidra validation on 2026-10-02: annotated run
`working/decompiled/nocd-96ofmeq_/` applied the script successfully, exported
all seven routines without failures, and recorded `annotations_applied: true`
and `input_unchanged: true`. The annotated wrapper now shows the correct
`search_route_candidate(context, object, unknown_argument, x, y, z, &budget)`
call and no longer contains the earlier `extraout_ECX` placeholders. The
remaining search export is still only partially typed. The full tooling suite
and dedicated reconstruction/baseline tests pass.

The search milestone now covers signed priority ordering, duplicate/stale queue
entries, strict relaxation, opaque payload copies, closest-node fallback,
budget exits and continuation, 16-waypoint output, direction/category stores,
and the original weighted heuristic. Sanitizer tests cover blocked and reachable
graphs, seam crossing, vertical moves, tie order, score improvement, stale
entries, exact budget boundaries, continuation, truncation and untouched bytes.
These are synthetic graphs; they do not reconstruct creature movement rules.
The neighbor generator now covers both branches, all movement helpers and the
creature scalar chain. `snapshot_neighbors` wires frozen map, terrain, creature
and independent type records through the complete creature movement/cost path.
Protocol and full-search integration tests use synthetic memory; fresh in-game
route comparisons remain outstanding.
Next live work: trace search internals and both search contexts, rather than
only the route wrapper.

## Frozen world replay

`with_creature_scalar` supplies `0x5205b0`, including its two lower routines.
See [scalar evidence](../../research/runtime/pathfinding-creature-scalar.md).
`RouteWorldSnapshot` owns captured inputs; `snapshot_neighbors` retains that
ownership across the entire movement chain, and `replay_route_world` runs the
reconstructed search. The Python replay launcher checks the companion hash.

```bash
./tools/trace-route-experiment.py --world-snapshot --calls 1 --seconds 180
./tools/replay-route-world.py working/experiments/route-trace/run-REPLACE/world-0001.bin
```

World capture adds longer pauses. Load a map and issue orders once armed.
Replay compares all output route bytes, budget and flag when the completed trace
call is present, and saves a `.replay.json` report. Single-file replay covers fresh creature searches. The sequence mode below
adds direct callers and continuation replay from preceding captured calls. See [protocol and limits](../../research/formats/route-world-snapshot.md).

## Common search and continuation replay

```bash
./tools/trace-route-experiment.py --search-sequence --calls 20 --seconds 300
./tools/replay-route-world.py --sequence working/experiments/route-trace/run-REPLACE
```

This mode watches common search `0x54b800` entry and return, including both known
contexts and unlisted callers. It records refreshed world inputs for every call
and keeps replay state independently per actual context. A nonzero initialization
byte resets state; zero resumes a captured preceding search. Initial mid-search
captures are inconclusive until a fresh call, and stale or mismatched chains are
not reported as validated continuations. It writes `sequence-replay.json` with
route/budget/flag and best-node comparisons. Real-game sequence agreement remains
pending. See [capture and replay evidence](../../research/runtime/pathfinding-search-sequence.md).

## Route wrapper capture and replay

Run `./tools/trace-route-experiment.py` without arguments. It stages a new
disposable game copy and attempts up to ten complete calls within 180 seconds
after arming. Once it prints “Tracing is armed,” load a map/save and issue
movement orders. `--seconds 300 --calls 3` adjusts the bounds; `--prepare-only`
stages evidence without launching. Close other Chaos.exe instances first.
All runs get fresh directories under `working/experiments/route-trace/`.

On x86 hosts the controller defaults to the same architecture-specific prefix
as `run-x86.sh` (`working/wineprefix-x86_64/` on this Arch host), and uses the
system `winedbg` launcher. `--prefix DIRECTORY` overrides `WINEPREFIX`, which
overrides the default. ARM hosts retain `working/wineprefix/` and the existing
i386 PE debugger selection. Each manifest records the host and selected prefix.
For example:

```bash
./tools/trace-route-experiment.py --seconds 180 --calls 10
./tools/trace-route-experiment.py --prefix "$PWD/working/wineprefix-x86_64" --prepare-only
```

Preparation alone is allowed while the game is open; live capture still
requires closing existing game instances.

The controller checks the executable hash, discovers the loaded image base,
requires x86 registers and verifies instruction bytes before installing four
temporary debugger breakpoints. It pairs entry, search-call, search-result and
return observations by thread and stack address. Logs and manifests preserve
failures too. Successful cleanup removes breakpoints and detaches, including
after recoverable capture errors. `--terminate-on-error` restores termination
of the experiment's game on failure. If cleanup cannot restore/detach safely,
the controller still terminates its owned debugger group. A `cleanup_unverified` result needs
manual inspection. It never patches the executable on disk or kills wineserver.
Debugging pauses execution, so these captures cannot benchmark performance.

`./tools/validate-route-trace.py` defaults to the newest run with calls.jsonl.
It builds the C++ replay and compares normalized coordinates, arguments, budget,
flags, return value and the entire copied snapshot. The original search output
is supplied as measured input: a match validates only the wrapper, **not the
pathfinding search algorithm**. Reports preserve trace/model hashes and capture
origin; no samples is an unsuccessful result. Manually label scenarios rather
than assuming an AI call belongs to your order.

Earlier ARM64 live limitation, observed 2026-10-02: on the ARM64 Wine 11.18 staging
setup, both native WineDbg and the installed i386 PE debugger expose a context
without `$eip`. Native DLL overrides and a uniquely named debugger copy did not
resolve it. Latest evidence: `working/experiments/route-trace/run-0yycduv3/`;
status incomplete, image base 0x00400000, no breakpoints installed, own game
terminated, both input and copied executable hashes unchanged. There are no
live route samples yet. A debugger/runtime combination exposing the target's
x86 context is needed before live validation can proceed; the cause of this
runtime limitation remains unconfirmed.

On the x86-64 Arch host with Wine 11.16, a bounded setup check on 2026-10-02
read `$eip`, discovered the loaded image at `0x00400000`, verified instruction
bytes, installed all four breakpoints and printed “Tracing is armed.” Wine's
memory output includes padded addresses and module labels; the parser now
accepts that format. Evidence: `working/experiments/route-trace/run-ccmzm3z4/`.
No map was loaded during the ten-second check, so no complete route samples
were obtained and the run correctly ended incomplete with exit status 2.
Cleanup deleted all four breakpoints and terminated only the experiment's game;
both executable hashes remained unchanged. Confidence is high for x86 setup;
end-to-end live route capture/replay still needs movement orders on a map.

The first in-map x86 attempt (`run-fnmyb93f`) stopped at the route-entry
breakpoint but failed parsing `x /1x`: WineDbg printed the valid dimension
`00000050` without an address prefix. The old failure cleanup then deliberately
terminated the game; this log shows a controller error, not an engine crash.
Single-element byte/DWORD output is now accepted only as one complete value;
ambiguous output and debugger error messages still fail closed. Regression
tests cover this response and cleanup that removes breakpoints before detaching.
Live singleton reads of both dimensions, the configured budget, and the search
state byte succeeded with the repaired parser in
`working/tests/singleton-probe-4o5cw9cm/`. That startup probe had zero dimensions
because no map was loaded; it verifies parsing, not route semantics. The probe
explicitly terminated its own game after reading. All ten synthetic tests pass.

First complete live x86 samples: `working/experiments/route-trace/run-66lrvexa/`
contains two complete calls with an 80x80 map, configured budget 300 and measured
remaining budget 299. Both match the wrapper model across arguments, flags,
return values and the full copied snapshot; search outputs remain measured
inputs, so this does not validate the search algorithm. Confidence is high
for those two wrapper observations; their gameplay scenarios are unclassified.
The ten-call capture was incomplete because WineDbg reported
`Process of pid=0134 has terminated` before the next register read. The log
does not establish why the game exited. Earlier controller output misleadingly
called this an x86-context failure and recorded a detach despite `No process
loaded` responses. Target exit is now detected at the debugger prompt and
cleanup records `target_already_exited`. All twelve synthetic tests pass;
replaying this entire recorded debugger session through the updated controller
reproduced both saved calls exactly and correctly classified its terminal event.

`python3 tests/test-route-trace.py` checks parsers, fail-closed architecture
handling and synthetic replay/mismatch cases. Synthetic results are not game
evidence. See [WineDbg's command reference](https://raw.githubusercontent.com/wine-mirror/wine/master/programs/winedbg/winedbg.man.in)
for the debugger command interface used by the controller.
