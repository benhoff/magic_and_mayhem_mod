# Borrowed alias target bodies and cursor consumers, 2026-10-23

This extends the [earlier alias boundary](borrowed-alias-consumers-20261023.md)
for pinned NoCD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Static inspection is retained in SQLite as
`alias-targets-results-20261023/inspection-1.json`.

The base-reset/deleting-wrapper scan identifies 46 callback classes: the prior
13 adjusted 16-byte classes and 33 compact 12-byte classes. Compact classes
read target4 and receiver8 and forward one argument without receiver adjustment.
Both families propagate the return, reset the base vtable to5c5c40 before free,
and use only deleting flag bit0. Arguments are cached before dispatch so a
callback may delete itself. This is exhaustive for the reviewed wrapper and
base-reset signature, not for every engine callback encoding or target body.

Actual actor callback51e890 is now implemented through its return at51ec59.
The data at51ec5c/51ec6c is a four-entry jump table and ten-byte selector map,
not additional instructions. Events5/14 conditionally dispatch message7/12;
event8 clears8cd and calls51f5e0/506d30; event9 performs the recovered location,
rate, path, activity, descriptor, engagement count and message10 branches.
An inactive actor on successful placement returns before restoring104.
World/action/presentation callees remain explicit ordered services.

524130 owns the recovered order-tick control: admission gates, marker44,
repeat flag901, cursor8fd advancement, marker disposal, list unlink/free,
count902, type3 cleanup and normalized20-byte order submission. Snapshot copying
418d50 is already covered by OSR1: it retains8fd while construction strips8f9.
There is no original stale-cursor or missing-owned-list guard.

The other direct cursor references have explicit projections:

- 523e0f..523e34 increments902, replaces8fd with list head-next, then calls512220.
- 526977..5269a5 serializes each live list payload, compares cursor identity and
  records its ordinal without dereferencing the cursor. If no node matches,
  it retains the caller's scalar. In the full save body this stack slot last
  receives actor7d1's resource ID (or minus1) at5263fb; it is not initialized0.
- 5292c6..5292f0 reseeds8fd, then follows only signed-positive index steps.
  Out-of-range positive indices cycle around a well-formed circular list.

The projections use original instructions with explicit register/stack
preconditions and return bridges outside the compared intervals. They do not
claim complete actor queue append, actor serialization, file I/O or deserialization.
Confidence is high within the selected instruction and fixture domains.

Checked native storage refusal is a host policy. Original nonempty-list stale
cursor and stripped-snapshot missing-list accesses fault in isolated children;
empty-list admission skips the inaccessible cursor. Duplicate owning callback
aliases similarly fault on the second release; no ownership repair or rollback
is inferred. Callback allocation failure publishes null708/cf3 and still creates
an owned list; list/sentinel allocation failure retains completed allocations
before faulting. Native throwing targets and throwing free services test retained
ownership and absence of post-dispatch callback reads, without SEH equivalence.

Trial reports preserve compilation failures, a fixture self-deletion bookkeeping
failure, an allocator-observation mismatch, and an original projection stack-offset
failure. Trial4 and trial6 are scoped successful development comparisons. Final
source-bound execution and session regression are recorded separately below.

Other actual callback targets, nonmatching callback encodings, arbitrary provider
mutation, Windows SEH/unwinding, complete parent serializer/append bodies, World
cadence and installed/live replacement remain pending. The earlier BA1 results
and all historical hashes are preserved; shared dispatcher changes require new
execution before asserting current-code compatibility.

The final prospectively source-bound `alias-targets-final-1.json` compares 20,593
checks, 198 compact callback invocations, 3,584 actor callback event cases,
128 order ticks and save/load/reseed cursor projections. Native64, original32
and ASan/UBSan/LSan match digest `13058600155535184850`. All tracked allocations
retire after fixture disposal, canaries survive, target text remains RX and
unchanged, and original manifests pass before/after. Exact final implementation
and test source versions are retained in the SQLite prospective-input record.
Development reports retain their original fingerprints, including negative runs.
The new51e890 and compact-thunk range exceptions retain all inventory unknowns.

`alias-targets-session-1.json` completes the full existing 64 native,64 sanitized
and64 original actor-session variants:1,024 checkpoints,393,604,952 normalized
bytes and comparison SHA256
`57e0179403ac9697ac96809693aeaf6f07cca84cbef0820b511004fcee193630`.
This validates changed-dispatcher compatibility for the existing session scope;
it does not claim that a full session now exercises all new event targets.
The initial audit used an incomplete cache override and retained its archive
hash failure. Subsequent local checks use the repository-configured verified
cache; no historical archive or evidence identity is refreshed.

For a new comparison, choose a previously unused output below `working/tests`
and run `tools/test-alias-targets.py --output <absolute-output-path>` through
the main checkout's `tools/working-retention.py run --reserve-gib 0.2 --` guard.
Pass the absolute script path when running from a worktree: the guard launches
its child in the main checkout. The runner binds prospective sources before
compiling native64,original32 and sanitizer binaries; it imports the bounded
runner report into SQLite even when an attempt fails. Original manifests run
before/after. The final input declaration also preserves exact changed source
text. Raw compiler outputs/binaries are retained under the selected output;
`claims-<run>.json` is a bounded interchange consumed by the comparison runner.

The incoming committed range8d3d2b51..4ccfd582 was reviewed separately from this
uncommitted batch. All56 first-parent file transitions match exact parent/current
receipt hashes, with journal/register exports identified separately and no
unresolved gaps. The SQLite `incoming-history-1.json` report retains the review;
it does not assert a past gate pass or fresh engine validation.

A refreshed generated [coverage snapshot](coverage/summary-alias-targets-20261023.md)
is retained through the SQLite writer. Source-census review and exact change
receipts are bookkeeping, separate from the execution evidence above. Generated
reports use SQLite compatibility references; all retained failures remain
available. Original inputs were never changed.

The main-drive guard stopped the first local closeout at89.90GiB free. Its
accepted receipts and partial backup attempt remain retained. All17,757 task
file identities were copied and verified on the second drive, including exact
bytes,modes and symlink targets; allocated-block differences were recorded
separately. Git registration now names
`/home/hoff/Videos/mnm-alias-consumer-targets-20261023/worktree`; the original
main-checkout task path is a compatibility symlink. Only the redundant old
physical copy was released after verification, with relocation receipts on both
drives. All source/evidence/pins remain intact. `relocation-1.json` records this
storage provenance; relocation asserts no new engine validation. Subsequent
closeout uses task-owned temporary storage on the second drive.
