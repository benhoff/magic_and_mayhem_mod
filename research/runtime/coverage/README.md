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

The SQLite workflow below automates receipt preparation, recording and export.
The manual workflow remains supported for historical trees and initial adoption.

The change gate is separate from the discovery audit:

```sh
python3 -B tests/test-coverage-gate.py
python3 tools/check-re-coverage.py
```

Use `--timings` to print elapsed times for the current snapshot, Git-base
snapshot, merge-history review and receipt check. With `--json`, the same
durations appear in `timings_seconds`; timing does not change approval rules.
For example:

```sh
python3 tools/check-re-coverage.py --base HEAD --timings \
  --json working/tests/coverage-timed-next.json
```

The gate reuses file fingerprints and audited evidence provenance only within
one snapshot. A later call, staged tree or Git base is read independently. Source
links are inverted once while retaining their original order and membership.
Git exports stream through bounded buffers into the temporary tree; historical
code is never executed. No persistent cached pass can approve a changed tree.

The default command compares against the reviewed `gate-baseline.json` snapshot.
Once the gate has been committed, use `--base HEAD` for local changes or an
explicit reviewed commit for a branch comparison. The full CI audit is manually
requested and uses the event base, or HEAD's parent for a manual run. It reads that tree without switching
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

An older recorded receipt naming no behavior can become inactive when its only
matching files are unchanged registered execution reports and a later exact
pending receipt re-accounts those same file hash pairs. The older receipt then
grants no file, behavior, gap or validation credit. Its evidence remains immutable
and stale when its sources changed. Code files, current behavior claims and status
promotions still require their normal current receipts and execution support;
an earlier pending review cannot retire a later recorded claim.

For actual Git merges, the gate reads incoming parent registers and receipts
without executing historical code. Identical behavior contracts, original build
hashes, evidence definitions/links and status dimensions can retain their parent
milestones as historical inheritance. Changed scopes or higher stages still need
fresh recorded validation. Unmerged branches cannot supply inheritance. Imported
receipts receive no final source, gap or current-validation credit; the combined
tree needs its own exact reviews against the actual PR/push/staged base. Current
validation remains independently pending when merged sources differ from earlier
execution fingerprints. The report names the inspected commits and inherited
statuses. CI fetches full history; staged checks also inspect actual MERGE_HEAD.

The [CI workflow](../../../.github/workflows/re-coverage.yml) runs isolated
accounting, evidence-store and backup guard suites on ordinary pushes and PRs,
without fetching evidence or requiring publication. Its **Full evidence audit
(manual)** job runs the archive-backed suites and change gate only when a manual
workflow run selects `verify_evidence`. That job requires its referenced payloads
to be available; a source-only run does not claim a complete evidence audit.
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
The local staged gate remains required for commits. Ordinary CI validates the
tooling with isolated fixtures; full evidence validation is explicitly requested.

To start a reviewed baseline, use `--freeze-baseline` with a new path. This
explicitly accepts existing accounting debt without changing any original
evidence hashes. It refuses overwrite and requires a valid, reconciled audit.
Ordinary changes should use receipts rather than repeatedly freezing baselines.

### Exact execution claims and current readiness

The central register now enables `claim_bindings_required: true`. The gate rejects
disabling it after adoption. Retained Git trees predating this policy keep their
historical accounting rules; their results do not acquire current scope bindings.
Historical statuses and evidence hashes are unchanged. `current_validation`
independently reports support for implementation, comparison, integration and
replacement. Unbound, changed-scope, incomplete-source and stale results remain
explicit pending limitations. A fresh superseding result can support a current
claim while older records remain stale.

For a new execution record, set its evidence definition's `claims_pointer` to a
JSON list in the immutable report. Each item contains `behavior`,
`contract_sha256`, and an optional `scenarios` map from scenario ID to its exact
contract SHA-256. Prepare these declarations **before** running the experiment:

```sh
python3 tools/draft-coverage-claims.py --behavior <behavior-id> \
  --scenario <scenario-id> --output working/tests/claims-next.json
```

The declaration is not execution evidence and contains no success assertion.
Include its claims and source fingerprints in the executed report, verify that
the sources remained stable, and register the actual outcome under a new ID.
Do not attach declarations retrospectively to old records to make them current.
The declared behavior/scenario/evidence links must agree in the register.

Behavior identities include the scoped input domain, original-build references,
implementation and test paths, and optional `validation_dependencies` and
`validation_domain`. Statuses and evidence IDs are excluded to avoid a cycle.
`validation_domain` can describe required inputs, branches, failures and explicit
exclusions; reviewing its scientific adequacy remains a research responsibility.
Scenario identities include level, scope, behavior IDs and test paths. Widening
either identity leaves prior results historical. All implementation and linked
test paths and explicit dependencies must appear in a supporting result's source provenance;
integration/replacement additionally require a bound scenario and its test-input
fingerprints. Compiler dependencies are not inferred: review shared headers,
build settings, libraries and data inputs explicitly.

The dashboard distinguishes source-fingerprint freshness from scope-bound current
validation. Milestone achievements retain historical stage accounting; they count
as current only when all required stages have appropriate bound execution proof.
Track targets are checked against the status schema, including
`integration: live_equivalence` and `replacement: scoped_live`.

### Receipt composition and census scope

Exact A-to-B and B-to-C receipts can now satisfy an A-to-C accounting delta.
The gate reports the receipt chains it used and requires a continuous hash path
for every changed file and behavior. Empty reasons, missing transitions, and
unsupported final recorded-validation claims still fail. Intermediate execution
results remain historical; a chain cannot promote a final status without current
execution evidence. This is content/contract accounting, not an audit of which
Git commits contained each intermediate version; retain the committed-history
review required by AGENTS.md.

The census also includes root build/dependency files (`CMakeLists.txt`, Makefiles,
Meson files, Python project/requirements files and CMake presets), and component
`.cmake`, `.mk`, `.pro` and `.pri` inputs. New build formats still require an
explicit census-scope review. Exclude immutable originals and generated working
trees. Adding a build file does not validate the code it builds.

Preserve existing snapshots and immutable results. For future work, prefer one
reviewed durable census per coherent change, retain full per-run reports under
`working/`, and regenerate summaries only when they communicate a changed
milestone. The scenario corpus should grow around complete player journeys and
failure/recovery transitions; checklist totals are not a release criterion.

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

A recorded change review may name `validation.stages`, a nonempty unique list
drawn from `implementation`, `comparison`, `integration`, and `replacement`.
For example, `"stages": ["implementation"]` requires fresh source-bound execution
for the implementation while preserving an unchanged historical live milestone
as independently pending. It grants no comparison, integration or replacement
promotion credit. Each promoted dimension must have its own matching recorded
stage credit; listing an inactive dimension or malformed/duplicate stages refuses.
Omitting `stages` retains the original requirement to validate every currently
declared milestone. File hashes, evidence freshness/kinds, behavior contracts,
scenario requirements for selected stages, and explicit pending-gap accounting
remain mandatory. This separates implementation closure from wider acceptance;
it does not make changed source equivalent to historical original/live results.

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

### Independent merge-parent evidence ID collisions

When two independent branches minted the same evidence ID for different reports,
keep the first-parent record at its ID and import the other under a distinct ID.
Optional `evidence_id_imports` entries name `commit`, `historical_id` and
`imported_id`. The Git gate requires an actual committed or pending merge, exact
imported record equality except ID, and an independent actual parent containing
the other canonical record. Report hashes, assertions, source bindings and old
receipts remain unchanged; imports do not grant validation or status credit.
Previously recorded import mappings are append-only. Naming an unrelated branch,
changing an imported report or using an ancestor as a collision witness fails.

Evidence-ID import contexts include committed merges reachable from HEAD and
from verified MERGE_HEAD tips during a real pending merge. This preserves an
earlier collision resolution carried by an incoming branch. Merely naming an
unmerged branch, including one with a resolved historical collision, supplies
no context; exact immutable records and independent co-parents remain required.

Staged checks capture the audit snapshot, reviews and any local/bootstrap
baseline from the index, then release that temporary tree before materializing
a Git base. Scratch storage holds one full repository tree at a time. Unstaged
reviews or baseline changes cannot alter the captured staged metadata; all
original source/gap/history/validation checks remain in force.

### Review journal storage

`change-reviews.json` keeps the active receipts and an ordered `archives` list.
Each archive is a repository-relative, SHA-256-addressed JSON shard under
`review-archives/`. The gate loads archived receipts before active receipts,
verifies their hashes, and enforces the same append-only receipt comparison
across filesystem, staged and historical Git trees. Archives are journal storage,
excluded from their own source fingerprints just like the active journal.
Legacy journals without archives remain supported.

Run `python3 tools/coverage_reviews.py` to split a growing journal into compact
32 MiB shards without changing receipt values or their order. Append new receipts
to the active `reviews` array; retain every referenced archive. This does not
change evidence reports, source fingerprints, historical claims or validation.
A deletion at the tip cannot remove an oversized blob from an earlier commit;
repair unpushed history before retrying a rejected push.

### SQLite review workspace and automatic exports

Use `tools/coverage-bookkeeping.py` for routine change reviews. Its local SQLite
database at `working/coverage/bookkeeping.sqlite3` owns drafts, accepted local
receipts and indexed file/behavior/gap/evidence relationships. The committed text
journal and its existing archive shards remain the durable authority for clean
checkouts, historical Git trees, staged checks and CI. SQLite requires only
Python's standard library; no service or third-party package is needed.

After reviewing the implementation/test links and updating any semantic findings
or evidence definitions, prepare the review:

```sh
python3 tools/coverage-bookkeeping.py prepare --base HEAD \
  --index-review-reason 'Reviewed the affected source links; describe the actual indexing review here.'
```

This imports the historical journal without changing it, generates and pins a
new content-addressed source census, discovers exact changed file/contract hashes
and new gap identities, and stores a draft in SQLite. Omit
`--index-review-reason` when the existing census is current. The supplied reason
acknowledges an indexing review only: the tool cannot infer semantic mappings,
reconcile focused-register changes, renew evidence or review source code for you.
Inspect the printed draft and gate errors before accepting it. Existing source
censuses, original artifacts, evidence hashes and receipt values are preserved.

Record the supplied review and validation decisions, exporting automatically:

```sh
python3 tools/coverage-bookkeeping.py record \
  --reason 'Describe the concrete change, affected contracts and review.' \
  --validation-reason 'Describe checks run and the remaining validation boundary.'
```

`record` uses the latest prepared draft; supply its ID explicitly when managing
several drafts. Validation defaults to `pending`. Use `--validation recorded`
with repeated `--evidence <id>` and optionally `--stage <dimension>` for fresh
registered execution evidence. The existing gate enforces its source, scope,
scenario and evidence-kind requirements. `--validation not_required` retains the
existing restriction to eligible support work. Reasons must be substantive; the
tool fills mechanical fields, never review judgments or validation claims.

Recording freshly recomputes both Git-base and working snapshots. Changed source,
contracts, census or HEAD invalidate the prepared draft; prepare again after
edits. The gate receives the same exact receipt values as the manual workflow.
Partial reviews can use repeated `prepare --path <exact-changed-path>` and/or
`--behavior <exact-changed-id>`. A selected shared file includes all its affected
behavior contracts; all new gap identities remain explicit in the draft. Partial
recording may leave other changes unreviewed. The result reports `gate_passed`
and every remaining `gate_errors`; successful receipt storage does not claim a
passing gate. New receipt-validation errors refuse recording.

Useful queries and recovery commands:

```sh
python3 tools/coverage-bookkeeping.py sync
python3 tools/coverage-bookkeeping.py find --path tools/coverage_gate.py
python3 tools/coverage-bookkeeping.py find --behavior TOOL.coverage-gate --validation pending
python3 tools/coverage-bookkeeping.py export
```

`sync` losslessly imports exact historical values and rejects duplicate IDs,
rewrites, deletions and reordering of imported receipts. Imported records are
append-only in SQLite. The database is bound to its worktree, preventing receipts
from another checkout being mixed in accidentally. `export` appends accepted local
receipts to the active JSON journal with deterministic serialization, preserving
all archive bytes and journal extension fields. No-op exports preserve every
byte. Cooperating writers serialize through a worktree lock; changed output bytes
are checked before atomic replacement. External editors must not modify the
journal during export. Other completed external appends are imported and retained.

The database transaction commits before export. A failed export leaves its
accepted receipt available for retry; retry also tolerates a crash after journal
replacement but before the import metadata commits. `record --no-export` keeps a
receipt local until `export`. Exported receipts rebuild the SQLite workspace from
a fresh checkout; unexported drafts/receipts exist only in that local database,
so export before discarding it. Do not commit the binary database.

Finally inspect/stage the generated journal, census and register changes along
with the implementation, tests and documentation. Run the normal staged gate
against `HEAD`. The workflow never stages files, commits, refreshes historical
evidence fingerprints, infers behavior mappings or uses a cached gate pass.
The current migration covers change reviews; the behavior register and immutable
execution reports remain in their established text/file formats.


### Active and historical implementation reviews

`implementation_reviews` indexes only active static reconciliation documents,
whose entries match each behavior's exact `implementation_review` backlink.
Merged documents with no remaining active backlinks are retained unchanged under
`implementation_review_history`; their files and recorded hashes remain available.
Keeping every superseded version in the active list duplicates decisions and can
prevent the dashboard from loading. Moving an inactive reference preserves its
record and grants no new engine understanding, implementation or validation.

## SQLite evidence and automated bookkeeping

On a fresh clone, run `python3 tools/evidence-store.py fetch --verify`, then
`python3 tools/install-coverage-hook.py`. The manual full CI audit verifies pinned SQLite archives
without expanding their JSON files. The audit, gate, coverage UI and migrated
research readers read original bytes directly through `coverage_evidence.Reader`.
Readers verify content hashes and preserve historical source fingerprints; they
do not hydrate the working tree. Large reports,
generated summaries, binary captures and old receipt shards are exact SQLite
objects outside the Git object history; source, handwritten research, live
registers, the active review journal and small pinned manifests remain in Git.
The clean filter stores a pointer containing the original SHA-256 and byte count.
Restoration validates compressed downloads, the database and the original bytes.
A missing or corrupt object fails closed when read. Source pushes require valid
pointer identities, not remote payload availability. Archived findings retain
their original hashes and claims.

Generated reports under `research/runtime/` and `research/formats/` fit this
storage policy. Editable central/focused registers, the active review journal,
gate baseline, source locks, UI policy, wire schemas, test fixtures and authored
game/launcher configuration remain JSON. Bootstrap manifests stay outside the
database so a fresh clone can find and verify its archives.
New immutable research exports can use `coverage_evidence.write_report`;
`coverage-bookkeeping.py prepare --index-review-reason ...` and the source-index
CLI already write managed census outputs directly to SQLite. Outputs explicitly
requested under `working/` remain ordinary JSON for external tools.

Inventory and compact an existing expanded checkout explicitly:

```sh
python3 tools/evidence-store.py inventory-json --output working/retention/json-inventory-<new-id>.json
python3 tools/evidence-store.py compact-json \
  --output /var/local/magic-and-mayhem-archive/json-compaction-<new-id> --apply
```

Compaction preserves exact payloads in SQLite, verifies a cold copy and a real
restore, and writes matching receipts to the checkout and cold archive before
replacing expanded files with small references. It rechecks source identities,
retains negative evidence, preserves file modes and never stages files. A
concurrent change aborts replacement; inspect the retained plan/partial receipt.
Resume a reviewed interrupted plan with `evidence-store.py apply-plan <plan.json>`.
The pending database also records queryable `json_documents` path, hash, size,
success, scope and schema metadata for each sweep. Local pending evidence can be
read before publication; source pushes do not require payload publication.

To inspect or hand a report to a legacy external reader, export exact bytes:

```sh
python3 tools/evidence-store.py export research/runtime/<report>.json --output working/<report>.json
```

`hydrate <research-path>...` explicitly restores selected legacy inputs in place.
Bare `hydrate` remains an optional full expansion and index refresh; it is no
longer part of routine auditing or CI. Do not edit a reference as JSON. Export it,
make a new result under a new evidence ID, and publish its captured bytes before
pushing. Git references retain existing paths so recorded hashes and links do
not need a semantic rewrite.

Use `python3 tools/coverage-bookkeeping.py prepare --base HEAD` and `record` with
concrete `--reason` and `--validation-reason` values. Recording recomputes source,
contract and HEAD fingerprints, appends the exact SQLite receipt and exports the
text journal automatically. `--index-review-reason` explicitly reviews and pins
a new source census. Inspect `gate_errors`; a partial receipt is not a gate pass.
Registers and exported receipts remain reviewable text, and no cached result
approves a later tree.

Local completion and evidence distribution are separate operations. Create one
owned worktree per bounded task batch; keep concurrent batches in their own trees.
For example, start from a clean reviewed commit with
`git worktree add -b native-next-batch working/worktrees/native-next-batch HEAD`.
Run the appropriate bounded tests before recording the batch.

Finish a local batch with concrete review and validation reasons:

```sh
python3 tools/finish-work.py \
  --reason 'Reviewed the owned changes and their explicit behavior links' \
  --validation-reason 'Describe executed checks and remaining validation boundaries' \
  --index-review-reason 'Reviewed changed source links and census findings' \
  --message 'Describe the bounded change'
```

The command prepares exact receipts in the local SQLite workspace, verifies the
whole change gate, exports accepted receipts, makes verified consistent SQLite
backups, stages the owned worktree and commits with the normal hook enabled.
It requires all configured archive snapshots locally; `evidence-store.py fetch
--verify` is a separate explicit setup operation. Local completion never uploads
or fetches evidence. Its temporary Git trees can resolve the local pending store;
every resolved payload still undergoes its original hash and size checks.

Local Git merges, including integration into local master, use the same local
evidence and verified-backup requirements. They never require publication or
network access. Keep the evidence database, review workspace and SQLite backup
catalogs under retained local storage. In this checkout the persistent evidence
database is `working/coverage/evidence-pending.sqlite3` in the main checkout;
owned integration worktrees resolve that same store through their local pending
database link. Review workspaces remain worktree-local. `finish-work.py` returns
the commit ID and verified backup paths together; preserve that result and the
backup catalogs. Git contains report identities and exported accounting; a Git
clone alone does not contain unpublished evidence payloads.

A normal local finish therefore ends with a Git commit and retained SQLite
backups. Creating a GitHub release is optional and separate. Remote source
distribution can proceed without publishing payloads. The pre-push guard checks
pointer format and pre-migration history without opening stores or fetching
archives. Full evidence audits need the referenced verified SQLite inputs;
ordinary CI runs isolated tooling tests and the archive-backed audit is manual.
Recipients may obtain payloads separately or use the optional release backend.
Unpublished references do not assert that a fresh clone can read their reports.

Private SSH backup is an explicit operation after local completion:

```sh
git config mm.evidenceBackupHost <ssh-alias>
git config mm.evidenceBackupDirectory /absolute/private/project-directory
python3 tools/working-retention.py run --reserve-gib 4 -- python3 tools/backup-evidence.py
python3 tools/backup-evidence.py --status
python3 tools/backup-evidence.py --retry
```

The host must already be accessible through SSH and provide Python 3.11 or newer
and rsync. The command uses SQLite's online backup API for the main checkout's
evidence and review databases and the selected worktree's review database, if
present; `--review-db` includes an additional worktree's drafts. Other worktrees'
unexported drafts are not discovered automatically. Locally cached pinned archive
databases are included so database restoration does not require a public release
download. Live database files and their WAL files are never copied independently.

The private directory contains `objects/<sha256>.sqlite3` snapshots and
`sets/<sha256>.sqlite3` manifests identifying the Git commit and original source
of each database. Completed manifests appear only after all remote hashes and
SQLite integrity checks pass. Directories are private and completed files have
mode 0600. Existing objects are verified and reused; corruption refuses rather
than overwriting a prior object. Interrupted transfers remain pending in
`working/coverage/private-backups/catalog.sqlite3` in the main checkout. Retry
uses the original retained snapshots; it does not refresh historical evidence.
Nothing is deleted automatically, and no commit hook invokes this command.

For restoration, download the selected set manifest and query its `backup` and
`files` tables. Retrieve its named objects over SSH, check their SHA-256 values
against `files.sha256`, and verify SQLite integrity before opening a copy. The
`source` column identifies each database's original location; restore pending
evidence as `working/coverage/evidence-pending.sqlite3` while its writers are
stopped, and place pinned archive objects in the configured evidence cache.
Keep the recovered Git commit with the database set. This backs up databases;
untracked game installations, original media and Git source history are outside
the backup command's scope. Private backup is separate from optional public
release publication and does not make those payloads available to hosted CI.

Use `--record-current-promotions` only after reviewing the requested milestone
changes. It records stage-specific proof from existing current audit support;
it does not change statuses, manufacture claims, or grant other stages credit.
Missing, stale, incomplete or unbound execution proof refuses completion. Without
that option, new promotions require previously recorded valid proof receipts.
Failed checks or commits preserve source, accepted SQLite receipts and backups.

Only when evidence publication is explicitly requested, synchronize from a clean
owned worktree (this command publishes to GitHub Releases):

```sh
python3 tools/sync-evidence.py \
  --output working/evidence-publications/<new-id> --tag evidence-<new-id> --push
```

Synchronization gathers only unpublished payloads across outgoing commit history,
including report versions absent from HEAD. The existing publication transport
verifies uploaded/downloaded immutable SQLite assets before updating the manifest.
The command then records and commits availability metadata. Omit `--push` to
publish evidence and commit its manifest without pushing the source branch.
An upload failure prevents this command's optional branch push. A normal
`git push` does not require publication. The manually requested full CI audit
verifies published evidence from a fresh checkout; ordinary CI uses isolated fixtures.
Publication changes storage availability, never an engine validation milestone.

`.evidence-commits.json` maps old commit IDs to their storage-only rewritten IDs.
Historical Git-base and merge-journal reads restore pointer bytes using current
tooling, without executing historical code. Merge import IDs retain their old
recorded values and are translated only for graph lookup. Force-push CI maps its
predecessor through the same table. The original graph remains in a separately
encrypted rollback release; keep the private recovery key on both local drives.
Historical branches need current tooling, or the current tool with `--root`, to
restore their pointers. This migration supplies no new original-engine comparison
or live replacement evidence.

Shared local hooks may use the main checkout's absolute .githooks path. They run current gate tooling with `--root` against the selected worktree; staged content and older branch source stay local. The installer preserves an existing absolute path belonging to this checkout.
