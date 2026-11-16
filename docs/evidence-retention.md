# Evidence acceptance and finite retention

User approved 2026-11-15. This is the repository-wide retention policy. It
supersedes indefinite retention, mandatory backup/checkpoint creation at each
finish, perpetual pins and replacement/restore requirements for intentional
expiry in older instructions, including machine-local notes. Acceptance and
retention are separate decisions. Existing storage receipts remain historical
facts; their original hashes are never rewritten.

## Acceptance

Every commit requires appropriate evidence for its exact change: focused tests,
source and contract review, census/accounting and the normal staged hook. Ordinary
prose needs proportionate review, not an engine run. New engine validation or
status promotions require fresh, scope-bound execution/comparison proof. Explicit
pending validation never claims equivalence or substitutes for required checks.
The exact combined candidate is checked again before integration. Missing proof
required for that candidate blocks acceptance; unrelated historical payloads do
not. No cached gate pass is authoritative.

Keep the supporting reports, necessary inputs/raw observations, configuration,
source state and tested executables through integration plus **14 days**. Start
the clock when integration is observed, not from the source commit's timestamp.
Pending integration does not expire by age. A custom promotion must record its
clock with `tools/evidence-lifecycle.py integrated --commit <exact-commit>` after
releasing admission. The standard integration wrapper records candidate and tip
clocks automatically. Recording an older integration today starts its window
today; never backdate custody from a Git author/committer date.

`tools/evidence-lifecycle.json` is the authored configuration: a 14-day review
window, 2 GiB maximum unique new/changed report payloads in one accepted source batch and no
automatic finish snapshots. Reused retained reports are verified without charging them as newly retained
bytes. This report limit is not a total disk-usage quota;
raw captures/builds need the machine's producer reservation and working budget.
Narrow acquisitions or explicitly review a budget change before exceeding it.
Existing archive pairs retain their verified-copy semantics when selected;
finite lifetime does not silently weaken an explicit two-copy operation.

Finish verifies consumed/new report payloads and records exact commit, tree,
base, review receipts, result/limitations and payload identities in shared ignored
SQLite storage. Git retains the exact source and accepted active review exports and immutable archive reference identities.
This compact acceptance record does not preserve deleted supporting bytes.
Do not automatically snapshot every review database, duplicate every source
batch, seal every execution tree or retain every historical executable forever.
Backups and external archives are optional deliberate recovery choices.

## Expiry and holds

Expiry permits review; it never deletes data automatically. A reference, evidence
ID, successful test, pin or merged branch does not grant perpetual custody.
Longer retention requires a named purpose, owner, estimated allocation, storage
budget and review/release date. Examples are an unresolved regression, an active
consumer or a selected comparison witness. Revisit holds; unavailable storage is
not an instruction to create more backup copies. Keep acquisitions small and use
one active task checkout on constrained machines unless concurrency has a budget.

Keep original artifacts immutable, unique source changes, reachable source
history and user saves/configuration. Current source census, dependency index,
active accepted review journal and other operational gate inputs must remain readable
while those versions are in use. They have a current consumer: superseded physical
snapshots, complete bookkeeping schemas, drafts and caches do not all inherit
that obligation. Accepted receipts must be exported before a cache is discarded;
keep unexported/unfinished unique work. Unchanged archived review shards are
checked by their exact accepted reference identities during scoped checks, without
reopening their historical bodies. New/changed shards and active receipts still
require their bytes. Full journal/audit operations require the original shards. A future replacement for operational
metadata must migrate its consumers explicitly rather than treating missing data
as a gate pass.

For expired supporting evidence:

1. Review the exact accepted source/evidence identity, integration clock, pins,
   active tasks, process/file consumers and remaining comparisons. Discharge
   consumers or record a budgeted hold. A plan's reviewer statement is part of
   this review; automated ancestry and open-file checks cannot infer ownership.
2. Preserve an accepted compact declaration with original ID/hash, definition,
   result, source/integration identity, reproduction command/inputs, owner,
   retirement reason and lost verification capability. Prepare observations/plans
   in SQLite; export the declaration as the authored retirement control journal.
3. Commit and integrate the declaration with the normal gate while original
   reports are still available. New declarations verify the unchanged original
   report. Subsequent scoped checks retain historical results, mark retired
   evidence unavailable and grant it no current validation credit.
4. Remove the reviewed bytes outside admission. Intentional expiry needs neither
   a replacement copy nor a restore of bytes deliberately relinquished. Keep
   exact hashes/results, distinguish missing data from intentional retirement,
   and record actual allocated bytes reclaimed. Unknown files remain held.

Unavailable unchanged historical reports are distinct from accepted retirement.
Scoped accounting may preserve their exact base definitions and report the gap,
but grants them no current validation credit or deletion permission. New/changed
reports still need original bytes. Unchanged original inventories may be missing
only while original build/mapping metadata remains exactly accepted; those mapping
checks and associated validation remain explicitly incomplete. New mappings and
validation promotions require their appropriate original proof. Available corrupt
payloads are errors, never treated as absent. Missing historical applicability
reports grant no preservation credit.

A complete evidence audit still requires original payloads and fails when they
are unavailable. Retired proof cannot establish a new validation claim or status
promotion. Recover exact original bytes or acquire a new evidence ID for new
proof. A historic pass remains a historic result; present replay/reanalysis and
whole-corpus reproducibility are explicitly relinquished after expiry.

## Commands

From an owned checkout, after the window and consumer review:

```sh
python3 tools/evidence-lifecycle.py prepare --commit <integrated-commit> \
  --evidence <id> --owner '<owner>' --reason '<why evidence can expire>' \
  --consumer-review '<tasks, pins, processes and comparisons discharged>' \
  --result '<original result and limits>' \
  --reproduction '<exact source, inputs, configuration and command>'
python3 tools/evidence-lifecycle.py export --plan <printed-plan-sha256>
```

The exported `research/runtime/coverage/evidence-retirements.json` is an authored
append-only control journal, like accepted change reviews, not a replacement for
the reports. Review, commit and integrate it before physical removal. Clocks,
acceptances, plans and cleanup receipts live outside retiring worktrees in the
main checkout's `working/coverage/evidence-lifecycle.sqlite3`.

For a stopped plain pending report store, preview and then execute:

```sh
python3 tools/evidence-lifecycle.py prune-store --database <absolute-pending-db> \
  --consumer-review '<all writers and live checkouts reviewed>'
python3 tools/evidence-lifecycle.py prune-store --database <absolute-pending-db> \
  --consumer-review '<same review>' --execute
```

A pin additionally requires `--pin-review '<owner and release decision>'`.
Only payloads declared retired on master and discharged by all live checkouts
are selected. All other objects and surviving version bindings are preserved
and hash-verified. Unknown tables, symlinks/hardlinks, live sidecars and consumers,
shared object pairs and hash-named release containers are refused. Those formats
need a source-specific retirement operation; never label a repacked database
with an old container hash. Raw acquisition trees likewise need their exact
ownership, unique-input and consumer review before scripted removal. The tool
never follows directory trees or blanket-deletes evidence databases.

## Worktrees, caches and migration

Clean, inactive, integrated source checkouts may retire through Git without
force after unique files/settings are preserved and evidence still inside its
review window has been handed to surviving storage. Reachable committed source
is reconstructible: another full checkout, source bundle and independent restore
are not prerequisites merely to remove its expanded copy. Keep normal source
backup practice separate from each task's closeout. Stop owned processes and
remove reviewed temporary output at closeout regardless of free space.

For a storage migration promising continued recovery, continue verifying complete
survivors/replacements and the required actual restores. The same operation may
expire reviewed evidence and migrate survivors, but it must name the distinction
and prove the survivors. Older compaction/archive tools retain their stricter
contracts; use them only when that preservation is intended, not to implement
expiry by pretending lost bytes survive elsewhere.
