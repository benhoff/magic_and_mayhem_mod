# Snapshot verification throughput

The snapshot still audits registered evidence and freshly verifies every remaining
research report payload. Remaining SQLite-backed reports are verified by at most
four workers, each with its own read-only SQLite connections. Each worker decodes
one bounded chunk at a time and checks the original payload size and SHA-256.
Report bodies are discarded; only verified fingerprints return to the current
reader. Work is balanced by original payload size, with at most four worker tasks.

Aliases with the same SHA-256 and size share one verification within this batch.
Every path still passes the existing report-storage policy and containment checks.
File identity, size, modification and change times are rechecked after verification;
changes during or after the batch invalidate the result. Corrupt, missing, truncated
or incorrectly sized payloads still fail the snapshot. Ordinary files keep their
normal hashing path, and readers without memoization keep their existing behavior.

Workers finish and close their connections before snapshot processing continues.
The audit's already verified inputs are reused within that same reader. Working,
Git-base and staged checks each create a new reader and verify payloads again.
No persistent validation result, skipped gate or evidence-hash refresh is introduced.

The parallel-agent workflow now shares four execution slots across the whole
repository, rather than allowing four readers per concurrent check. Sequential
snapshot audit work holds one slot; prefetch borrows available extras. Linked
worktrees and temporary Git materializations share the same kernel-lock budget.
Independent-process controls cover six competing jobs, nested borrowing and
owner termination. This bounds contention; it does not promise a new speed ratio
for every concurrent workload. See [parallel integration](integration-workflow.md).

## Retained real-corpus comparison

On this machine, one snapshot of Git tree
`a68af15c2040219bd16ef3ed09ece40db22e2786` took **205.52 seconds** before this
change and **106.31 seconds** afterward, about **1.93x faster**. The tree has
13,266 research JSON pointers covering about 31.6 GiB of logical report bytes.
CPU sampling attributed about three quarters of the original snapshot's samples
to SQLite payload loading/decompression and integrity verification.

The baseline ran before the implementation changed. The new implementation then
audited an exact Git materialization of that retained tree, without executing its
historical code. SHA-256 hashes match for every complete snapshot section and the
full audit output. This includes all 4,937 stale evidence records and 2,489
validation gaps; the timing gain grants no engine-validation credit. These are
single-run measurements on a warm shared machine, not a whole-merge time guarantee.

`SNAP01.controls` retains the profiles, exact operator sources, regression output,
source fingerprints and comparison in SQLite. An initial materialization failed
at the system `/tmp` inode limit; its diagnostics remain separate. The successful
comparison uses owned host-visible RAM scratch. Check both byte and inode capacity
before running a full materialization.
