# Storage consolidation

The [user-approved hybrid policy](evidence-storage.md#hybrid-retention-approved-2026-11-13)
supersedes blanket dual-copy and permanent preservation requirements below.
Essential evidence remains mirrored; verified historical bulk may have one copy;
reviewed reproducible caches/environments and completed source copies may retire
without another full archival copy. Keep reconstruction inputs, unique changes,
saves/current inputs, useful results and explicit retirement records. Pins and
old failures require classification rather than indefinite retention by default.

Generated evidence is shared by payload identity; execution state remains private.
The machine's main-drive 90 GiB and cold-drive 100 GiB reserves are unchanged.

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
An unconfigured checkout retains materialized backups. A failed compaction keeps
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

Completed clean merged worktrees may become sparse only after every removed
tracked file matches its Git blob, is cold-backed and is restored independently.
Registered Git metadata, branches, hooks, originals, untracked files and all
working/ outputs remain. WORKTREE-ARCHIVED.txt names the exact source snapshot.
Modified, pinned, shared, recent or active source checkouts remain intact.

## Explicit tracked-directory compaction and snapshot unions

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
