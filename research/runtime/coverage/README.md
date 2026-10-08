# Binary-to-behavior accounting

This package adds a discovery inventory and an auditable code/behavior index to
the [coverage ledger](../coverage-ledger.md). It records what has not been mapped
as well as what has. A function with one behavior link is not a fully recovered
function; checklist counts never establish whole-engine completion.

## Run the audit

From the repository root:

```sh
python3 tools/audit-re-coverage.py
python3 -B tests/test-re-coverage.py
python3 tools/audit-re-coverage.py --require-fresh
```

## Required change accounting

The change gate is separate from the discovery audit:

```sh
python3 -B tests/test-coverage-gate.py
python3 tools/check-re-coverage.py
```

The default command compares against the reviewed `gate-baseline.json` snapshot.
Once the gate has been committed, use `--base HEAD` for local changes or an
explicit reviewed commit for a branch comparison. CI compares the actual PR
base, push predecessor or merge-group base. It reads that tree without switching
branches or executing historical code. Updating a census or baseline in the same
change cannot reset the Git comparison. Initial adoption uses `--bootstrap` only
when the base predates the gate; using it after adoption fails.

The baseline retains unresolved functions, unlinked files and historical stale
evidence. The gate fails on unreviewed additions/edits/deletions/link changes,
unaccounted new gaps, invalid discovery/audit claims, rewritten history and
unsupported status promotions. Behavior fingerprints include linked source,
test and document bytes, evidence definitions, scenarios and original build
bindings. A shared-file edit affects all linked behaviors. Research/design
Markdown and JSON and workflow controls are watched alongside the source census.
`gate-baseline.json` and the review journal are excluded from their own source
fingerprints; Git comparisons still use the actual old tree and preserve review
history. Historical discovery inventories must remain available unchanged.

For each change:

1. Update the affected register entries and research; retain partial/unknown
   behavior explicitly. Renames must also update file links and, where needed,
   rebind the same historical provenance keys to the new path.
2. Reconcile the source census and focused registers. Store referenced snapshots
   under a durable repository path; ignored `working/` snapshots cannot be read
   by a fresh CI checkout. This step acknowledges index review, not validation.
3. Generate an exact draft, choosing the same base used for the final check:

   ```sh
   python3 tools/check-re-coverage.py --base HEAD \
     --draft-review working/tests/coverage-review-next.json \
     --json working/tests/coverage-gate-next.json
   ```

   During initial uncommitted adoption, omit `--base HEAD` to use the reviewed
   snapshot. Output paths must be new. The draft command produces incomplete
   accounting; its successful exit is not a passing gate.
4. Supply concrete `reason` and `validation.reason` values, then append the draft
   receipt to `change-reviews.json`'s `reviews` array. Preserve its exact before/
   after hashes, every affected behavior and each new `pending_gaps` key. Run
   appropriate tests. A further edit requires regenerated hashes and review.
5. Run the gate again without `--draft-review`, with the same base. Existing
   unrelated debt remains visible without blocking this change.

Receipts have a stable ID, reason, `files` (`path`, before/after SHA-256 or null
for addition/deletion), `behaviors` (ID and before/after contract SHA-256), exact
pending-gap identities, and an explicit validation decision:

- `pending`: this source/contract change is accounted for, with a stated remaining
  validation boundary. Historical results stay historical.
- `recorded`: name fresh non-static execution evidence IDs in `validation.evidence`.
  The gate verifies links, appropriate comparison/integration/replacement kinds,
  changed-source fingerprints and freshness. A census/static review cannot supply
  this proof. Scoped implementation or validation-stage promotion requires it.
- `not_required`: support/build/research/control work with no affected recovered
  or previously validated contract. Engine/protocol changes require `pending` or
  `recorded`. This decision does not bypass the repository's normal tests.

Old receipts remain append-only. They do not excuse future edits, newly added
gap identities, or evidence overwrites. Human review still determines whether a
scope, test or pending reason adequately describes the work. The gate establishes
accounting, not scientific completeness or the identity of every indirect caller.

The [CI workflow](../../../.github/workflows/re-coverage.yml) runs both guard suites
and the change gate without game media, Ghidra, Qt, Wine or original execution.
Configure the repository's merge rules to require **Coverage accounting**; a
workflow file alone does not configure branch protection. `tests/test-tooling.sh`
also runs the local gate. Install the local pre-commit check manually with:

```bash
python3 tools/install-coverage-hook.py
```

The installer sets this repository's `core.hooksPath` to `.githooks`, refuses to
replace custom hook paths or active default hooks, and can be run again safely.
Disable it with `python3 tools/install-coverage-hook.py --uninstall`.
It does not change global Git settings or remote merge rules.

The hook runs `tools/check-re-coverage.py --staged` against the actual `HEAD`
tree. Initial adoption automatically uses `--bootstrap`; an unborn branch uses
the staged frozen baseline. Only staged files, census, register and reviews are
checked. Unstaged repairs cannot approve an incomplete commit. Stage these
related files together after recording the exact review, then commit normally.
To inspect the same staged gate manually after adoption, run
`python3 tools/check-re-coverage.py --staged --base HEAD`.
Git's `--no-verify` bypasses local hooks; required CI remains the remote gate.

To start a reviewed baseline, use `--freeze-baseline` with a new path. This
explicitly accepts existing accounting debt without changing any original
evidence hashes. It refuses overwrite and requires a valid, reconciled audit.
Ordinary changes should use receipts rather than repeatedly freezing baselines.

## Discovery audit details

The normal audit fails on invalid references, unsupported status claims,
changed evidence records, incorrect original-build hashes, missing files, or
invalid source provenance. Unknown functions, unimplemented behaviors and stale
source fingerprints are findings. `--require-fresh` additionally fails when
current files differ from recorded evidence sources. This is expected for the
initial historical movement evidence; it does not mean those comparisons failed
when recorded. The audit never reruns a comparison or silently refreshes evidence.
`tests/test-tooling.sh` includes the normal audit and its synthetic regression
tests. No original artifact is consumed by these offline checks.

Within one audit, validated paths and file fingerprints are reused. Every new
audit creates fresh caches, including calls made for a staged tree or Git base;
there is no persistent cache or previous-pass exemption. Function body lookup
uses an index of half-open ranges, preserving inventory order when bodies
overlap and retaining all owners of nested or fragmented ranges. The guard
suite checks boundaries, overlap ordering and changes between audit calls.

Write new reports without overwriting earlier evidence:

```sh
python3 tools/audit-re-coverage.py \
  --json working/tests/re-coverage-current.json \
  --markdown working/tests/re-coverage-current.md
```

The [generated summary](summary.md) is a snapshot. Use the command above
for current evidence freshness. Full findings in the JSON include addresses,
flows, candidate callbacks, imports and behavior/scenario links. The register
now includes pathfinding, motion, audio, asset readers, rendering, animation,
Qt menus/services, runtime adapters, protocols, native world/tooling and known
gameplay gaps. All 104 spell configuration IDs remain explicit; a switch-case
link does not establish spell implementation or complete dispatch semantics.
Preferences entries were consolidated from their focused register, preserving
their original evidence and independent live-observation scope.

The [source index](source-index-20261005-final.json) snapshots the current component
roots, including build files, protocol schemas, assembly, headers, implementations,
tools and tests. Its links are exact registered implementation/test paths, not
automatic semantic matches. The audit reports new, changed, removed and unlinked
sources, and changes to consolidated focused registers. Unlinked support tools
and tests remain mapping debt rather than being silently counted as coverage.

After reviewing a changed component and updating its links, create a new census:

```sh
python3 tools/index-coverage-sources.py \
  --output working/tests/source-index-new.json
```

Review the diff, then set `register.json`'s `code_index.path` and `code_index.sha256`
to that snapshot. Prefer a durable repository path when preserving a reviewed
baseline. The exporter refuses overwrite. A census refresh records a source
indexing review; it does not rerun comparisons, promote status or clear stale
original evidence. `--require-fresh` concerns evidence fingerprints; census drift
is independently reported by the normal audit.

Calls into executable ranges outside discovered function bodies are also
reported. Unregistered computed jumps remain dispatch candidates even when
Ghidra inferred some targets. These findings help direct further investigation;
they are not an automatic recovery of indirect behavior.

## Inventory provenance and reproduction

`binary-nocd.json` pins No-CD executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
It contains all non-external functions discovered by a fresh Ghidra 12.1.4
analysis, half-open body ranges with byte hashes, imports, data-reference
callback candidates, calls, computed flows, cross-function jumps, and the
complement of function bodies in file-backed executable sections. Raw discovery
also retains ordinary intra-function jumps in the ignored per-run output.
Virtual zero-fill is outside the file-backed accounting denominator.

```sh
python3 tools/inventory-binary.py --output working/tests/binary-inventory-new.json
```

The exporter checks the whole executable hash, verifies the immutable original
manifest before/after (also on failure), runs read-only binary analysis in a
fresh working project, checks the input stayed unchanged, and records command,
tool location, exporter-source hashes and raw-discovery hash. It refuses to
overwrite a prior inventory. No game runs and no executable is patched.
Ghidra is discovered through the existing decompilation tooling; `--ghidra`
and `--executable` can select explicit paths, but another build is rejected.
Exporting does not automatically change the register's pinned inventory.

Discovery remains provisional:

- Unassigned bytes can be padding, embedded data or undiscovered code.
- Inferred computed targets do not establish complete switch/dispatch coverage.
- Data references to code are callback/table candidates, not verified callbacks.
- Register-loaded imports, dynamic loading, COM, external DLLs and OS behavior
  need separate contracts and inventories. The current inventory includes the
  EXE's import boundary, not the bodies of all supplied/system DLLs.
- No static inventory proves that all original behavior has been found.

The first inventory discovers **6,675 functions and 190 imports**. Importantly,
it does not assign the tested motion action at `0x005104b0` to a function body.
The register explicitly supplies its documented `0x005104b0..0x00510e3a`
recovered range, anchored to original entry bytes in the NS05 comparison record.
This does not change the discovery inventory or remove its unassigned bytes.
The audit reports each behavior anchor relying on this exception.

## Register contract (schema 1)

`register.json` is the editable owner of behavior IDs and links. Addresses are
integer preferred VAs scoped to a build ID; they are not stable runtime pointers.
Ranges are half-open. Paths are repository-relative files; escaping or missing
paths are rejected. One original function can link to many behaviors, and one
behavior can link to several functions/ranges.

| Collection | Required content |
| --- | --- |
| `builds` | ID, original `source_sha256`, inventory path and `inventory_sha256` |
| `behaviors` | ID, subsystem, title, scope, confidence, kind, original references, documents, implementation paths, test paths, evidence IDs and all five status dimensions |
| `evidence` | ID, kind, scope, immutable JSON record path/hash, JSON pointer to source fingerprints, and explicit success assertions |
| `scenarios` | ID, bounded scope, validation level, behavior IDs, evidence IDs and test paths |
| `dispatch_tables` | ID, original address, document, scope, explicit completeness flag and value-to-behavior entries |
| `recovered_ranges` | ID, build, start/end, document, original evidence ID and JSON pointer to pinned entry-byte anchors |
| `classifications` | Build/function entry, category, reason and documents supporting the classification |
| `api_bindings` | Build/import IAT VA, scope and behavior IDs |
| `code_index` (optional) | Source-census snapshot path and SHA-256; explicit links must match the register |
| `register_imports` (optional) | Focused-register path, reviewed SHA-256 and consolidation scope; changes require reconciliation |

`original` references require `build`, `address`, and `scope`. The default
relation is code in a discovered function. `relation: data` admits a table in
a file-backed section without counting it as function coverage. A code reference
outside discovered functions must explicitly link `recovered_range`; the range
needs original comparison evidence with a matching build and recorded entry
bytes. The extent remains a documented recovered boundary, not a claim of
full function understanding or equivalence.

Evidence `sources_pointer` resolves to either a path-to-SHA-256 map or a list of
`{path, sha256}` records. `assertions` is a nonempty list of JSON-pointer/expected
value pairs. Original comparison evidence additionally requires `build` and
`original_hash_pointer`. Live equivalence needs `equivalence_pointer` resolving
to true; live replacement also needs `bypass_pointer` resolving to true. Pinning
a report protects its identity, not the scientific validity of its scope:
reviewed independent comparisons and honest limitations are still required.

Optional `source_bindings` maps recorded source keys to current repository paths.
Use it to select source provenance from reports that also fingerprint installed
inputs, or to bind frozen experiment source paths back to their current code.
The audit compares the original recorded hashes against those current paths;
bindings never replace the hashes. Missing keys, duplicate destinations and
escaping paths are rejected. Original input/build identity remains pinned by the
report and original-hash assertion. These bindings do not rerun input checks.

`recorded_inputs` optionally lists source-map keys that fingerprint binary/data
inputs rather than current code. Their original hashes remain pinned in the
immutable report and provenance key set; the offline audit does not open those
artifacts. Original comparison still requires the matching build-hash assertion.
Code/script/header/schema keys cannot be reclassified as inputs. This separates
input identity from current source freshness and makes checks portable to CI.

Partial implementation may be registered without tests; the audit exposes this
as `behaviors_without_tests`. Scoped implementation still requires test links.
The `REVIEW.*` records fingerprint this indexing review only. They are static
evidence and cannot authorize original comparison, integration or replacement.

| Independent dimension | Values |
| --- | --- |
| Understanding | `unknown`, `partial`, `scoped` |
| Implementation | `none`, `partial`, `scoped` |
| Comparison | `none`, `recorded` |
| Integration | `none`, `headless`, `preview`, `live_observation`, `live_equivalence` |
| Replacement | `none`, `scoped_live` |

`kind: native_policy` identifies intentional native semantics. It cannot claim
original comparison equivalence. A recorded original comparison must link
matching-build evidence that fingerprints linked implementation files.
Headless/preview integration requires native integration evidence and a scenario
at that level. Live status requires matching live evidence and scenarios.
Replacement additionally requires scoped implementation, recorded comparison,
live equivalence and evidence of original work being bypassed.

Behavior confidence is `unknown`, `provisional` or `high_within_scope`; it
describes the documented scope/evidence, not untested branches or the freshness
of current code. Source freshness is reported independently.

Classification categories are `runtime_support`, `legacy_only`, and
`unused_candidate`. These require a reason and supporting documents; a category
is not permission to delete functionality. Unused candidates remain hypotheses.
The register declares no exclusions or imported API mappings.

## Adding coverage

Incremental upkeep is required by
[AGENTS.md](../../../AGENTS.md#reverse-engineering-coverage-upkeep) and described
in the [architecture](../../../docs/architecture.md#coverage-ownership-and-incremental-work).
The author of each relevant change updates the affected records. Inventory
generation and auditing run explicitly; they do not automatically write semantic
behavior mappings or register new findings.

1. Assign a stable behavior ID and link original addresses, build, scope and
   research. Keep unknown/default/failure paths and incomplete tables visible.
2. Link native code and meaningful tests. Split recovered behavior from deliberate
   native policy, such as checkpoint format or new-order reset semantics.
3. Register immutable evidence with explicit outcome/build assertions and source
   fingerprints. Keep older evidence; a new result gets a new evidence ID.
4. Link the behaviors actually exercised by a bounded scenario. A headless
   continuation test cannot establish visible gameplay, live scheduling or pause.
5. Run the audit, inspect remaining gaps and freshness, and update the coverage
   ledger when a milestone changes. Review the diff when refreshing inventory
   or summary snapshots; never drop unknowns merely to improve a coverage number.

The initial movement checklist covers selected request/search/legality,
planar setup, fine motion, route consumption and event-0/event-2 composition.
The NS06 evidence additionally links ordinary terrain height and bounded
category-four/pure-vertical setup to a second headless continuation scenario.
This reconciles the terrain-aware movement milestone added during the accounting
work without rerunning or extending its original comparisons. The older NS04/NS05
records remain registered and their source staleness stays visible.
It separately lists other events, reverse/special profiles, original
orders, following, dynamic occupancy, shared scheduling, environment callbacks,
pause/cadence and visible native integration. Native checkpoints and queued-order
semantics have their own policy entries. This is a starter checklist, not an
exhaustive inventory of every branch in those routines.
