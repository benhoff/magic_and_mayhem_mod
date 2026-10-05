# Effect creature ordering, boundaries and mixed exits

Build: No-CD SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
TL17 extends the TL16 authored-world comparison without changing production code.
Confidence: high within these bounded fixtures; no installed capture or live replacement.

The original `004883f0` runs whole and unmodified, using real trajectory, terrain,
occupancy, cell unlink/insert and cleanup helpers. The native path remains
`transitionEffectCreatureWorld`. Separate test/runner and evidence files preserve
TL16's historical inputs and source fingerprints.

The ordered matrix targets each of the 27 candidate positions in six geometries:
ordinary middle, low XYZ edges, high XYZ edges, a 1x1x1 world, a 2x2x3 world and
an asymmetric 1x2x2 world. Each target varies ordinary occupancy, status 27,
creator exclusion, an out-of-catalog ordinal and competing occupied cells.
Five supported effect types and both membership settings produce 8,100 calls.
Candidate cells wrap in X/Y, reject invalid Z, and may alias. Competing fixtures
assign different creature ordinals and filter one status before selection; an
independent authored compass/plane ordering predicts the first eligible hit.
Full footprints and authored fine coordinates deliberately isolate candidate
selection from installed creature-placement invariants.

Twenty two-step type-3 calls guarantee a creature hit on step one, then exercise
terrain return 1, blocked-cell return 1, raw height return 2, a no-hit continuation
retaining return 0 and the previous id, or a different hit overwriting the id.
They cover both membership settings. Full record comparisons include preserved
candidate state, raw versus published coordinates, terrain cache publication,
trajectory counters and effect links. A height exit is not resumed from its
partially advanced raw parameters; that admission remains separate.

Twelve authored worlds make two calls each. Between calls the fixture changes
status, the current cell occupant, fine X, footprint rows, creature id, or removes
the occupant. Caller mutations are mirrored explicitly into the private original
allocations before the next call. Each original call must leave its current
creature allocation unchanged; this does not assert cross-call immutability or
concurrent mutation support. Native and original outcomes distinguish a new
snapshot from prior candidate/parameter state.

`effect-creature-boundaries.json` records original32/native64/ASan-UBSan stream
comparisons, whole guarded effect/cell buffers, immutable per-call creature and
terrain inputs, executable/source fingerprints, strict existing native units and
CMake/CTest. Original manifests verify before and after; no patches, callbacks,
stubs or original instruction breakpoints are used in the recorded execution.

Remaining scope: kind34 creator exception, other types and metadata, type35
terrain mutation, original trajectory setup, installed descriptor/creature/cell
production, lifecycle/removal/recycling, live scheduling and replacement. This
matrix is bounded execution evidence, not exhaustive combined-branch coverage.
