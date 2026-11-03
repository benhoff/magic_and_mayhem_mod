# Evidence storage lifecycle

For lightweight campaign diagnostics, shared input provisioning and automatic
comparison-output handoff, see [campaign check storage](check-storage.md).

The storage unit for new local work is an immutable artifact or typed SQLite
record, rather than a complete database file or execution directory. Exactly two
verified copies of each new unique object live on independent filesystems. All
worktrees use the same pair. Git continues to contain small references with the
original payload SHA-256 and length; storage migration changes no engine claim.
Existing historical stores are adopted explicitly, never discarded by a generic
cleanup sweep.

| Change | Implementation | Completion condition |
| --- | --- | --- |
| Shared evidence across worktrees | `coverage_evidence`, `evidence_objects` | The same original-byte hash resolves through the configured shared primary; replica verification precedes publication of a new reference. |
| Two durable copies total | `Pair`, explicit legacy handoff | Both independent stores verify every admitted object. Old source/container copies retire only after a source-specific preserved catalog and an actual restore. |
| Incremental backups | `evidence_local.retained_backup` | Finish replicates missing immutable objects and checkpoints review schema/typed rows; unchanged records add no payload blocks. Whole database images remain an explicit interchange/private-backup option. |
| Shared inputs and bounded execution | Existing two-slot `experiment_environment` pool, prefix borrowing, completed execution queue | A stopped prefix is exclusively leased, changes retained and state reset. Completed private game/capture trees share immutable file blocks after verified handoff. |
| Disposable derived caches | `evidence_cache`, `finish-work.py` | All review rows, pending drafts and auxiliary schema are checkpointed and restored successfully before eviction. Lazy reopen restores them. The gate still checks the actual current/staged tree. |

## Configuration and normal use

Choose a primary and replica on distinct mounted filesystems, under retained
`working/` paths. These Git settings are local machine configuration, not tracked
paths:

```sh
git config mm.evidencePrimary /cold/project/working/primary.sqlite3
git config mm.evidenceReplica /main/project/working/replica.sqlite3
# Optional override; normally tools/evidence-storage.json in the caller checkout.
git config mm.evidenceStoragePolicy /checkout/tools/evidence-storage.json
python3 tools/evidence-objects.py init
python3 tools/evidence-objects.py verify
```

The authored policy preserves the current 100 GiB primary and 90 GiB replica free
space floors. SATA remains a retained historical tier with its existing 10 GiB
buffer. This change does not lower any floor. Use the main checkout's storage
producer guard before a large import, test or restore. The maximum automatic
verification restore is 16 GiB of declared scratch output. The execution pool
already limits each source/family to two slots and refuses occupied slots.

New report producers use `coverage_evidence.write_report` as before. It writes,
verifies and fsyncs both stores before publishing a reference. SQLite uses
append-only object and block tables. Grouped writes commit both replicas before
publication or retirement. WAL permits existing report readers to continue
during a large import; free-space checks reserve WAL frames and their eventual
database checkpoint. Preserve database sidecars while a store is open. Raw 1 MiB block hashes deduplicate repeated
content across reports, input files, captures and review records; blocks use
zlib compression. The compatibility `chunks` view allows legacy read-only
consumers to resolve original report bytes. Historical source-specific auxiliary
catalogs are restored explicitly, rather than falsely exposed as the shared
store's auxiliary tables.

`tools/finish-work.py` performs queued completed-workspace handoffs, exact review,
local evidence checks, incremental retained backups, staging and the normal
commit hook. After successful finish it checkpoints and evicts the standard
review cache; `--keep-review-cache` keeps it materialized. An explicitly selected
alternate review DB is retained. No transfer, publication or engine rerun is
attached to the hook. Publication and private SSH backup remain separate commands.

## Execution and review-cache restoration

The acceptance pilot borrows a stopped admitted prefix for the entire child
lifetime. Registry changes, failures and prefix identity are preserved by the
existing pool. A stopped completed pilot queues its capture and private game
paths together with the exact acquisition report and immutable source bytes.
A held or failed prefix return stays in its execution tree for review.
Launching never archives or deletes the execution tree. Batch finish processes
only queues owned by that exact checkout.

An explicit handoff records **every** member, original `.retain` contents, file
bytes, links, mode, owner/group, modification timestamp and xattrs. It refuses
source checkouts, special files, multiply-linked files, changed sources and active
host/Wine consumers. A full restore from the replica must pass the exact member
comparison before the execution tree can become a small pinned restore stub.
This is a reviewed transfer of the pins and evidence, not permission for general
cleanup to ignore them. Unique fixture inputs and source checkouts remain held.

```sh
python3 tools/evidence-workspace.py seal /checkout/working/completed-run --retire
python3 tools/working-retention.py restore /checkout/working/completed-run
# Direct alternative:
python3 tools/evidence-workspace.py restore /checkout/working/completed-run
```

Restore before running a historical consumer that requires physical paths. The
restore itself needs disk headroom. RAM verification scratch is temporary; both
durable object stores and their pins remain required. Ordinary reads of Git
report references need no workspace expansion. The review-cache marker is
restored transparently by `coverage_bookkeeping.Workspace`; pending drafts are
preserved along with accepted receipts. Cached audits never authorize a later
source or index state.

## Adopting a completed legacy batch

Adoption is separate from new report recording. Import creates a source-specific
manifest containing original object identities, Git bindings, metadata,
recognized auxiliary tables and original schema. An actual restored V1 database
must match the logical catalog and schema. Unknown tables/schema refuse rather
than being silently dropped. Original payload hashes remain exact; SQLite page
layout is not a scientific result or a new promise of byte-exact container
restoration.

```sh
python3 tools/evidence-objects.py import --source /batch/working/evidence.sqlite3 --retire
python3 tools/evidence-objects.py checkpoint --source /batch/working/coverage/bookkeeping.sqlite3
```

Retiring the old primary preserves its historical pathname as a read-only
compatibility alias. Store-specific auxiliaries restore through `restore-source`
with the returned manifest SHA and size. No old hash is refreshed. Explicit
legacy snapshot retirement additionally opens and verifies **both** old catalogs,
restores each old snapshot and compares every source record against the supplied
new evidence/cache roots before replacing either catalog. Original catalog
schema, receipt bytes, object mappings and snapshot/encoding hashes are retained
in mirrored immutable roots. A small SQLite tombstone names the replacements and
states that the old physical page images have retired. This operation supports
only the reviewed two-image evidence/review catalogs; unknown or unequal records
refuse removal. A retry recognizes a completed catalog without changing its
history.

Use `evidence-objects.py restore-retired --catalog ... --snapshot OLD_SHA --output
NEW_FILE` for an explicitly **logical** reconstruction. It does not claim the old
container SHA. Direct old snapshot APIs refuse the tombstone with that command
hint, rather than returning different bytes under an old hash. Containers that
are themselves experimental inputs, unknown unique records, source snapshots and
still-active writers are outside this migration.

## Verification and limits

`tests/test-evidence-objects.py` exercises independent fixture filesystems,
interrupted replication, corrupt replicas, concurrent readers during a large
transaction, held-prefix handoff refusal, legacy/read compatibility, typed WAL
records including NUL/BLOB/rowid values, schema/trigger preservation, equal and
unequal snapshot handoffs, real workspace restoration, active consumers, source
mutation, scratch limits and pending-draft cache recovery. Prefix and launcher
regressions exercise state reset and ownership. Run `evidence-objects.py verify`
for a full current integrity/hash scrub; `sync` transfers missing objects only
and does not claim a full scrub of existing objects.

The actual rendering-watchdog batch migration is recorded under
`research/runtime/evidence-object-results-20261101/`. A completed storage handoff
is storage provenance. The first-World watchdog and other rendering acceptance
requirements remain open. Filesystem independence is protection against a
single local filesystem failure, not an off-host disaster-recovery guarantee.
Private remote backups remain explicitly requested operational work.

## Measured adoption result

[The reviewed closeout](../research/runtime/evidence-object-results-20261101/joined-01.json)
records 205 passing controls in 13 suites and a full verification of 10,155
objects on each independent NVMe. The completed watchdog batch's evidence DB,
two old snapshot catalogs and two execution trees occupied 15.75 GiB. The
complete new shared pair occupied 3.67 GiB, including this task's additional
controls and preservation metadata: **12.08 GiB less allocated storage (76.7%)**.
The reduction is measured across both filesystems, chiefly on the second NVMe;
it is not a claim that main-drive free space increased by that amount. Future
review records and unique captures still consume space. Other historical batches
retain their existing stores pending explicit reviewed adoption.

All 961 original report payloads and the original legacy reader's joined report
retain their exact hashes. Both retired execution trees passed full file-level
restores, including original pins: 13,267 pilot files and 34,498 experiment files.
The prior negative controls and exact changed source phase are retained separately.
The original source-checkout copy was retained because automatic approval review
rejected its deletion; the active work continued on a verified second-NVMe copy.

For this migrated batch, use the **new** tool with the historical checkout root:

```sh
python3 /var/local/magic-and-mayhem-archive/active-worktrees/evidence-object-store-20261101/tools/evidence-workspace.py \
  --root /var/local/magic-and-mayhem-archive/active-worktrees/rendering-watchdog-20261101 \
  restore /var/local/magic-and-mayhem-archive/active-worktrees/rendering-watchdog-20261101/working/pilot
```

Substitute `working/experiments` for the other execution tree. Run the normal
storage guard with enough reserved output before a physical restore. Ordinary
SQLite report reads require no execution-tree restore. The current machine
settings and preserved legacy fallback paths are recorded in
[machine setup](../research/runtime/evidence-object-results-20261101/machine-setup-01.json).
The checked-in source changes live on `codex/evidence-object-store-20261101`;
the main checkout's pre-existing edits remain separate.

## Existing file-level archives during the wider rollout

Older runs can contain files already held by the previous per-file archive.
`working-retention.py restore PATH` expands a sealed parent, then continues the
original file-level restore if the requested descendant is still absent. It
validates the working namespace before expanding anything. Select a historical
checkout explicitly with `--root`; preserve its original catalog and select the
original storage-tier policy with `--policy` when required. For example:

```sh
python3 tools/working-retention.py --root /historical/checkout \
  --policy /preserved/original-policy.json restore working/tests/group/run-id/input.bin
```

The 2026-11-01 sweep inventories all 206 present checkout working directories and
1,009 physical SQLite files. Its immutable inventory and run plan are retained in
the shared SQLite catalog under `project_working_inventory` and
`working_run_handoff_plan`. The plan separates 5,873 named runs: 2,714 need detailed
handoff review; 3,159 are held by role/size/newest-run rules. It also holds 1,796
unclassified groups for role review. These are review classifications, not
reclaimed-space claims. Active/canonical Wine prefixes, held leases, current
source-literal inputs, recent regular files/directories, source checkouts and
unsupported members stay held. Every actual handoff still requires a complete
file manifest, two verified stores, a real restore and immediate source checks.

The legacy catalog expands to 5.75 GB, so the sweep caches a checksum-bound
per-run membership flag rather than copying millions of file records into a
second cache. Failed preflight/index attempts are retained as negative custody
evidence; they removed no run data. Source checkout aliases and prior archive
backend aliases remain protected. No engine claim is renewed by this rollout.

## Sweep throughput and verification

Shared-v1 admission hashes each raw chunk before requesting compression. Blocks
present in both stores are reused without encoding. If one replica is missing a
block, its verified existing compressed bytes supply the repair. New blocks are
encoded once at the existing zlib level six and the identical encoding feeds
both stores. Every completed object is still decoded and hash-verified in each
replica before a reference or retirement can be published.

The compression pool defaults to four workers, with at most eight pending 1 MiB
chunks. Small files are grouped within that same bound so encoding can overlap
across files as well as chunks. Each original file identity is checked after
reading and again after admission; all later full source/restore checks remain.
`compression_workers` accepts integers from one to eight. Workers receive
only bytes and perform compression; all SQLite reads, writes, transaction commits
and publication run on the caller thread. Small reports avoid thread startup.
Prepared encodings spool after 8 MiB and are released before workspace restoration;
producer guards and declared scratch capacity remain required. Store schemas,
source payload hashes, compression level and full real-restore checks are unchanged.

`evidence_sweep_schedule.order_runs` orders reviewed candidates by main-device
priority and descending allocated size. The maintenance coordinator still checks
activity, current inputs, leases, newest runs, prior archive paths, source identity
and pins before each handoff. A larger run gains priority, never eligibility.
Stop/resume belongs at a completed-case checkpoint with mirrored receipts; do not
launch competing sweep processes against the same queue and writer lock.

The reviewed fresh-store benchmark is retained in
`research/runtime/sweep-throughput-results-20261101/benchmark-02-reviewed.json`.
The same roughly 500 MiB input took 50.5 s with the old writer, 27.0 s with
one optimized compressor and 18.7 s with four bounded compressors. All variants
used empty independent stores, retained identical object identities and compressed
block sizes, and performed a full real restore. This measured workload is about
2.7 times faster; it does not predict the whole sweep. The source-stable 15-suite
control report is `throughput-controls-02.json` in the same results directory.
