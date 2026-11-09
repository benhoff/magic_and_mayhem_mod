# Faster local integration

Prepare each change in its own worktree. Acquire admission **before** preparing
a merge against master, and keep it through combined review, backups, the normal
staged hook and final promotion. The installed reference-transaction hook requires
admission for every local master update, including fast-forwards and plumbing
updates. Install current hooks from a durable checkout with
`python3 tools/install-coverage-hook.py`; its absolute hook path applies the same
current guard to older linked worktrees. Preview without changing Git:

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

## Four to six parallel agents

Keep development, builds and focused tests in separate owned task worktrees.
Acquire integration admission only when ready to combine with master. The
following command waits, reads master after admission, creates a new integration
branch in the clean owned worktree, merges pinned ready tips, reviews history,
refreshes the census, runs normal finish/backups/staged hooks and promotes the
checked commit. It preserves the prior task branch and refuses dirty source or
master checkouts. Preview is the default:

```bash
python3 tools/integrate-work.py --tip <ready-feature>
python3 tools/integrate-work.py --tip <ready-feature> --execute \
  --branch integrate/<unique-batch> --db working/coverage/<batch>.sqlite3 \
  --report research/runtime/<batch>/preparation.json \
  --reason '<reviewed combined change>' \
  --validation-reason '<current validation and retained limitations>' \
  --index-review-reason '<reviewed source and behavior ownership>' \
  --message '<combined change>'
```

Repeat `--tip` to batch compatible ready branches into one combined review and
staged gate. The batch strategy combines disjoint central register/journal additions
and append-only ledger tails while retaining every keyed record, using candidate
Git trees and index entries without intermediate commits. Standard Git text merges
handle other paths. A single-tip merge uses the metadata resolver below. Changes
to the same metadata record, deletions and unresolved code conflicts require
review or separate integrations. The command does not invent semantic resolutions,
reset sources or bypass a failed finish. Failed preparations retain an immutable
report and the resulting worktree state. Previously unreceipted committed versions
require a concrete `--history-reason` and every affected `--behavior`; this records
custody only. Branch names and report paths must be new.

Custom operators still use the wrapper/Lease API above. Children invoking Git
must inherit the lease descriptor (`lease.run`); a bare environment variable or an
unlocked descriptor cannot authorize promotion. The storage guard validates and
forwards this descriptor when launching a guarded child. The guard rejects bypasses while
preserving checked candidates and source changes. It is a local concurrency
guard, not an alternative to coverage review, backups or Git access permissions.

Snapshots share four verification slots across linked worktrees, including
temporary Git trees. Sequential audit work holds one slot; a parallel prefetch
borrows currently available extras. Six simultaneous checks do not each reserve
four readers. Slot locks release on process exit; no PID cleanup or retained pass
cache is needed. Ordinary coding/build jobs do not acquire these slots. The limit
controls verification concurrency, not total compiler or game-run concurrency.

Temporary Git copies check their estimated allocated bytes, inode headroom and
write access before extraction. They try the configured temporary directory and
then bounded `working/coverage/scratch`. These copies are temporary, contain no
original media and are removed after their consumer exits. Capacity can still
change under concurrent writers, so retain the outer storage guard and machine
floors. `--timings` prints scratch selection/preflight details; backup results now
split root verification, writer-lock wait and the writer section.

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

Existing canonical root payloads receive fresh two-copy verification through
read-only SQLite snapshots before taking the exclusive archive writer lock.
After admission, exact roots must still exist in both catalogs to use that
invocation's verification. New or missing roots take the locked verification and
replication path. Immutable object/metadata triggers and the shared writer lease
remain required; this does not reuse a previous invocation's integrity result.
The shorter lock permits other agents to append evidence while those reads run.

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
Remaining research report payloads now receive bounded parallel verification with
at most four independent readers. See [snapshot throughput](snapshot-performance.md)
for integrity boundaries and the retained full-corpus comparison.

Run the bounded benchmark using owned RAM scratch and the storage guard:

```bash
TMPDIR=<owned-ram-scratch> python3 tools/working-retention.py run --reserve-gib 0.1 -- python3 -B tests/test-finish-performance.py --benchmark
```

Its small two-filesystem fixture reports backup times, catalog query counts and
repeated evidence resolutions; it does not predict a full merge time. Persist
citable results through `coverage_evidence.write_report`.
