# Storage consolidation

The [finite retention policy](evidence-retention.md), approved 2026-11-15,
is authoritative for acceptance, expiry, routine finish and worktree closeout.
The preservation/restore procedures below apply when continued recovery is
promised. They do not require retaining deliberately expired evidence, every
bookkeeping database or reconstructible committed source forever. Historical
copy receipts remain true for their recorded phase. Intentional expiry needs
an accepted retirement record and consumer review, not a replacement copy.

Generated reports are shared by payload identity; execution state stays private.
Use the actual machine's storage floors. The older archive examples below use a
90/100 GiB layout and are not defaults for a constrained development workstation.

## New output

New review workspaces use schema 2. They fingerprint imported journal receipts
instead of duplicating every historical body and normalized link. They retain
their own accepted receipt bodies and every draft. Sync still refuses rewritten,
removed or reordered history. A reviewed migration can accept a merged journal's
new order only when all old IDs remain and every shared receipt body matches.

Set repository-local mm.evidencePending to an absolute ignored working/ path
to share immutable report payloads across worktrees. Readers also consult a
worktree's existing local store. Both write_report and Git's configured evidence
filter use the shared destination. Repository-local mm.evidenceExtra entries name
retained legacy SQLite stores for read-only fallback. Local completion propagates
these paths into temporary Git trees without copying their payloads. Pin these
stores and keep their paths available; consolidation may replace a path with a
verified shared-store alias. Older runner code that writes its own store
still needs a reviewed migration; this setting does not rewrite frozen sources.

Set both mm.snapshotStore and mm.snapshotColdStore to stores on independent
filesystems to opt local completion into exact chunked SQLite backups. Completion
first retains a consistent SQLite snapshot, verifies its evidence objects, stores
both copies, performs a real cold restore and only then removes the raw backup.
Finite-policy finish does not create these backups by default; enabling
`finish_backups` opts into this legacy preservation workflow. A failed compaction keeps
the raw verified snapshot and fails completion. Private transfers keep their
existing immutable snapshot/manifests format and temporarily materialize archived
snapshots when explicitly invoked. Cleanup itself sends no data remotely.

## Recovery

Use the current main checkout's tools. Frozen historical readers predate the new
catalog representations.

    python3 tools/sqlite-snapshot-store.py restore --catalog /path/to/cold/store.sqlite3 --sha256 <original-snapshot-sha256> --output /new/path/snapshot.sqlite3

    python3 tools/consolidate-evidence.py restore --catalog /path/to/cold/source-catalog.sqlite3 --source /old/worktree/working/coverage/evidence-pending.sqlite3 --cold /path/to/cold/evidence-pending.sqlite3 --output /new/path/evidence.sqlite3

    python3 tools/restore-worktree-source.py --catalog /path/to/source/store.sqlite3 --snapshot <retirement-snapshot-id> --output /new/source/directory

Snapshot restoration verifies every chunk, whole-file identity, SQLite integrity
and foreign keys. Evidence consolidation preserves each source's original report
bytes, bindings and metadata; its explicit restore reconstructs the original
logical tables rather than a mutable database's page layout. Unknown tables,
binding conflicts, active consumers and unsafe outputs refuse consolidation.
Hardlinked mutable stores are held for a separate alias/activity review.

Legacy archive history remains byte-exact in verified gzip files on both drives.
Small append-journal rows refer to immutable, compressed SQLite records. The
current working-retention.py restore resolves both representations. Keep the
history indexes, compressed history, append journals and .records.sqlite3
sidecars together. A restore scans the retained history and can take over a minute.

Review-cache migrations retain their original byte-exact database in both
snapshot stores. Known embedded evidence/check tables are copied and independently
fingerprinted; pending receipts and draft bytes remain in the compact cache.
Schema 2 caches require current bookkeeping code. Restore the original schema 1
snapshot to a separate working path when reproducing an older tool.

For the existing compaction command, clean merged worktrees may become sparse
only after every removed tracked file matches its Git blob, is cold-backed and
is restored independently.
Registered Git metadata, branches, hooks, originals, untracked files and all
working/ outputs remain. WORKTREE-ARCHIVED.txt names the exact source snapshot.
Modified, pinned, shared, recent or active source checkouts remain intact under
that command. Full retirement uses the separate reviewed procedure below.

## Task closeout and worktree retirement

Task cleanup remains mandatory. Follow [finite retention](evidence-retention.md):

1. Select the exact linked checkout, branch and HEAD. Confirm integration, clean
   source, no unfinished Git operations and explicit task completion. An ancestor
   HEAD alone cannot establish that another owner's work is finished.
2. Inventory ignored/untracked data, unique source/settings, symlinks/hardlinks,
   pins, locks, processes and path consumers. Unknown data and unresolved owners
   remain held. Never follow links to delete shared data or unlock another task.
3. Preserve unique source, saves/configuration and reachable committed source.
   Hand off supporting evidence still inside integration plus 14 days or a named
   budgeted hold. Retire expired evidence with its accepted declaration and lost
   capability record. Current gate metadata must remain readable. Reproducible
   compiler/staging/install output may be removed when its consumers finish.
4. Update surviving reader/writer paths and hooks before removing old paths.
   Export accepted review receipts and preserve unfinished unique drafts; do not
   archive every historical bookkeeping database/schema merely to remove a cache.
5. Record the exact inventory, source identity, decisions, custody/expiry and
   remaining limitations in a hash-bound SQLite plan outside the selected tree.
   Reconstructible committed source needs no additional bundle or independent
   restore merely to remove its checkout. An optional migration that promises
   continued recovery still requires its actual survivor/replacement proofs.
6. Recheck clean source identity, integration and inactivity immediately before
   scripted removal. Invoke `git -C <surviving-checkout> worktree remove <tree>`
   without force; preserve branches/ready refs. A refusal stays held. Release
   integration admission before any cleanup.
7. Record the result and reclaimed allocated bytes separately from free-space
   changes. Holds name allocation, owner, purpose, budget and review/release date.

Newly created temporary output may retire at closeout despite recent timestamps.
This does not expire in-window evidence or another task's active inputs. Existing
archive/compaction commands below intentionally retain their preservation
contracts; their stronger restore requirements are not prerequisites for finite
expiry or ordinary committed source-checkout removal.

### Redundant report materializations

Historical checkouts can appear modified because a generated report was expanded
from its committed SQLite reference. `tools/compact-worktree-reports.py` offers a
smaller storage operation before full retirement: replace only expanded bytes
whose size and SHA-256 exactly match that checkout's existing Git reference.
Unique edits, authored registers, hardlinks, report symlinks, working data and
original inputs are ineligible. This preserves the registered checkout and does
not authorize its removal or renew engine validation.

Run from an owned maintenance checkout. Review the target's pins, consumers and
recovery routes first. Both planning and application require an inactive, merged
linked checkout without staged changes, locks or unfinished Git operations.
Planning skips reports written within 24 hours. Every selected payload needs two
verified copies on independent filesystems and a real restore from each; the
complete SQLite execution plan is also checkpointed and restored before mutation.
Missing copies refuse by default. `--retain-missing-copies` explicitly retains
the exact Git-bound bytes in the maintenance checkout's configured pair, then
rechecks both restores; it never accepts corrupt copies or lowers reserve floors.

```bash
python3 tools/compact-worktree-reports.py --main MAIN --own MAINTENANCE \
  --restore-directory RAM plan OLD_CHECKOUT --output working/report-plan.sqlite3 \
  --reason 'Exact reviewed ownership, pin meaning, consumers and replay limits'
python3 tools/compact-worktree-reports.py --main MAIN --own MAINTENANCE \
  --restore-directory RAM apply working/report-plan.sqlite3 \
  --expected-plan-sha256 HASH_RETURNED_BY_PLAN
```

Application verifies the unchanged plan, operator, HEAD, activity and both payload
copies again. Atomic replacement preserves mode, group, extended attributes and
modification time; the plan retains prior file identity/metadata. Each completed
replacement and the final custody receipt survive in SQLite. An interrupted
application retains its completed prefix; inspect it and prepare a new plan for
the remainder. Preserve the prior outcome/notice before explicitly removing a
task-owned old notice; an existing different or shared notice refuses overwrite.
Do not reuse the original plan hash after execution. Complete plan checkpoints
group immutable object writes while performing an actual schema/row restore;
both stores commit and sync before the verified receipt admits replacement.
The execution journal commits each 64 replacements and flushes the completed
prefix on ordinary failures. A power loss can leave one group marked prepared
after its pointers were written. Reconcile those exact pointer bytes against
the retained before identities/allocation rather than trusting state alone;
both original payload copies and the admitted plan already survive independently.

`WORKTREE-REPORTS-CANONICALIZED.txt` records the replay boundary. Current SQLite
readers resolve references directly. Frozen readers requiring physical JSON, binary or source bytes must
hydrate selected paths with current `MAIN/tools/evidence-store.py --root
OLD_CHECKOUT hydrate <path>...` before replay. Verify the reader's configured
payload routes before accepting a handoff. Pins on comparison data and historical
source remain meaningful; this operation changes only redundant report encoding.
No full-tree, physical JSON or all-concurrent-writers compatibility is claimed.

### Tracked directories and snapshot unions

`tools/compact-evidence-storage.py --root MAIN worktree CHECKOUT --scratch RAM`
compacts only tracked `research/` and `tests/` directories of an explicitly
selected clean, merged, inactive linked checkout. It holds recent, modified,
hardlinked, source-pinned and active trees. Every removed file must match its Git
blob and survive two verified shared stores plus a real independent restore of
bytes and metadata. Working data, original inputs, untracked files, core sources
and Git registrations stay local. `WORKTREE-ARCHIVED.txt` records the immutable
manifest and recovery command. Use `restore-worktree` for exact file metadata in
a new directory; `git sparse-checkout disable` reconstructs committed source
bytes in the checkout. Neither method asserts fresh execution.
Use `--directory research` to leave all test fixtures physical when they have
current consumers or shared links. Read-only inspection compares exact index
blobs with the evidence filter disabled, so inspecting pre-migration raw reports
does not rewrite their identities or admit them as new reports.

`snapshots --source OLD --source OTHER --destination PRIMARY --destination REPLICA
--scratch RAM --retire` builds a new union on independent filesystems. Unknown
tables, conflicting identities/encodings, changed inputs and active consumers
refuse retirement. Both union stores verify every original snapshot hash. Real
snapshot restores and complete logical reconstruction of every distinct source
catalog precede retirement. Source schemas, parts, receipts and original encoded
chunks remain available through `restore-catalog`. Whole source-container page
layouts are explicitly retired; original snapshot and report hashes stay exact.
Update local snapshot settings to the new canonical paths. Old aliases retain
historical names; readers rejecting symlinks must use the canonical union path.
Defaults preserve 100/90 GiB. For the explicitly authorized SATA archive, pass
`--replica-minimum-free-gib 10`; this is its existing floor, not a main-drive
budget change. Private union construction does not lease the shared evidence
writer; only publication of verified custody roots acquires that lease.
After inspecting an interrupted attempt, `--resume` reuses only a recognized
owned idle union whose retained source identities still match. It rechecks full
snapshot hashes and real catalog restores before retirement; unrelated output
or changed sources refuse rather than being overwritten.
Both copies also compare their schemas and every original catalog record,
including encoded chunks and source membership maps. Valid raw snapshot hashes
alone do not establish that a historical source catalog can be reconstructed
from either copy. Each union's own interrupted verification-receipt prefixes
stay separate; original receipt bytes remain inside the preserved source records.

## Retain the proof and bound temporary output

For new work, retain exact source identities and unique changes, tested binaries,
claim-supporting raw inputs/captures, results and useful failure diagnostics.
Use semantic diagnostics by default and collect full captures only for a named
comparison. Keep new compiler objects/caches, resettable execution environments
and reproducible derived output temporary, with exact reconstruction inputs and
commands. Remove these only after reviewing consumers and pins. Unknown output,
unique negative inputs and binaries actually used for validation stay held.
Existing evidence is not expired by age or source drift. This policy changes no
engine contract, denominator, historical evidence hash or accepted status.
Place temporary build directories outside queued execution roots; retain the
tested executable and its build/source report with the proof. This prevents
ordinary completed-output handoff from permanently admitting compiler caches.

## Maintenance boundaries

Keep retention pins, saves, configuration, unique inputs, current fixture inputs,
recent data and active consumers. Review identities, dependencies and capacity
before removal; keep both durable receipts and restore proof. Retiring storage
does not refresh historical source fingerprints or establish engine equivalence.
The external archive is finite: consolidation is preferable to moving repeated
full copies between nearly full filesystems.

Transient game/prefix reuse remains a separate lifecycle policy described in
[working retention](working-retention.md). A historical private prefix is not a
shared writable template. Keep exclusive leases, clean reset and exact retained
state before reusing an environment.
