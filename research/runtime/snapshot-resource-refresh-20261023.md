# Resource-bearing snapshot refresh — OSR1, 2026-10-23

Resource-bearing observation refresh is now implemented and compared in the
normal one-actor/owner0 domain. Refresh copies the actor's packed value while
clearing the snapshot's owning callback/list pointers. The actor retains its
resources. Other copied aliases remain borrowed, including stale aliases after
an unobserved actor reset or destruction. No reference counting or generation
repair is introduced.

## Correction to the earlier ownership interpretation

Pinned NoCD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
High confidence for the instruction stores and the compared normal domain.

The older Ghidra export of4181d0 misrepresents the dynamic stack frame and omits
two writes between the two418d50 calls. The instruction trace shows:

| Address | Confirmed operation |
| --- | --- |
|4185a3|Zero EDI; intervening normal-return callees preserve EDI|
|41861f|Temporary snapshot base is ESP+48|
|41862c|Assign live actor into that temporary|
|418631|Write zero to ESP+750, temporary+708|
|418638|Write zero to ESP+941, temporary+8f9|
|418658|Assign the stripped temporary into the existing stored snapshot|
|41866c|Destroy the stripped temporary through505f70|

Both assignments clear6d0/6d4 through418d50. Its normal temporary destructor
therefore has no callback, list or histogram allocation to release. This is the
same resulting ownership separation as construction through418b60/42fb40.
The earlier OOS1 claim that resource-bearing refresh could retain owning aliases
was incorrect: it missed418631/418638. Its conservative native refusal is now
removed for valid stored values. Earlier evidence and source hashes remain
unchanged; OOS1's execution credit remains its original resource-free domain.
New OSR1 execution establishes the resource-bearing extension independently.

Native OwnerObservationSnapshots.refresh now copies the stripped value. It
retains an explicit native invariant guard against a corrupted destination with
nonzero708/8f9. Normal construction and refresh always leave these fields zero.
That guard is not an original guard or evidence for arbitrary destination state.

## Concrete resources and retained aliases

Actual5060a0 creates a16-byte callback, a12-byte list and48-byte sentinel.
The callback has vtable5c7264, function field51e890 at+4 and actor backpointer
at+c. The actor owns it through708 and aliases it throughcf3. Vtable slot1
points to52cbe0; its52cc00 destructor writes base vtable5c5c40 before the
flag1 deleting path frees the object. Actual52cc70 releases list nodes during
reset. Actual505f70 performs actor destruction, including histogram release
through58f6f0. Native composition uses the existing ActorLifecycle API and a
bounded callback deleting provider for this class/flag1. Callback invocation,
other callback classes and exceptional flags remain separate.

Snapshot708/8f9 and6d0/6d4 are zero. Snapshotcf3 and8fd retain the callback
and authored list-cursor addresses from the copied actor. Snapshotac retains the
borrowed CFG pointer. The original does not repair those aliases when a hidden
actor resets or is destroyed; later visible refresh replaces their bytes. The
fixture records pointer identity and allocation liveness without dereferencing
freed aliases. This does not establish safety for every downstream alias consumer.

Actor reset drops histogram backing without freeing it, as previously recovered
by the actor lifecycle work. OSR1 preserves and checks those retained generations;
later fixture disposal explicitly frees them. Observation refresh does not acquire
or free that backing. Actor destruction does release the currently referenced
histogram. The fixture does not silently repair the original reset retention.

## Execution evidence

The source-bound successful `snapshot-resource-trial-2.json` prospectively binds
44 inputs, including37 compiler inputs. Native64, original32 and
ASan+UBSan+LSan agree on7,550,016 checks and digest947184330397392515.
It covers64 episodes,2,304 mode1/2 updates,256 record insertions,1,088 refresh
events,192 expiries,192 task clones and1,152 retained stale snapshot-reference
observations.576 real callback generations are created and deleted across576
actor resets and256 explicit actor destructions. Every owned allocation is
released after preserving the explicit reset-retention baseline.

Each episode has three visibility cycles. Populated resources are present at
insertion and refresh. List content changes, an unobserved reset, visible refresh,
age1024 retention/1025 expiry, reacquisition, actor destruction while an observation
survives, and another reset/refresh are composed with retained task copies.
Full packed snapshots, resource allocation bytes, callback/list ownership zeros,
borrowed alias liveness, callback base-vtable transition and source pointer/activity
identity participate. A before/after resource image verifies that observation
updates do not release or mutate the actor's allocations. Actual413330 generates
bank1 cache/task copies; actual40a340 cleans up task sources after record expiry.
Native inputs are authored independently; original output never seeds native state.

The successful run follows one preserved compilation failure: renaming the reused
OOS1 fixture's main exposed its implicit main-only return. An explicit return was
added without changing its ordinary engine path. A separate OOS1 compatibility run
against the corrected shared header supplies new evidence for its unchanged scope.
Its native guard case now checks corrupted destination ownership, not rejection of
a valid resource-bearing source. Neither historical OOS1 reports nor their hashes
are rebound. Manifest checks pass before/after, original text remains RX/unchanged,
and only allocation/free/buffer-free boundaries are redirected in the new fixture.

Inspection1 retains24 entry instruction/decompilation exports and the concrete
callback vtable. The new dispatch entry preserves the unresolved callback invocation
slot. Incoming f2253bb8..ac0b9e3b review finds27 exact file transitions and no
missing/mutated receipts; this is accounting, not a past gate or new execution.

## Remaining boundaries

One stationary ordinary actor, one enabled owner, increasing ticks and authored
visibility/activation/order values/histogram backing remain the domain. Task and
notification lists in4181d0 are absent. Multi-record erase/cursor continuation,
other owners, whole-brain4156b0 destruction, allocation failure/SEH, arbitrary
callback classes and actual consumers of stale borrowed aliases remain open.
Intermediate source traversal/probe bytes, other task families, training/backends,
full World/session cadence and installed/live replacement remain separate. Native
ordered indexing does not implement the original red/black tree ABI. The Critical
actor-services umbrella stays partial; gameplay balance is unchanged.
