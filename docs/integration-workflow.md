# Proportionate local integration

Every change uses an exclusively owned task worktree and branch. Complete the
bounded implementation, focused tests and exact accounting before integration.
The normal staged hook remains mandatory. Publication is separate from local
completion.

## Required checks

| Change | Required work |
| --- | --- |
| Routine source, tooling or prose | Focused checks appropriate to the change, exact file/contract review, source census, immutable history and affected-claim gate. Prose alone needs no engine run. |
| Engine behavior or validation claims | Relevant build/address mappings, provenance and scenarios, appropriate comparisons, fresh execution for promotions, explicit remaining limits. |
| Intentional evidence expiry | Integration plus 14 days, consumer/pin review, accepted retirement declaration, explicit lost capability; no replacement or restore of relinquished bytes. |
| Storage migration promising recovery | Source-specific survivor/replacement verification and actual restores before removal. |

`python3 tools/check-re-coverage.py --base HEAD --staged` reads actual index
blobs. It extracts source inputs and the affected evidence closure rather than
creating full historical checkouts. Shared evidence, scenarios and dispatch links
expand scope. Discovery/policy changes conservatively widen semantic review.
The output names the checked behavior/evidence IDs and states that this is not a
full-corpus audit. No gate pass is cached.

`change_scope_index` in the central register pins a generated SQLite-backed
provenance map. Its entries bind exact evidence definitions, original report
identities and dependency paths. Repeated paths are interned; original source
hashes remain in the reports and are verified when consumed. Unchanged entries must match the actual base; new/changed entries
are decoded from verified original reports. It supplies dependency lookup only.
Generate/refresh it after adding evidence:

```sh
python3 tools/index-coverage-evidence.py
```

A base predating this index is inspected directly during adoption. Repositories
without an index retain the legacy full path. Explicit `--full`, baseline
freezing and bootstrap keep whole-corpus checks. Scoped local reads verify each
consumed payload's SHA-256, permit unpublished local evidence, and never fetch
missing archives. They do not assert fresh whole-container checks for untouched
SQLite archives. Use explicit fetch/verify, full audit and storage maintenance
when those guarantees are needed.

## Validate before promotion admission

1. Finish the feature in its owned checkout with `tools/finish-work.py`; keep the
   normal hook enabled, retain required evidence and pin the ready tip. Supporting
   evidence remains available through integration plus the 14-day review window;
   a queued handoff does not establish custody.
2. Observe master and review dependencies, committed-history gaps and conflicts.
   Prepare a combined candidate in the same owned checkout against that exact
   base. Reconcile metadata and run the required combined accounting, focused
   checks, required in-window evidence and normal staged hook outside admission.
3. Pin the resulting candidate commit and tree from the invocation-bound finish
   result, not a later reading of HEAD. The commit's tree must equal the
   staged tree checked/retained by finish. Preserve the original feature tip.
4. Acquire admission with the observed master as `expected-master`. Recheck the
   exact candidate commit/tree/branch and clean candidate/master checkouts, then
   fast-forward master to that immutable checked commit. Release immediately.

If master changes during preparation, promotion refuses and preserves the
candidate. Prepare/revalidate against the new base outside admission. Candidate
mutation, failed checks, missing inputs and conflicts also preserve work and
never reserve the promotion lock. Do not perform acquisition, backup, testing,
debugging or decision waits under admission.

`tools/integrate-work.py` implements this sequence, including compatible ready
batches. A feature pass cannot substitute for checks of a different combined
tree. Preview is read-only:

```sh
python3 tools/integrate-work.py --tip <ready-feature>
python3 tools/integrate-work.py --tip <ready-feature> --execute \
  --branch integrate/<unique-batch> --db working/coverage/<batch>.sqlite3 \
  --report research/runtime/<batch>/preparation.json \
  --expected-master <observed-base> \
  --reason '<combined review>' --validation-reason '<tests and limits>' \
  --index-review-reason '<reviewed source links>' --message '<combined change>'
```

The installed reference-transaction guard still requires a valid inherited lease
for every master update. Install current hooks from a durable checkout with
`python3 tools/install-coverage-hook.py`. Custom operators wrap only their checked
promotion in `tools/with-integration-lock.py --execute --expected-master <base>`;
recheck the pinned candidate and checkouts before updating master. Use `Lease.run`
and inherited descriptors for child Git calls. Do not wrap preparation or finish
in this lock. `finish-work.py` requires an owned branch and refuses admission.

## Cleanup after promotion

Release promotion admission before workspace cleanup. Before reporting task
completion, remove reviewed temporary output and retire the completed owned
worktree through [the closeout and retirement procedure](storage-consolidation.md#task-closeout-and-worktree-retirement).
If a checkout or allocation must remain, record its allocated size, owner,
specific reason and release condition. Cleanup is required even when free space
is above the maintenance target; required evidence and recovery inputs retain
their custody protections. Keep hooks and recovery records in a surviving durable
checkout, and run retirement from outside the selected tree.

## Recurring metadata and history reviews

Both helpers preview by default. Mutation requires `--execute`, a concrete reason
and a new SQLite-backed report path under `research/runtime/`:

```bash
python3 tools/review-integration.py metadata
python3 tools/review-integration.py metadata --execute --reason '<reviewed disjoint additions>' --report research/runtime/<batch>/metadata-union.json
python3 tools/review-integration.py history --base <last-reviewed-commit> --tip <incoming-tip> --tip master
python3 tools/review-integration.py history --base <last-reviewed-commit> --tip <incoming-tip> --tip master --execute --behavior <affected-id> --reason '<exact range reviewed>' --db working/coverage/integration-review.sqlite3 --report research/runtime/<batch>/history-review.json
```

Metadata reconciliation requires an actual pending merge. It handles only the
central register, active review journal and append-only ledger tails, preserving
whole keyed records. Differing edits to the same record, deletions, unknown
conflicts and non-append-only ledger edits require manual review. Only resolved
metadata paths are staged. A conflicted census pointer stays at the first parent
and is explicitly flagged for regeneration. Review the result and regenerate the
combined census before the gate. No semantic coverage or evidence-ID collision
resolution is inferred, and nothing is committed.

History review deduplicates Git blob reads. Repeat `--behavior` for every affected
registered contract and `--pending-gap` for new limitations. Existing exact receipt
pairs retain their pinned hashes; previously unreceipted pointers are verified
against retained payloads before recording. New receipts explicitly carry pending
retrospective validation. They assert neither a past gate pass nor new execution.
The normal combined-tree review and staged gate remain required.

## Acceptance and finite retention

Follow [the evidence lifecycle](evidence-retention.md). Routine finish verifies
used report bytes, preserves exact review exports and records a compact acceptance
in shared SQLite storage. It does not automatically duplicate complete databases,
source batches or execution trees. Optional `finish_backups` retains the legacy
snapshot/pair workflow when deliberately configured. `--evict-review-cache` keeps
its explicit recovery contract; ordinary expired cache disposal follows lifecycle
review instead. Incomplete unique drafts must be exported or held.

After releasing admission, the standard integration wrapper records clocks for
its exact candidate and incoming tips. Custom promotion records inclusion with
`tools/evidence-lifecycle.py integrated --commit <commit>` after releasing the
lease. Never start expiry from a source commit's date. Evidence remains required
through integration plus 14 days and any reviewed active hold. Expiry declarations
are accepted under normal hooks while original reports are available, then a
separate cleanup may remove the reviewed bytes. It never runs from a hook or
launcher. Full audits still require original reports; retired reports cannot
support new validation or promotions. Keep current operational accounting inputs
readable until their consumers are migrated.

Use `--timings` for actual scope review, payload verification and staged-hook
costs. Explicit archive/migration operations preserve their stricter contracts,
including exact copy counts and restore proofs. An old hash never names a
repacked container. Private backup/publication remain explicit independent work.
