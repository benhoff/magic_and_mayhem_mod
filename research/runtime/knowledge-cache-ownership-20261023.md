# Knowledge source-cache ownership — OKC1, 2026-10-23

OwnerKnowledgeCache now owns the ordinary owner0 source-cache vectors used by
413330. It builds raw source records directly from borrowed typed observations,
retains cache roots across lookups, and invalidates the two banks independently
of task copies. OwnerTask6Composition.prepareCached clones that raw forest using
the recovered OwnerTaskSources API, then maps its ordered nodes into the existing
generated planner/feature/save/body/replacement/retirement composition.

## Original layout, findings and confidence

Pinned NoCD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
High confidence within the declared successful owner0/two-observation domain.

Each owner's48-byte knowledge row starts at brain+11c02+48*owner. Its source
cache vector headers are+10 and+20: begin/end/capacity at+14/+18/+1c and
+24/+28/+2c. Bank0 admits observations marked at the supplied tick; bank1
includes stale observations. A populated cache vector reuses its first source
as root even when that root has no children. It does not rebuild on a marker
change until invalidation. The vectors retain **creation** order (root,leaf,
wrapper,leaf,wrapper in the two-observation fixture), which differs from the
recursive task clone's preorder. Child-vector capacities retain append/merge
history, not just the final number of children.

4174f0 constructs an anonymous cache source and stores the bank argument at
byte25.417e80 stores the borrowed snapshot pointer at35 and its low byte at25;
activity89 is3 and key9 is initialized before the producer writes its memory
key. Anonymous root key9/region19/flag21/activity89 retain allocator provenance.
The checked fixture uses A5-filled allocations; arbitrary allocator contents
are outside this proof. Source35 remains borrowed. Cache grouping/merge writes
positions,region,uncertainty and movement flag;417e60 calls407650 and optionally
frees the source. Destruction does not free its borrowed snapshot.

4181d0 invalidates both source-cache banks for all eight owners **before** mode
dispatch. It destroys each bank's sources in forward creation order and calls
42f250, which resets end to begin while retaining backing capacity. This proof
executes the actual mode1 entry with owner0 populated and the other seven owner
rows empty. The native caller composes cache invalidation with the existing
markOwnerKnowledge projection. Mode2 tree/snapshot mutation and task/notification
consequences remain separate. A mere cache hit or planning sweep is not an
invalidation.4156b0 is inspected as context, but whole-brain reset/destruction
is not implemented or validated here.

Both cache and task records borrow the same snapshots; they own separate source
records and child/vector backing. Cache invalidation leaves active task clones
intact. Task body snapshot-to-live rebinding changes the clone while the cache
continues borrowing its original snapshot. Cache source nodes retire independently
of common40a340 evaluator cleanup and418050/428b40 task retirement. Suffix feature
backing retention remains the original baseline; later fixture disposal is an
explicit host policy, not a recovered brain destructor or leak fix.

## Execution evidence

`knowledge-cache-final-1.json` prospectively binds76 inputs. Native64, original32
and ASan+UBSan+LSan execute128 generated task6 episodes with both cache banks,
768 cache builds,640 cache hits and384 invalidations. All8,576 source records
are released:3,072 cache sources and5,504 task sources (3,200 planner-owned plus
2,304 auxiliary bank0 copies). Bank0 rebuilds admit two,one andzero observations;
repeated lookup after marker mutation verifies reuse, including empty-root reuse.
The matrix compares cache/vector capacities,creation order,stable source bytes
[0,3d) and[89,8d),constructor tags,ordered topology,disjoint cache/task identities,
borrowed snapshot survival and exact release order/postcleanup fields. Active
and evaluator copies survive explicit invalidation before later task retirement.

The original generated task6 matrix remains:960 candidates,811 saves,1,313
planning slices,128 initial installs,192 replacements,320 retained sweeps,896
body slices,128 risk stops,448 task releases and640 suffix buffers retained at
release. Native inputs are independently authored; no original output seeds
native state. Original manifests pass before/after and text stays RX/unchanged.
The actual final check count,digest and compiler closure are in the immutable
report; compatibility is independently rerun below.

`task6-composition-cache-compat-1.json`, in the preceding OTC1 result directory,
reruns the unchanged prior task6 fixture against the extended shared header:
2,020,665 checks,71 declared/60 compiler inputs and digest8574435755676549549
agree on original/native/sanitizers. New compatibility evidence supports the
existing OTC1 scope; its old evidence and source hashes remain historical.
Inspection2 retains23 original entry ranges,including42f250 and4181d0. Missing
older-project decompilations retain central inventory instruction ranges; no
function boundary is invented. The incoming3a5788e8..fa354308 review finds33 exact
file transitions and no missing/mutated receipts. Ten Critical-needs policy tests
check planning presentation only.

Trial1 rejected a misleading-indentation compile warning. Trial2 completed the
original/native/sanitizer assertions but its final summaries differed by two
native-only accounting entries per episode; that incomplete result is preserved.
Final1 corrects those fixture entries and additionally checks every cache/task
source set for disjoint allocation identities. Neither failed attempt is counted
as current equivalence, and no historical source hashes are refreshed.

## Boundaries and next recovery

The32x32x8 eight-region World,ordinary brain0/owner0,two borrowed enemy snapshots,
model,successful allocations and external scoring34/training refusal/commands/
registration services remain supplied. Native cache grouping handles the tested
merged/unmerged two-member cases. Arbitrary record counts,owners,negative-region
line queries,unusual activity/allocator states and exceptions are not established
by this matrix. Intermediate cursor/probe bytes remain excluded as before.

This closes **source-cache vector/forest ownership**, not the raw observation
RB-tree or creature-snapshot allocator. Supplied snapshots remain live through
all referring task retirements. Original observation insertion/refresh/expiry,
resource-bearing snapshot construction/copy/destruction,cache/task interaction
with erased records,mode2 task notifications,full World cadence and installed/
live replacement remain open. The native cache header's unused first16 bytes
are fixture storage; no portable RB-tree ABI or whole-brain ownership is inferred.
The broader Critical actor-services contract remains partial.

The next bounded step is owned observation-record/snapshot insertion,refresh and
expiry composed with these caches and surviving task copies. Compare snapshot
resource ownership and the stale/missing-memory path explicitly before claiming
that tasks can safely outlive original record erasure. Other task families and
full World/live acceptance remain separate work.
