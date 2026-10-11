# Working storage retention

The repository policy targets **1280 GiB (1.25 TiB)** of allocated storage under `working/`
and at least **100 GiB** of filesystem free space. Game installations, toolchains,
manifests, runtime staging, logs and retention reports are exemptions from cleanup
selection, but still count toward the overall budget. Registered Git worktrees
are retained whole, including their ignored outputs. The target was increased
to defer cleanup; automatic archiving is disabled.

Create a new dry-run inventory:

```bash
python3 tools/working-retention.py inventory --output working/retention/inventory-20261011.json
```

The inventory names nonoverlapping review units, allocated bytes, references in
tracked research/fixture metadata, worktree ownership and retention reasons. It
retains the newest two runs per group and immediate directory contents modified
within 24 hours. Put a `.retain` marker inside a run or its parent group to pin it.
Unclassified directories remain protected. Symlinks are not followed; `original/`
is never opened. Existing reports are never overwritten.

`review_for_cleanup` means a human must review the run; it is not proof that its
contents are disposable. Dynamic paths, references in other Git branches and
running processes are not fully observed. Recent timestamps are not a liveness
test. Preserve raw inputs, reference oracles, reports, manifests and unique source
changes until their dependencies and archive have been verified. Prefer removing
rebuildable isolated `game/`, `wineprefix/` and compiler outputs within reviewed
completed runs; preserve the recorded metadata and hashes. The inventory command
never deletes files. The separately authorized archive workflow can remove bulk
payloads after verification; it never removes worktrees.

Check the budget or guard a future experiment:

```bash
python3 tools/working-retention.py check
python3 tools/working-retention.py run --reserve-gib 10 -- python3 tools/test-creature-navigation.py
```

Both commands refuse with exit status 2 if measured usage plus the requested
reservation exceeds the budget, or the reservation would reduce free disk space
below the minimum. The reservation is a preflight estimate, not allocated space.
During `run`, filesystem free space is checked every two seconds. If it falls
below 100 GiB, the guard sends SIGTERM to the command's isolated process group,
allows up to five seconds for shutdown, then kills any remaining descendants and
returns status 2. Ctrl-C also stops that group. A free-space measurement failure
stops the command rather than leaving it running unmonitored.

This is an opt-in launch guard, not a filesystem quota. Existing experiment tools
and worktree copies require adopting the main checkout's wrapper. Already-running
jobs and independent writers are not controlled. Very fast concurrent writes can
consume space between checks; the reserve provides a buffer, not a guaranteed
global disk limit. The working-size target is checked before launch; runtime
monitoring checks free space without repeatedly scanning the whole tree.
No background pruning is enabled, and `auto_archive` is false. Guarded launches
refuse when there is insufficient space; manual archiving remains available for
later cleanup.

In the first cleanup pass, inventory and review before deleting or relocating
anything. Record the approved paths and exclusions, preserve retained artifacts
with their exact SHA-256 hashes, verify any archive, then measure disk usage again.
Retain the dry-run report under `working/retention/`. Keep storage accounting
separate from recovered-engine coverage and current validation claims.

## Verified archives on the second NVMe

Cold storage lives at `/var/local/magic-and-mayhem-archive`, on the second NVMe.
Create it once with owner `hoff:hoff` and mode `0700`. The sandbox requires an
execution escalation for tools writing outside the workspace; ordinary terminal
use does not require sudo after directory creation.

```bash
python3 tools/working-retention.py archive --prune
python3 tools/working-retention.py archive --prune --include-worktree-output
python3 tools/working-retention.py restore working/experiments/example/run-old/capture/input.bin
```

The first command selects completed runs in the main checkout; the second also
considers ignored run output inside registered worktrees. Neither changes tracked
worktree code or removes checkout directories. Selection preserves pins, the two
newest runs per group, nested source snapshots, symlinks and runs whose traversed
contents changed in the past 60 minutes. A best-effort `/proc` open-file/cwd check
also skips visible active runs; it cannot prove absence of all concurrent users.

Files below 64 KiB, source, reports and configuration stay in place. Larger binary
payloads are stored as deduplicated Zstandard objects addressed by original
SHA-256. Each object is decompressed and checked independently; the source is
rehash-checked before cleanup. Durable catalog receipts on both disks precede
unlinking. A changed source, failed compression, failed verification or failed
receipt leaves the source file present. Archive storage retains at least 100 GiB
free. Catalogs are append-only JSONL and preserve original paths, bytes, modes,
timestamps and compressed hashes. Interrupted operations can leave an extra
verified object or receipt, but do not authorize deleting an unverified source.
Compression uses the Python `zstandard` package when installed and otherwise
the `zstd` executable. Both produce independently decoded and hash-checked objects.

Archived payloads must be restored before a later experiment reads those original
working paths. Restoration verifies both compressed and decoded hashes and
refuses to overwrite changed local data. Keep the cold-storage catalog and objects
together. This relocates retained evidence rather than invalidating its original
hashes; it does not rerun engine validation. The archive is on a second drive in
the same machine, so an independent off-machine backup remains a separate task.
