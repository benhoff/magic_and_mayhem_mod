# Proportionate local integration

Every change uses an exclusively owned task worktree and branch. Complete the
bounded implementation, focused tests and exact accounting before integration.
The normal staged hook remains mandatory. Publication is separate from local
completion.

## Three levels of checks

| Change | Required work |
| --- | --- |
| Routine source, tooling or prose | Focused checks appropriate to the change, exact file/contract review, source census, immutable history and affected-claim gate. Prose alone needs no engine run. |
| Engine behavior or validation claims | Relevant build/address mappings, provenance and scenarios, appropriate comparisons, fresh execution for promotions, explicit remaining limits. |
| Evidence deletion or storage migration | Source-specific retention plan, consumer/pin review, required full survivor/replacement verification and actual restores before removal. |

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
   normal hook enabled, retain required evidence and pin the ready tip. Required engine acquisitions must already be durably
   sealed; an unprocessed handoff queue does not establish retention.
2. Observe master and review dependencies, committed-history gaps and conflicts.
   Prepare a combined candidate in the same owned checkout against that exact
   base. Reconcile metadata and run the required combined accounting, focused
   checks, retained backups and normal staged hook outside admission.
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

## Routine retention and explicit storage maintenance

With the configured shared pair, finish checkpoints complete review schema,
rows, drafts and auxiliary data into two verified object stores. A checkpoint-only
receipt explicitly says no real reconstruction was performed and authorizes no
retirement. The live cache stays in place. After staging, finish retains the exact
changed source blobs and cited report payloads in two stores, binding their tree
and base. It does not checkpoint/scrub every unrelated archive object.

`--evict-review-cache` is an explicit storage operation. It retains the full
checkpoint, verifies real reconstruction and checks current source/consumers
before removing the cache. Queued workspace retirement is also explicit through
`--handoff-completed-workspaces` or a selected `evidence-workspace.py seal`.
Legacy catalog migration,
private backups and full integrity scrubs keep their own stronger workflows.
`--keep-review-cache` remains compatible; keeping is now the default.

Use `--timings` for evidence preflight, affected snapshots, accounting, backup,
staging and hook durations. Full mode retains the bounded scratch and verification
worker controls described in [snapshot performance](snapshot-performance.md).

The changed-batch manifest printed by finish records its base/tree, each source
member's original-byte hash/size/Git mode and deletion markers, plus cited report
payload identities. Read it with `evidence_objects.configured(root)` and
`Pair.read(sha256, size)`. Member bytes use the same API. Reconstruct a delta only
in an owned checkout of its named base, restore recorded executable modes, then
compare its tree with the pinned tree. These are recovery inputs, not a new
engine execution claim.
