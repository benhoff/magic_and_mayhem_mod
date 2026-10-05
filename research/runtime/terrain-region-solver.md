# Owned terrain placement and one-attempt backtracking

Selected No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This milestone reconstructs section placement, cell/count rollback, pruning
carry and the one-attempt region solver over an already described catalog and
optional preplaced Specific sections. It is offline evidence. Native recipe
loading, the Specific-placement/fallback driver, ten-attempt seed progression
and generated scene production remain separate from this recovered solver.

## Selected functions and storage

| Original address | Selected contract |
|---|---|
| 0052d440 | Initialize grid fields; leaves source block Y untouched |
| 00531980 | Write current carry into a slot and increment its descriptor count |
| 00531b20 | Place a section by walking its internal connectors |
| 0052d480 | Clear all fields of one slot; caller separately decrements count |
| 0052fb00 | Prune a candidate table, leaving the last processed scratch carry |
| 00532030 | Fill empty slots, retreat/prune/reselect on failure |

The foundations are documented in
[seeded selection](terrain-region-selection.md) and
[multi-block constraints](terrain-region-constraints.md).
`terrain_solver.hpp/.cpp` owns a selection grid, descriptor bank, optional
per-cell descriptor/rotation assignments, retained candidate tables and optional
scratch carry. All per-cell native storage is indexed `column*rows + row`.
It does not share original packed objects, host pointers, Wine state or widgets.

The original grid initializer does not write source block Y (+39c5 in the first
slot), although it initializes the other selected slot fields. Its cell-clear
helper later sets that field to ffffffff too. Native `storedSourceRows` preserves
this distinction: new owned storage uses **zero as an explicit native policy**,
placement writes the selected source Y, and removal writes ffffffff. Original
fixtures begin with zero-filled receiver storage and compare this complete
controlled field state. Original allocator/uninitialized-memory equivalence is
not claimed; the field is not used to draw an unoccupied slot.

An occupied assignment must agree with its descriptor, rotated edges and stored
source row. Descriptor and grid capacities retain the constraints milestone's
bounds. The basic pruning API still returns only the candidate table; the new
`pruneRegionCandidateDetails` additionally returns the original final carry,
or no carry update if no descriptor was processed.

## Placement and removal

Placement loads the requested descriptor and rotated edges, writes its section
ID, descriptor index, rotation and source block coordinates into the anchor,
and increments its count. Multi-block placement saves the root's pre-placement
count, then runs `block-columns*block-rows - 1` passes. Each pass follows current
internal edges in north/east/south/west order, wrapping coordinates and skipping
occupied neighbors. An empty neighbor loads the matching descriptor at the
saved count, rotates it by the original requested rotation, writes it and
increments that descriptor's count. The final carry is retained.

This is the original placement operation after selection, not an admission
check. Public native placement validates extents, refuses an occupied anchor,
invalid rotation/count overflow or a missing connector, and commits changes
only after its owned temporary state succeeds. These refusals are native bounds
policy. The original unchecked write can overwrite an occupied anchor; that
path is outside the native interface's validated domain.

Removal requires a populated slot and positive descriptor count. It decrements
that count, clears the assignment/edges and writes ffffffff into the retained
source-row field. Carry and retained candidate tables remain unchanged. The
reference reproduces the original caller's count decrement plus unchanged
`0052d480` and compares all grid fields/counts afterwards.

## One-attempt solver

`solveRegionPlacement` accepts state by value, clears retained candidate tables,
and scans rows first, then columns. Prepopulated slots are skipped. For each
empty slot it constructs ordinary candidates first, falling back to special
candidates only if ordinary selection is empty. It uses the prior recovered
seeded chooser and placement operation. The receiver seed is unchanged.

When both candidate sets are empty, it retreats to a previous occupied slot,
increments the backtrack counter, removes that placement and prunes the tried
rotation from its retained table. A remaining candidate is reseeded/selected
and placed there; forward scanning resumes. Empty tables cause further retreat.
The selected original exits on more than 49 retreats at the outer loop boundary,
and can return earlier when retreat reaches the origin. Partial placements,
counts and tables remain observable on failure, and the native result owns
that partial state. The caller's input remains unchanged.

Specific descriptors have an unusual boundary check in this driver: after
retreat, a Specific slot at column greater than zero returns failure. At column
zero the original jumps to the preceding row's final column. Native processing
preserves that selected scan behavior but refuses movement before the grid or
retreat into an unassigned slot. The original unsafe out-of-grid dereference
paths are not executed by the corpus. This is not the original Specific
placement/fallback function `0052f3f0`.

For multi-block retreat, the driver clears the first block and loads its carry.
It follows occupied internal neighbors in four north/east/south/west passes,
decrementing/clearing each. It tracks the smallest row and column separately
and updates the descriptor used for pruning when either selected coordinate
minimum changes. This is the recovered scan policy, not a bounding-box or
whole-section erase invented by the native implementation.

It rebuilds ordinary candidates at the selected minimum anchor, removes the
tried descriptor without a fixed rotation, then traverses unrotated descriptor
connectors in three passes to prune the remaining source blocks. Connector
lookup here excludes the **current scratch carry descriptor**, which may have
been changed by pruning; it does not simply exclude the walk descriptor.
Pruning-carry reconstruction is necessary for this driver. After multi-block
removal, subsequent single-block retreats rebuild ordinary tables before
pruning rather than relying solely on stale retained tables.

The original result must be read from **AL**, the low byte of EAX. A confirmed
failure fixture returned EAX `ffffff00`; treating the whole register as a C++
boolean would incorrectly report success. The native result is a typed bool,
and the binary reference checks the original low byte.

## Evidence and bounded reproduction

```sh
python3 tools/test-terrain-solver.py
```

The wrapper guards original files before/after, pins the executable hash,
independently unpacks all 683 installed MAP headers, records hashes and runs a
frozen-source option through `--source-root`. It privately maps the unchanged
PE32 image. The selected RNG/choice paths use the previously documented private
FS exception head. No executable bytes are patched, no Wine game is launched,
and no Win32 imports are called on the selected fixture paths.

The reference checks placement across all four source shapes, grid dimensions
2..5, every anchor and four rotations, with preexisting descriptor count one.
It then removes each occupied slot individually. A separate pruning corpus
checks both table bytes and the detailed final carry.

Synthetic solver fixtures include ordinary/special admission, fixed non-origin
placements, occurrence exhaustion, asymmetric edge signatures, multi-block
catalogs, successful completion and partial failure. Every original/native
comparison checks the low-byte status, complete 50-descriptor packed region,
all 25 packed candidate tables and all 25 packed grid slots in the controlled
receiver, plus final carry and unchanged seed. Thus leftover tables and count
side effects are compared along with visible assignments.

Each installed header supplies a **single-source fixture catalog** with maximum
nine, on a 4x4 grid. These are solver fixtures using installed data, not original
realm recipes or full generated maps. They intentionally include configurations
that cannot fill the grid. Successful/failed layouts are compared against the
original attempt driver; scene pixels are not generated in this milestone.

Validation passed: 3,136 placement cases, 7,056 individual removals, 2,048
pruning-carry cases and 3,755 solver cases (3,072 synthetic plus 683 installed
header catalogs). Of these, 2,067 completed, 1,940 backtracked and 231 used
multi-block backtracking; no native refusal was reached. All ten reconstruction
CTests passed in each normal and ASan/UBSan build.

Native ownership/refusal tests and the complete deterministic corpus also run
under ASan/UBSan. Counts and provenance are recorded in
[the companion report](terrain-region-solver.json).

The 49-candidate pruning bound remains explicit. Additional native refusals
cover missing connectors, negative/overflowing counts, inconsistent assignments,
rollback outside the grid or into empty slots, and more than 10,000 driver
operations. This finite operation budget is native policy for unchecked
original-loop paths. No fixture in the validated solver corpus reached a native
refusal. Native bounds policy is separate from original error-handling behavior.

Confidence is high for the bounded placement and one-attempt driver domain.
Remaining work is original CFG/catalog initialization and connector threshold
policy, Specific placement/fallback and wildcard-location orchestration,
ten-attempt reset/seed progression, conversion of generated assignments to the
owned terrain assembly plan and generated scene comparisons. Live integration,
entities, lighting and water remain separate.
