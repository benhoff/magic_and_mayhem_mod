# Faster local integration

Prepare each change in its own worktree. Acquire admission **before** preparing
a merge against master, and keep it through combined review, backups, the normal
staged hook and final promotion. The lock is cooperative: other operators must
use it too. Preview without creating a lock or changing Git:

```bash
python3 tools/with-integration-lock.py
python3 tools/with-integration-lock.py --execute --expected-master <reviewed-commit> -- <integration-command> <arguments>
```

Admission waits, then refuses if master changed. It uses a separate Git common
directory lock, leaving Git's index lock available. `finish-work.py` on master
acquires it automatically; feature branch finishes stay independent. Wrapping
only finish cannot protect earlier merge preparation. Python operators spanning
several commands should use `integration_queue.integration_lock` and
`lease.run(...)` for children. The inherited descriptor must survive every child
hop (`pass_fds`); an environment variable alone is refused. Never remove a busy
lock file.

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

## Avoid repeated work during finish

`finish-work.py --timings ...` reports phase durations. Shared-store backups now
compare object/version catalogs in bulk SQL, transfer missing objects and freshly
verify root payloads. This is incremental replication, not a scrub of all existing
objects; explicit pair verification supplies that check. Divergence is refused.

A regular SQLite source without write bits or WAL/SHM/journal sidecars can reuse
a retained successful logical restore. Each reuse freshly hashes the entire
source and verifies the complete referenced object closure in both stores. The
proof binds source checksum/size and checkpoint codec bytes. Changed sources or
codecs take the full checkpoint-and-restore path; mutable/WAL sources always do.
Missing/corrupt proof or referenced objects refuse reuse. `checkpoint_reused` and
`real_restore_performed` distinguish retained restoration from current checking.
Logical reconstruction never claims the old physical SQLite container bytes.

Audit and snapshot share a bounded verified-byte cache within one invocation.
File identity, size, modification and change times invalidate edited inputs; JSON
values are parsed afresh. Working, Git-base and staged snapshots each open a fresh
reader. There is no persistent pass cache and the normal staged hook still runs.

Run the bounded benchmark using owned RAM scratch and the storage guard:

```bash
TMPDIR=<owned-ram-scratch> python3 tools/working-retention.py run --reserve-gib 0.1 -- python3 -B tests/test-finish-performance.py --benchmark
```

Its small two-filesystem fixture reports backup times, catalog query counts and
repeated evidence resolutions; it does not predict a full merge time. Persist
citable results through `coverage_evidence.write_report`.
