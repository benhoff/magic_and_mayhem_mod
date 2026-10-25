# Twenty-second bounded rendering batch (2026-10-25)

The existing owned fixed-region, catalog and seeded single-block selection
implementations now have current-source boundary controls and original comparison
proof. `RE.terrain_region`, `RE.terrain_catalog` and `RE.terrain_selection` move
implementation from partial to scoped. Understanding remains partial, comparison
recorded, and integration/replacement none. The model algorithms needed no
behavior change. The catalog test now uses checks that run in Release instead
of assertions compiled out by NDEBUG; all three units gain boundary controls.

- `RE.terrain_region`: 3136 original placement cases/46656 blocks plus two installed authored regions/32 blocks match. Independent native whole-cell assembly matches 18 fixtures/288512 cells in Release and ASan/UBSan, including both installed regions.

- `RE.terrain_catalog`: 2087 original catalogs/26888 descriptors match, including 39 available installed catalogs and 291 threshold quirks. All 40 native recipe decodes match independent input parsing. Release-active ownership/capacity/refusal controls pass; the missing Medieval Test catalog remains unavailable.

- `RE.terrain_selection`: 256 original seeds/262144 full-state draws, 207840 admissions, 4096 candidate tables, 10240 location tables and 78584 original choices match. 1232 installed single-block descriptor cases match. Native endpoint-seed/capacity/mask/refusal controls pass.

## New native controls and exact proof

The catalog unit checks owned headers/requests, repeated section IDs, source
provenance, Specific versus Random maxima, threshold assignments, fifty accepted
descriptors, expanded multi-block overcapacity, late occurrence refusals and
source/counter bounds. Selection checks full compact capacities 50/25, endpoint
seeds, every nonempty rotation mask, returned entry indices, late invalid entries,
reseeding after refusal and signed header endpoints. Fixed assembly checks late
bad block coordinates/duplicates/rotations, caller source/plan preservation,
owned output after source destruction and zero-layer admission.

The new native assembly CLI compares every cell field and projection counter to
the independent Python transcription in `tools/test-terrain-region.py`. Two
installed Celtic regions 6/9 supply 288000 cells; sixteen synthetic rotated,
toroidally anchored cases supply 512 more. The synthetic cases exercise object
and reference stripping, four definition/ownership rotations and both axes of
anchor wrapping. Existing mixed-height units check untouched upper cells. TTD
and MAP data remain read-only; no object/entity state is created. Ordinary native
projection is a policy boundary distinct from original entity-linked copying.

Original reference controls privately map the pinned PE32 image and call safe
selected placement, descriptor, RNG, admission and choice routines. The catalog
fixture copies the unchanged 131-byte threshold window 0x52e3ca..0x52e44d into a
private executable buffer and appends a return adapter; mapped original code is
not patched. Selected seeded choice helpers install/restore a private FS exception
head, execute no Win32 calls and do not claim original exceptional recovery.
The central register now names the previously implicit candidate-clear/scan,
location/rotation-choice helpers and the exact catalog threshold/descriptor
scope. A function link covers its stated bounded operation only.

[Execution2](rendering-twentysecond-three-results-20261025/execution-2.json)
holds prospective final-contract claims, all declared sources and compiler header
closure, input identities, command/log hashes, six Release/ASan/UBSan unit passes,
three original comparisons, repeated complete native sanitizer corpora and both
whole-cell assembly matrices. The first passing [execution](rendering-twentysecond-three-results-20261025/execution-1.json)
retains its [earlier contract inputs](rendering-twentysecond-three-results-20261025/execution-1-contract-inputs.json).
Execution2 renews proof after precise address-link accounting; no code changed
between these executions, and Execution1 is not rebound to the new contract.
Both experiments verify the original manifest before and after. The executable
SHA-256 is `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

The input corpus independently decodes 40 shipped recipes, 683 MAP files and 616
single-block headers. Thirty-nine complete installed catalogs are accepted.
Medieval REGION0 still references eleven absent `Realms/Medieval/Test` MAPs;
explicit unavailable header markers lead to one native refusal. No Village files
are substituted, and original missing-file/dialog continuation is unexecuted.
Native rotation-mask counts are 831360; only the 78584 selected original choice
calls receive original comparison credit. Historical image, producer and
installed-wide reports retain their exact source fingerprints.

Reproduce with `tools/test-rendering-twentysecond-three.py`, a new absolute
`--output` below this checkout's `working/tests`, a new `--report` below
`research/runtime`, and an existing read-only installed `--asset-root`. Launch
through the main checkout's retention guard. The runner chooses a private TMPDIR,
builds units with explicit NDEBUG for Release, compiles ASan/UBSan and PE32 helpers,
and stores success or failure through the SQLite report writer. No Qt/GPU/Wine
build, fresh installation or complete source snapshot is needed. Durable reports,
contract inputs and verified SQLite backups are retained; compiled binaries,
decoded CFG/MAP interchange, bundles/goldens and logs are temporary execution
outputs pinned under `working/tests`.

The user [approved an 89 GiB reserve for this batch](rendering-twentysecond-three-results-20261025/storage-approval.json)
after the normal 90 GiB preflight refused. The storage inventory found zero
eligible cleanup candidates. A separate batch policy implements that approval;
the tracked normal policy remains 90 GiB. The worktree, builds and durable SQLite
backups are on the Videos drive. Local completion uses the normal staged gate
and verified backups; no publication or remote source push is requested.

[Planning UI controls](rendering-twentysecond-three-results-20261025/ui-controls.json)
and the [coverage audit](rendering-twentysecond-three-results-20261025/coverage-audit.json)
retain the separate accounting checks. The
[incoming history review](rendering-twentysecond-three-results-20261025/incoming-history.json)
finds exact receipts for all 18 latest-commit file transitions. These accounting
results do not widen the engine validation scope.

## Remaining boundaries

`RE.terrain_region`: Original CFG/file/dialog/allocation recovery, complete world/object generation and scene/GPU presentation, caller/resource lifetime and live integration/replacement remain separate. Whole-cell assembly is checked against independent Python, not a complete original world producer.

`RE.terrain_catalog`: Original whole CFG loading, missing/invalid-file dialogs and continuation, allocator/descriptor lifetime, complete world/scene generation and live integration/replacement remain separate. Medieval REGION0 lacks eleven Test MAP inputs; no substitute assets or original missing-file recovery are claimed.

`RE.terrain_selection`: Multi-block connector traversal/pruning/backtracking and generation retries, original production descriptor/grid ownership, whole-world/scene construction and live integration/replacement remain separate. Native malformed-input refusals do not reconstruct unsafe original failures.
