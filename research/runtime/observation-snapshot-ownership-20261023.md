# Owned observation snapshots — OOS1, 2026-10-23

OwnerObservationSnapshots owns the raw observation values that the earlier
knowledge-cache/task compositions borrowed from their fixtures. It reconciles
the independently recovered OwnerKnowledge logical decisions into stable owned
record allocations, preserves those allocations during refresh, and releases
snapshots and records on expiry. Native cache and task sources borrow the owned
snapshot address. The host ordered index is an intentional representation policy;
it does not implement the original red/black tree topology or iterator ABI.

## Original findings and confidence

Pinned NoCD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
High confidence within the declared normal successful allocation domain. Static
inspection3 retains instructions and read-only Ghidra decompilation for20 entries;
inspection1/2 remain historical inputs rather than current-source validation.

A tree node is0xe68 bytes: links at0/4/8, key12, seen16, marker20,
packed creature snapshot24 of size0xe4b, padding and color at0xe64.42a5d0
searches for duplicate keys;42cde0 allocates and42fb40 constructs a value.
42a6e0 transplants/rebalances nodes without copying a successor's snapshot,
then505f70 destroys the erased snapshot before597870 releases the node.
42f2e0 clears the tree. Native record allocation addresses survive refresh.
The one-record execution domain does not establish multi-record balancing,
duplicate insertion, general erase continuation, or PE tree interoperability.

418d50 copies all0xe4b packed bytes, then clears the words at6d0/6d4.
Those words belong to the auxiliary subobject at6b4;58f6f0 releases its
buffer through5979b0 if nonzero.418b60 and42fb40 additionally clear the
owned object pointer708 and owned list pointer8f9 after copying. Construction
therefore does not acquire these live-actor resources. Scalar inventory, form,
status, resource, configuration pointer and remaining packed bytes are copied;
this is byte-layout recovery, not independent semantic recovery of every field.
The configuration pointer is borrowed and remains fixture-owned.

505f70 calls the object's virtual deleting destructor with argument1, clears708,
removes list nodes through52cb60, releases the list head and outer list, clears8f9,
then calls54dac0 (no-op) and58f6f0.52cb60 unlinks/frees the node and decrements
count; its value word is not freed. The list/object/buffer comparison checks
normal populated destruction, zeroed resource pointers and repeated destruction.
The actual virtual object destructor and buffer allocator backend remain explicit
services. SEH/unwind, malformed/cyclic structures and aliasing ownership are open.

The initial refresh interpretation missed two original instruction stores.
418631 and418638 clear the temporary's708/8f9 before assignment into the stored
snapshot and temporary destruction. Resource-bearing source refresh therefore
does not transfer those owning pointers. [OSR1 correction and new execution](snapshot-resource-refresh-20261023.md)
implements and compares this extension, including borrowed cf3/8fd aliases and
actual actor resource generations. Historical OOS1 reports and fingerprints remain
unchanged; their original resource-free domain remains the execution scope below.
The old conservative source-resource refusal is superseded. A guard against an
invalid destination owning708/8f9 remains an explicit native policy.

## Comparison and composition

The source-bound final1 report retains exact metrics, digest, compiler closure
and commands:6,103,742 checks,digest17321810403833537543,42 declared inputs and35 compiler inputs. Original32, native64 and ASan+UBSan+LSan execute64 episodes,
1,728 supplied updates over three visibility/expiry cycles per episode. Each
uses one enabled owner0 and one stationary ordinary actor in32x32x8. Ticks
increase strictly. Visibility, CFG storage and absent task/notification lists
are authored; actual original4181d0 modes1/2 perform marking, cache invalidation,
matching, construction/refresh and expiry. Native matching is the existing
OwnerKnowledge implementation; raw reconciliation consumes its logical output,
never original output. Owner0's original RB helpers execute without replacement.

The matrix covers full packed patterned assignment and construction,256 record
insertions,896 refresh events,192 expiries and192 bank1 task clones. Complete
stored bytes are compared on visible and hidden ticks, including the exact
age1024 retention/age1025 expiry boundary after last observation. Source caches
are invalidated before updates. Actual413330 creates the original bank1 cache
and common task clone; native OwnerKnowledgeCache and OwnerTaskSources compose
with the new owned snapshots. Actual40a340 retires original task source copies.
All source/record/list/object/buffer allocations are released; freed blocks are
poisoned and allocation extents/lifetimes and suffix canaries are checked.

576 retained-source observations witness a freed snapshot address remaining in
a surviving clone's35 field. Its activity89 remains3 until a later source-update
contract changes it. This comparison checks identity and liveness only; it never
dereferences the freed snapshot. Expiry alone does not invalidate every task
pointer or change its activity. Original407c10 stale/missing resolution remains
separately recovered; executing it after expiry is not claimed by this batch.
Normal task cleanup does not take ownership of that borrowed pointer.

Manifest verification passes before/after each original-consuming run. Original
text is sealed RX and checked unchanged after execution. Explicit process-only
redirects are limited to allocation/free/buffer-free services. No original file
is patched. Initial trial1/2 stopped on fixture compile warnings; trial3 exposed
that a moving input created multiple anonymous observations while its assertion
assumed one. Trial4 uses the declared stationary actor and passes; final1 adds
native refusal coverage and formatting. Earlier results and hashes are retained.
Incoming fa354308..f2253bb8 accounting reviews26 exact file transitions with no
missing/mutated receipts; it does not assert a past gate or new execution.

## Remaining recovery

Normal resource-bearing refresh and selected borrowed alias lifetimes now have
separate OSR1 proof. Arbitrary callback classes/alias consumers, multi-record
erase/continuation, other owners, native PE tree compatibility if needed,
failure/SEH and whole-brain
4156b0 destruction remain open. Mode2 nonempty task/notification consequences,
intermediate source traversal/probe bytes, other task families, actual model
training/backends and full World/session cadence remain separate. The Critical
actor-services umbrella stays partial. This is bounded headless integration,
not installed/live replacement or gameplay-balance work.
