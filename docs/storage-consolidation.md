# Storage consolidation

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
