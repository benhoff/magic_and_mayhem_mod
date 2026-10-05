# Multi-block terrain admission and candidate pruning

Selected No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This milestone extends the recovered seeded-selection foundations with owned
1x1/2x1/1x2/2x2 descriptors and internal connector counters, original multi-block
admission/carry, candidate/location tables and three-pass candidate pruning.
It is an offline reconstruction milestone. The authored-region preview and its
accepted recipes remain unchanged; a complete generated scene still needs the
placement/backtracking/retry driver.

## Selected original contracts

| Address | Operation |
|---|---|
| 0052edf0 | Build a descriptor for one source block, assigning connector labels |
| 00531a50 | Find another descriptor sharing an internal connector and placed count |
| 005300a0 | Check anchor admission, then walk the connected blocks |
| 0052f790 | Build compact ordinary/special candidate and rotation tables |
| 005310f0 | Build compact location and rotation tables |
| 0052fb00 | Prune tried rotations, equivalent edges and saturated descriptors |

The prior RNG/selection contracts are in
[seeded selection](terrain-region-selection.md). Original descriptor/grid
layouts and authored placement evidence are in
[authored regions](terrain-authored-regions.md).

`terrain_constraints.hpp/.cpp` consumes native MAP header storage and the
owned selection grid. It has no Qt, widget, Wine or runtime-hook dependencies.
`RegionBlockDescriptor` stores source block coordinates, block dimensions and
layers separately from its selection fields. `RegionDescriptorBank` owns up to
50 descriptors and a caller-supplied threshold distinguishing internal labels.
Returned candidate/location tables use the already recovered seeded chooser.
No host pointer or original packed object is exposed to application code.

## Descriptor expansion and connector lookup

Expansion visits source rows, then columns. MAP header outer labels and source
block dimensions have the selected meanings documented in
[region recipes](../formats/region-terrain-recipes.md). The four original
connector counters at receiver +10/+14/+18/+1c provide internal pairs and
advance independently by four when their matching block is described:

| Counter | Source connection |
|---|---|
| +10 | Upper-left E / upper-right W |
| +14 | Upper-right S / lower-right N |
| +18 | Lower-left E / lower-right W |
| +1c | Upper-left S / lower-left N |

The counter values and threshold are caller inputs. Fixture values are above
the installed outer labels; the original configuration caller's threshold
calculation is not reconstructed here. Native expansion owns each descriptor
and updates the supplied counter state only after validation succeeds.
Specific maximum one, random caller maxima and single-block special-edge
classification retain their prior contracts; multiblock special flags are zero.

Connector lookup matches the label and the anchor's original placed count,
excluding the current carry descriptor. It does not filter section ID.
The north-edge scan retains its **last** match; if none exists, east, south and
west scans retain their first match in that priority order. This asymmetry is
confirmed with duplicate-label fixtures against the original helper.

## Ordered admission and carry

Initial anchor admission retains the original occupied-slot, count-equality,
neighbor-sentinel and one-axis checks, including the west sentinel asymmetry.
Rotated source dimensions must fit the block grid. Failed initial checks return
without walking. Single-block descriptors likewise finish at the anchor.

For a multiblock descriptor, the original marks the anchor visited and runs
`block-columns*block-rows - 1` passes. Each pass tests internal edges in
north/east/south/west order using the **current** carry. Neighbor positions wrap.
An unvisited, empty neighbor loads the matching descriptor, rotates it by the
original anchor rotation, checks its facing neighbor edges, and marks it
visited. These subsequent checks use the ordinary facing-edge sentinel on all
four sides. The carry can change several times within a pass.

An occupied internal neighbor marks failure and moves the walk coordinates,
but retains the previous carry and does not mark the blocked cell visited.
Walking continues after failure. Native admission preserves this ordering and
returns the final section ID, descriptor index, rotation and four edge labels
as an owned `RegionAdmissionCarry`. The fixture compares all seven original
scratch DWORDs at +3998..+39b0, as well as the admission result. Descriptor
counts and grid contents remain unchanged.

Missing connectors are refused by native bounds policy rather than permitting
an original descriptor index of -1 to be dereferenced. Descriptor/counter/grid
validation is native policy; complete original error handling is not modeled.

## Three-pass pruning

Pruning operates on a local owned table and returns a new compact table. The
selected original uses 50 packed records with an empty termination slot.
The native interface accepts at most **49** candidates for this operation.
The prior candidate builder and chooser still support 50 entries; a future
solver must handle this explicit pruning bound.

1. Clear the requested rotation on entries matching the tried descriptor, or
   remove that descriptor for requested rotation -1. When the original shifts
   entries in this first pass, it proceeds to the next index without revisiting
   the newly shifted entry. The native implementation preserves that behavior.
2. Compare every descriptor's edge signature under each rotation with the tried
   descriptor's unrotated signature. Clear the corresponding relative rotation
   even for another section/descriptor. Revisit shifted entries after removal.
   For request -1, rotation zero produces relative -1 and clears no bit;
   rotations one through three clear bits zero through two when signatures
   match. This is recovered behavior, not an invented all-rotations policy.
3. Remove candidates whose placed count equals their maximum, revisiting shifted
   entries. Clear the final packed record unconditionally.

The native model preserves duplicate entries, scan order, equality comparisons
and relative rotation behavior. Synthetic fixtures exercise empty tables,
49-entry tables, symmetric/equivalent edges, duplicate descriptors, all rotation
requests and counts below, equal to and above the occurrence maximum.

Static inspection of the shift/revisit loops shows a termination hazard when
all 50 records are active: removal can repeatedly duplicate the unchanged final
record without obtaining an empty sentinel. A preliminary full-table fixture
also failed to terminate in the native direct transcription and was stopped.
This is evidence for the conservative **native 49-entry policy**, not a claim
that an original live map generation hang was observed. Full-table original
pruning execution and general solver behavior are outside validated coverage.

## Validation and reproduction

```sh
python3 tools/test-terrain-constraints.py
```

The wrapper guards original files before/after, checks the executable hash,
independently unpacks all 683 installed MAP headers, records hashes, and calls
unchanged helpers from a private PE32 mapping. Native fixtures run both beside
the original and separately under ASan/UBSan. The selected seeded chooser uses
the private FS exception-head setup already documented for selection tests.
There are no executable patches or Win32 calls on these fixture paths.

All four shapes, block-grid dimensions 1..5, every anchor and all four rotations
are checked with empty, occupied and partially constrained grids. Complete
candidate/location tables include initialized unused slots. Admission checks
compare the original carry, and verify that descriptors and grid are unchanged.
Pruning checks every packed table field, including unused slots, and verifies
unchanged descriptors/counts. Native APIs intentionally do not reproduce the
pruner's scratch carry side effects.

Installed headers run through the unchanged descriptor builder in Specific and
Random forms. Selected source coordinates, dimensions, layers, selection
fields, every edge and all four post-expansion counters are compared. Each
installed descriptor is also admitted in all four rotations on an empty 5x5
grid, comparing final carry. Fixture section IDs are deterministic caller data,
not serialized MAP fields. Whole-CFG loading and generation are not executed.

Validation passed: 683 installed MAPs/1,736 expanded descriptor comparisons;
39,344 admission/carry checks (18,150 admitted); 2,048 connector lookup checks;
7,200 candidate and 500 location tables; 7,200 seeded candidate choices; and
8,192 pruning fixtures. All nine reconstruction CTests passed in each normal
and ASan/UBSan build. Original manifest verification passed before and after.

Counts and provenance are recorded in
[the companion report](terrain-region-constraints.json). Normal and sanitizer
reconstruction CTests cover the new owned interfaces plus existing selection,
authored assembly, section copy, geometry, camera, traversal, queue and visibility.

Confidence is high for the bounded descriptor/admission/table/pruning domain.
Native bounds are at most 50 descriptors, block shapes 1x1/2x1/1x2/2x2, grid
dimensions 1..5, IDs 0..99, maxima 0..999, rotations 0..3 (-1 for location/pruning
requests), positive connector labels/counters without overflow, square source
tile sides, extents at most 128 and layers 1..32. Returned storage owns its data.

Remaining work is Specific placement/fallback and count mutation, solver
backtracking, retries/RNG attempt progression and error handling, conversion of
generated assignments to the owned assembly plan and scene comparisons. Live
integration, entities, lighting and water remain separate.

The subsequent [placement/solver milestone](terrain-region-solver.md) adds
count mutation and backtracking, and separately validates the detailed pruning
carry needed by that driver. The basic pruning API retains its table-only result.
