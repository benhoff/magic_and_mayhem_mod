# Whole-brain teardown — WBT1, 2026-10-23

`OwnerBrainTeardown` implements the normal `415ac0` payload release and whole
`4c0920` final destruction contracts over checked PE32 value-address storage.
It composes the existing source, base task and observation-snapshot destructors.
The enclosing brain and actor pool remain borrowed. This is bounded offline
reconstruction and headless integration; native session installation is separate.

The pinned NoCD executable is SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence is high within the declared normal domain. Inspection4 preserves
existing inventory instruction ranges, read-only Ghidra decompilations and caller
references. Missing decompilations retain central instruction ranges; no function
boundary or unknown indirect target is inferred.

## Reset and final destruction are different contracts

Earlier ownership notes used `4156b0` as shorthand for the outstanding brain
destructor. Its actual role is state reset: it clears logical collection lengths
and observations, creates a fresh default evaluator, invokes registered factory
slots and resets counters/flags. It does not release the old task/source payloads.
The `416b80` reset wrapper calls `415ac0`, then `4156b0`, then re-registers live
actors. Construction through `4c02a0` also calls `4156b0`.

Final destruction through `4c0920` calls `415ac0` and then destroys the member
containers; it does not repopulate them. Reset, original construction, registry
factory execution and actor re-registration are inspected context only in WBT1.
Neither `releasePayloads` nor `destroy` is an idempotent disposal API. Calling
`releasePayloads` and then `destroy` without the intervening original reset would
release the same payloads twice. The recovered payload phase deliberately leaves
its collection slots and lengths intact.

## Confirmed normal release order

`415ac0` releases, in order:

1. Base task payloads in the brain+4 evaluator vector, in forward slot order.
2. Source payloads in the brain+1a primary vector, in forward slot order.
3. Source payloads in the brain+11e05 secondary vector, skipping pointer identities
   found in the primary vector. Comparison reads retained pointer slots, without
   dereferencing the already freed primary objects. This is the specific recovered
   overlap rule; there is no general deduplication of arbitrary owning collections.
4. This brain's assignment word at actor+678+4*brain-ID for all125 pool slots,
   through actual `514810`, with a zero assignment value.
5. Task payloads in the brain+ba map, in ascending iterator order. `42dd00`
   advances the cursor; it does not erase nodes. The map retains dangling task
   values until its later scalar container destruction.
6. Both source-cache banks for owner0 through owner7, bank0 then bank1, each
   in forward creation-vector order.

`4c0920` then destroys the12088 scalar map; inline sources11f33 and11ea6;
11e15 map and11e05 backing;11df5 map and11de5 backing;11dc3 borrowed task-alias
map and11db3 backing. It destroys the eight owner rows **in reverse order**,
owner7 down to owner0. Within a row it frees bank1 backing, then bank0 backing,
then the observation tree and its head. Finally it destroys the ba task-map
container, deques8a/5a/2a, primary source-vector backing and evaluator-vector
backing. The borrowed alias map never takes ownership of its task values.

Whole-tree erasure visits right subtrees, destroys the current node, then walks
left. Observation nodes also destroy their embedded snapshot at+24. The root
path uses `42fd70`/`419fb0`; the receiver adjustments total+24. Empty heads and
shared nil sentinels are separate allocations. Each container decrements its own
shared reference counter; the final user frees and clears the nil pointer.
Observation rows use global5faf4c with counter6e204c, not the adjacent pointer
word5faf50, which belongs to another tree family.

Deque destruction removes nonowning DWORD entries and releases4096-byte blocks
as the front crosses block ends. The final pop frees the block map. `42f950`
zeros iterator words+4 through+20, but retains the now-freed map pointer/capacity
at+24/+28. WBT1 preserves these final bytes instead of adding a native repair.

## Execution and reproducibility

The final2 comparison prospectively binds its behavior/scenario and every compiler
input. Independently authored identical fixtures run native64, mapped original32
and ASan+UBSan+LSan. Actual whole `4c0920` executes without engine-service
replacement; only allocation/free/buffer-free boundaries are redirected. The
original text is sealed RX and checked unchanged. Original manifests pass before
and after every original-consuming run. No original file is patched.

The matrix has16 episodes,32 brain destructions and256 owner rows. Two brains
share the original tree-family nil lifetimes. It includes empty/singleton/three-node
balanced observation and scalar maps; populated task/source forests; null vector
slots; primary/secondary source overlap; pending task values and borrowed task
aliases; both cache banks; inline source backing; and0/1/1024/1025-entry deques.
Snapshots retain borrowed actor identities and selected owned histogram buffers.
The owner0/1 admission refusal is tested independently as native policy before
any release, including in the original fixture binary.

All three executions agree on3,572,476 checks,5,606 releases,84 snapshot-buffer
releases and digest10878094686036488357. Complete final brain bytes and the exact
allocation release sequence participate in the independent comparison. The first
brain's destruction preserves the other brain's complete frame and assignment
word. Shared reference counters, final pointer/length zeros, allocation extents,
poisoned freed blocks, suffix canaries and zero surviving owned allocations are
checked. Original output never seeds native state.

Trial1 preserves a compilation warning and system `/tmp` quota failure. Trial2
passes the complete matrix. Final1 reruns unchanged engine/fixture code after
correcting ancillary original hash extents to the exact central inventory ranges.
Every attempt and its prospective source hashes remain immutable in SQLite.
Final2 additionally gives every authored tree its normal black root, red children
and black nil sentinel. It repeats the same successful totals and digest with
fresh source bindings. Separate final2 evidence IDs support the current fixture;
final1 and earlier results retain their historical source fingerprints.

The incomingb45abf10..9b83b3d7 accounting review checks537 exact committed
file transitions. It appends retrospective storage receipts for44 raw-bin report
additions that were outside the source census, preserving the initial negative
review. No unresolved receipt gaps remain in that reviewed range. This is
accounting, not an assertion that a historical gate passed or a new engine run.

The Critical dashboard controls pass9 cases and retain one pre-existing aggregate
count failure: incoming HEAD and this branch both have4 partial Critical rows
(2 recovery,1 validation,1 acceptance), while that test expects6. The retained
UI result compares both definitions without changing engine status. That unrelated
test expectation is not repaired as part of this teardown batch.

Run a new comparison from a checkout containing this change with:

```sh
python3 tools/working-retention.py run --reserve-gib 0.15 -- \
  python3 tools/test-whole-brain-teardown.py \
  --output working/tests/brain-teardown-new-run
```

## Remaining boundaries

The trees are independently authored balanced fixture inputs. WBT1 supplies
destruction over that build-specific representation; it does not implement a
portable red/black insertion, rebalancing or iterator ABI. Live resource-bearing
callbacks and their exceptional deleting flags remain covered only by their
separate existing evidence. Arbitrary ownership duplicates, cyclic/malformed
trees, unusual allocator states, other task-family derived state, failure/SEH,
reset/re-registration, training/backends, full World/session cadence and installed
or live replacement remain open. The Critical actor-services umbrella stays
partial, and gameplay balance is unchanged.

Durable declarations, successful/failed results, inspection and accounting reports
use SQLite evidence pointers. Compiler temporaries and retained execution binaries
remain ignored under this owned worktree's `working/`. The startup storage review
found no automatic cleanup candidates; no payload was deleted to complete WBT1.


The first closeout wrapper incorrectly used the storage launcher's main-checkout
working directory as its root. It exported one pending bookkeeping receipt for
unrelated main-checkout changes, then failed at backup restore setup before
staging or committing. `WBT1.misrouted-closeout-correction-20261023` explicitly
corrects that receipt's inapplicable rationale without granting source/contract
or validation authority. The failure and exact receipt are preserved in
`closeout-misroute-1.json`. The corrected wrapper anchors the root to its own
file, asserts the owned worktree path and passes that root explicitly to
`finish-work.py`. No unrelated engine source was changed or staged by this
closeout attempt.
