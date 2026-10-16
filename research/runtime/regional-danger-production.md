# Regional danger production (NS75)

This contract covers production of the regional byte rows consumed by `523070`
and regional search `53e5e0` in No-CD build
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
It separates recovered producers from the native visibility-memory, frame and
checkpoint policies. It does not recover the original strategic AI, owner task
interpreter, complete World scheduling or the producers of resources and spells.

## Evidence and confidence

Confirmed findings below are grounded in pinned disassembly, read-only Ghidra
exports and isolated calls into the unmodified producer/helper bodies. Discovery
outputs are retained under `working/tests/regional-danger-ns75-export-v1` through
`v7`; v4 contains producer pseudocode, v5/v6 contain helper pseudocode, and v7
pins the floating constants. The reproducible exporter is
`tools/export-regional-danger.py`. Original input manifests are verified around
these experiments. Static discovery alone establishes neither equivalence nor
integration; final source-bound execution is recorded separately below.

The new behavior IDs are `AI.regional-danger-production`,
`NP.visible-threat-observations` and `NP.regional-danger-continuation`. Confidence
is restricted to their declared logical input and tested headless domains.
Original owner knowledge/task production, spell creation, general species and
whole World execution remain pending. Existing unknown binary ranges and
unresolved flows remain in the inventory.

## Original state and dispatch

`415c40` increments brain tick `+14`, then invokes `41a060` and `41d270` only
for brain kind 1, unless globals `6e1f4c` and `6e1f50` respectively suppress them.
Its remaining task/knowledge work is outside this implementation. World
`46afc0` calls this driver for **two distinct contexts**, `690600` and `644550`,
before ordinary creature maintenance. Native composition deliberately advances
one explicit context per admitted normal simulation frame. It is not proof of
the original dual-context cadence. `464ec0` is task-interpreter context, not a
recovered per-owner frame provider.

| Brain-relative field | Recovered role |
| --- | --- |
| `12073`, `12074`, `12078`, `1207c` | Danger reset, owner, next memory key, effect/base region cursor |
| `12080`, `12084` | Associated commander pointer, pending danger total |
| `12088` / header `1208c` / count `12094` | Temporary distinct-region set |
| `12098`, `12099`, `1209d`, `120a1`, `120a9` | Deduction reset, owner, live-pool cursor, effect region cursor, pending total |
| `11c02 + 48*owner` | Owner memory tree; header `+4`, count `+12` |

Memory node key is at `+12`, observation tick `+16`, and creature snapshot `+24`.
These are private fixture projections, not portable pointers. Live pool stride
is `e4b`; at most 125 slots are scanned. Region stride is 660 bytes:

| Region-relative field | Role |
| --- | --- |
| `532 + 7*owner` | Seven published danger components |
| `588 + 7*owner` | Seven published friendly deductions |
| `644` | Published owner-independent base danger |
| `645..651`, `652..658`, `659` | Pending danger, deduction and base rows |

`542b00/542b40`, `542ba0/542be0` and `542c40/542c70` independently clear/publish
those rows. Publication copies complete owner rows and clears pending scratch;
partially scanned effects do not become visible early. `41a060` handles at most
four memory records per call and 32 effect regions per publication phase.
`41d270` handles four **qualified** live actors, scanning skipped slots without
consuming that actor allowance, then up to 32 effect regions. Owner scanning and
base-pass wrap retain their original cursor ordering, including unfinished work
at a slice boundary.

## Contributions and arithmetic

Danger memories with zero health or region -1 contribute nothing. The memory
active flag alone does not suppress a contribution. Strength is the signed
maximum of `maximumResource*64` and either own-owner current resource plus
available inventory kind 19, or the querying owner's stored resource estimate;
then threat `actor+91e` is added. The amount is
`strength*20 / (10000 + (tick-observedTick)*aging)`, with original DWORD wrapping
and signed division. `520620` modifies CFG `+589` by doubled status `778` and
halved status `77c`. A declared map observation point within five units removes
aging. Native code rejects undefined division rather than invoking host UB.

`411e50` recognizes classes 0/24/25/26. Broad contributions also include ranged
actors unless type 18 is flying. They use the ground/free 1×1 descriptor and two
hops of distinct connected regions for components 0/1. Status `114` contributes
configured fire power to component 2 on that same two-hop neighborhood. Ordinary
narrow actors with animation kind other than 2 use their actual movement
clearance for one hop; component 1 additionally requires region parameter 0 or
actor flag `104`. Addition clamps each byte to 0..255 after wrapped integer
addition. Deductions use the live active/health/region filters, owner/shared/allied
qualification, `(resource & ~255) + threat`, and their separately recovered
wide/narrow rules. Inventory kind 19 contributes to hostile current-resource
accounting, not the live deduction resource mask.

`542810` greedily descends strict wrapped XY plus raw Z squared distance.
Connection order resolves ties; a local minimum fails. Success returns the
middle path entry, index `(length-1)/2`. The producer comparisons exercise its
connectivity result; the otherwise unused middle-entry output is statically
derived, with direct output comparison still pending. `40a6b0` instead uses fine-scaled XYZ
squares and an unsigned shift by ten; its square root drives range weighting.
`529e40` uses the same center-connectivity predicate and
`trunc(wrappedUnsignedNumerator / (sqrt(distanceSquared)*2304.0 + 10000.0))`,
halved for `block7c`, then multiplied by ten. Constants `5c72a0=10000.0` and
`5c72a8=2304.0` are read from the pinned PE. Support is written to `actor+92a`;
commander danger/deduction totals are `+922/+926`.

Ranged owner-total reach comes from the **global type table** CFG `+34`, rather
than a potentially different snapshot CFG pointer. Logical `attackRange` must
represent that global field; helper fixtures give distinct type IDs to distinct
configured ranges. Animation-kind-2 and fire reach use `6bef73` and `6bcada`.
Kind `41` effect radius aliases the latter global too. These aliases were exposed
by expanded paired tests and corrected in fixture input binding; failed development
reports remain retained. A combat journey also exposed CFG `589` as both route
aging and valuation input. Danger-enabled native valuation binds that same type
field (and `5ad`); existing danger-disabled explicit environments remain intact.

## Declared effect inputs

Global list `6898e4` points to nodes with object pointer `+8`; object kind is
`+4c`, owner `+212`, and XYZ `+8/+12/+16`. Radius comes from the effect configuration
at `6b13c9 + 721*kind`, including the kind-41 alias described above.

| Kind | Hostile row | Friendly deduction row |
| --- | --- | --- |
| `41` | Component 2 +10 regardless of alliance | None |
| `45` | None | Allied component 3 +30 |
| `46` | Nonallied component 4 +20 | Allied component 4 +20 |
| `5b` | Nonallied component 5 +5 | Allied component 5 +25 |
| `5c` | Nonallied component 6 +20 | None |

`41f948` handler and `41f95c` translation tables are registered as **incomplete**;
other/default translations remain explicitly unmapped. This is danger-filter
behavior, not implementation of the underlying spells. `420e20` scans three
other lists: `689908` contributes fixed 10 with radius `6be5af`; `6898fc` uses
`6b7387/10` and the selected animation range; `6898f0` uses `6c0b9d/10` and its
selected animation range. Native inputs normalize these to position/radius/amount
records. The original fixtures preserve the actual three-list/global aliases;
they do not establish arbitrary per-object radii within one original list.

## Native integration and boundaries

`native_regional_danger.cpp` refreshes live actor fields, current `combatValue[5]`,
inventory units, aging and relations. It refreshes a hostile memory only when the
actor is visible to that owner, retains the old snapshot/time when hidden, and
removes remembered hostility when an actor is visibly friendly. This is an
explicit native observation policy. It does not implement the original memory
insertion/expiry/task system or recover how visibility is produced. Current and
maximum resource and estimated resources are explicit retained inputs; the normal
bounded encounter has zero resource production. Commander, shared-vision setup,
area effects, observation points and base hazards are also explicit inputs.

Configuration stages a private continuation and validates before publishing.
Ordinary native frames advance danger before creature maintenance/scheduling;
regional search consumes the resulting published rows through the existing danger
consumer. `native-encounter run-danger` enables regional graph loading and this
policy. `resume` and `edit-terrain` reconstruct services from embedded inputs.
No Wine, Qt, installed asset path or external original process is needed for a
native saved continuation. This does not remove Wine from the original-backed
full game.

The paired journeys independently construct original/native graphs before the
first tick. Original observations are read from actual original pool fields,
with the same declared visibility policy; no native frame state is copied back
into original execution. Four scenarios cover travel, a hidden moving threat and
visible death, diplomacy with area/base inputs, and actual melee damage/death.
Declared effect lists are cleared before unrelated original World effect work,
so these tests establish danger filtering, not full spell lifetime/World effects.
The harness compares actor/ANI/motion/route/shared-budget/R250/grid projections,
all six valuation fields, published and scratch danger rows, producer cursors,
totals, actor support and remembered observations on every tick. Original producer,
region-set, classifier, associated-actor and search helpers run unmodified; private
allocation/presentation boundary adapters and executable text guards remain.

## Reproduction

Run through the main checkout's storage guard, with absolute paths because the
guard changes its child's working directory to the main checkout:

```sh
python3 /path/to/main/tools/working-retention.py run --reserve-gib 2 -- \
  python3 /path/to/worktree/tools/test-regional-danger.py \
  --output /path/to/worktree/working/tests/regional-danger-new \
  --install /path/to/main/working/game-nocd \
  --build-root /path/to/worktree/working/tests/paired-encounter-bound-ns69-20261015-v1 \
  --claims /path/to/worktree/working/tests/prospective-danger-claims.json
```

Prepare the prospective declaration first using `draft-coverage-claims.py` for
the three IDs and scenario `regional-danger-ns75-20261015`. Build dependencies,
installed input hashes, original identity, before/after manifests, source stability,
raw traces and final snapshots are retained by the runner. Original32 execution
and leak sanitizers need isolated-process execution permissions in the sandbox.
Fresh-process saves run with `PATH=/nonexistent`; original comparison and native
wire/policy integration remain independent validation stages.


Finite fixtures are not exhaustive arithmetic or gameplay proof. Nonzero
inventory-resource perturbations, future-time/division-failure cases, exhaustive
saturation/overflow boundaries, every effect/alliance combination and the direct
middle-entry output remain comparison gaps. The public blocked-slice no-op
parameter does not recover the full original World guard/pause policy. These
limits do not acquire validation from the existing consumer/search regressions.


The first source-bound run completed all eight paired/replay journeys and 64
fresh-process boundaries, then failed its aggregate witness check: the driver
incorrectly required a death in each combat seed. Seed 17 has 36 hits and a death;
seed 91 has 19 hits and no death after 2048 ticks in both the original and native
runs. The corrected driver requires combat in both and a death across the set.
No engine or fixture behavior was changed to force a fatal outcome. The failed
v1 report and its exact superseded driver remain under the retained run; they
are not promoted to a successful result. The final v2 run has its own prospective
source declaration and executes three isolated journey workers after builds and
discovery complete. Only v2 supplies final milestone execution evidence.


## Recorded execution

The [immutable NS75 report](regional-danger-bound-ns75-20261015.json) passes
5,203,818 helper checks and 1,024 producer slice pairs across eight
synthetic domains, with nonzero danger/deduction/base rows and unfinished scans.
A hazard produced through the base pass changes the selected diamond route;
original and native searches agree with danger weighting disabled and enabled.
The state suite passes 399 checks, 96 restores and
10 admission/configuration/older-write refusals.

Eight original 32-bit/native 32-bit paired journeys pass 16,384 ticks across
four scenarios and two seeds. Travel reaches its goal; hidden actors retain last
visible snapshots; declared diplomacy/effect inputs and actual combat/death
projections match. Goal-frame counts measure occupancy, not distinct arrivals.
Native 64-bit and ASan/UBSan/LSan 64-bit full replays match traces and final snapshots
with 2048 in-process restores. Fresh-process chains match final snapshots at
64 boundaries, totaling 2,112 journey restores.
Fresh-process saves use only embedded inputs and PATH=/nonexistent. The CLI
run-danger, terrain edit and split resume agree; a refused edit publishes no file.

Eleven static graph fixtures, 258,440 navigation checks, one bounded incremental
maintenance fixture, five 1024-tick generated static journeys, both five-test
command/provider suites and both installed encounter regressions pass. All
265 prospectively declared sources and
257 compiler/configure inputs, installed inputs and 2927
immutable originals remain unchanged; original executable text is guarded.
Only the three declared NS75 contracts receive fresh scoped validation. Broader
shared-source historical evidence retains its original hashes and limitations.
The incoming `45f64f41..79d3fd3f` history review matches 30 file and 38 behavior
transitions to exact receipts, without asserting a past gate pass or execution.
