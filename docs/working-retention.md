# Working storage retention

The repository policy keeps at least **90 GiB of filesystem free space** on the
main checkout's drive. There is no fixed size limit on `working/`, including
nested worktree output. Before launching, the guard subtracts the estimated
output reservation from available disk space and requires the remainder to stay
at or above the reserve. For example, a 10 GiB reservation needs at least
100 GiB free. Available space includes consumption by other directories and jobs
on that filesystem.

`tools/working-retention.json` sets `minimum_free_gib` to 90 and
`max_working_gib` to `null`. A positive numeric maximum remains supported for
older or explicitly stricter custom policies; omitting it also selects free-space
only checks. Free-space-only `check` and `run` skip the working-size scan.
Inventory still reports allocated working bytes, with `over_budget_bytes: null`
when no size maximum applies and a separate `free_space_shortfall_bytes` field.

Game installations, toolchains, manifests, runtime staging, logs and retention
reports are exemptions from blanket cleanup selection. Registered Git source
checkouts are preserved; their ignored completed-run output can be reviewed
separately. Automatic archiving inside experiment launch remains disabled.

## Periodic maintenance

The user authorized periodic reviewed cleanup on 2026-10-20. Check available
space at session start, at least daily while work is active, and after each
build/experiment batch. Start a maintenance review below **150 GiB free**, or
after a completed batch produces at least **10 GiB**. Aim for **180 GiB free**
through eligible cleanup. These are maintenance thresholds; the main hard reserve
stays 90 GiB and the archive hard reserve stays 100 GiB. This is an agent workflow,
not an installed cron job or an unattended deletion service.

Use one new inventory per maintenance pass, retain its exact candidate plan,
and prioritize inactive copied game installations, Wine prefixes and historical
compiler outputs. Bulky captures may move to verified cold storage only when
their dependencies and retention rules permit it. Keep original references,
failures and regression inputs even when their historical source versions differ
from current code. Source drift changes execution freshness, not storage value.

Preserve all retention pins, unique sources, registered checkouts, reports,
configuration, saves, current tool/fixture inputs and the newest two runs in each
group. Files modified within 24 hours and directories active within the past hour
remain local. Inspect host-visible open files, working directories, executable
paths and Wine prefixes before acting. A manifest or old timestamps alone do not
prove a run has stopped. Recheck dependencies, pins and exact file identities
immediately before each removal. Never unlink a payload before complete archive
verification and durable receipts on both disks.

Routine maintenance within these reviewed bounds does not need renewed user
permission. Changing a reserve, removing a pin, deleting unique evidence or
removing source checkouts is outside that authorization. When eligible candidates
cannot reach the target, retain the measured shortfall and keep the exclusions.
Do not substitute blanket directory deletion or historical-hash refreshes.

Close each batch by distinguishing durable evidence from disposable environments;
prefer bounded captures and marked, idle, exclusively locked reusable fixture
prefixes. Prefer an exact Git revision plus preserved unique changes over another
full source copy when that retains the necessary provenance. Existing unique
snapshots remain protected. Keep a maintenance result with verified reclaimed
allocated bytes, before/after filesystem capacity, skipped candidates and a real
restore check. Independent writers can make free-space deltas differ from the
verified removals. Use the existing archive primitives or a retained, hash-bound
reviewed executor; do not treat an old plan as authority to remove changed files.

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

Check available space or guard a future experiment:

```bash
python3 tools/working-retention.py check
python3 tools/working-retention.py run --reserve-gib 10 -- python3 tools/test-creature-navigation.py
```

Both commands refuse with exit status 2 if the reservation would reduce free
disk space below the minimum. An explicitly configured numeric working-size
maximum is also checked for custom policies. The reservation is a preflight estimate, not allocated space.
During `run`, filesystem free space is checked every two seconds. If it falls
below 90 GiB, the guard sends SIGTERM to the command's isolated process group,
allows up to five seconds for shutdown, then kills any remaining descendants and
returns status 2. Ctrl-C also stops that group. A free-space measurement failure
stops the command rather than leaving it running unmonitored. Separately
authorized pruning stops once the configured free-space reserve and any custom
size maximum are satisfied; auto-archive preflight also preserves the launch
reservation.

This is an opt-in launch guard, not a filesystem quota. Existing experiment tools
and worktree copies require adopting the main checkout's wrapper. Already-running
jobs and independent writers are not controlled. Very fast concurrent writes can
consume space between checks; the reserve provides a buffer, not a guaranteed
global disk limit. Runtime monitoring checks free space without repeatedly
scanning the whole tree. Changing the minimum reserve requires an explicit policy
change; growth of `working/` alone does not require raising a size cap.
No background pruning is enabled, and `auto_archive` is false. Guarded launches
refuse when there is insufficient space; reviewed periodic maintenance is a
separate operation under the standing authorization above.

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

The inventory tolerates files removed by active captures during a `du` scan only when every diagnostic is a disappeared descendant and the final root total is present. Permission, I/O, missing-root and other measurement failures still refuse the launch. Measurements remain non-atomic; the existing reservation and live free-space monitor remain required. The native-render integration exposed this race; twelve guard tests include disappearing replies and fail-closed measurement cases.
