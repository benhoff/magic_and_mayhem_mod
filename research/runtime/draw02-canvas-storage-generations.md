# DRAW-02 canvas storage-generation boundaries

Step 2 remains in progress. Review `RS.canvas-create-lock`,
`DRAW.canvas-lock-recovery` and `DRAW.canvas-release-client` before extending
original wrapper storage observations. Successful startup pointers and synthetic
clients do not establish real allocation generations, COM aliases, all restore
paths or unrestricted use after unlock. No ownership status is promoted here.

## Pinned static inspection

`draw02-canvas-storage-static-01.json` preserves the broad disassembly export.
Its release window starts at `0x58b3e1`, one byte after the registered entry;
do not use that window alone to infer the entry instruction sequence.
`draw02-canvas-storage-aligned-01.json` separately checks the pinned NoCD hash,
PE file-backed entry offsets and exact prologues before decoding each routine
from its entry. These reports are static evidence, not engine executions.

At `0x58b660`, the initial Lock result is stored at `0x6f8150`.
In the lost-surface branch Restore overwrites that global; the retried Lock
result is not stored there. The shared result therefore cannot prove retry
success. The common tail reads descriptor pitch and pixels, optionally calls
auxiliary Unlock, and returns the saved pixel pointer even after that call.
A cached returned pointer does not prove that a map remains open.

At `0x58b3e0`, a null interface at wrapper `+8` exits before clearing other
fields. The `+4` and `+8` Release loops share one counter: after 1,000 calls
in the first loop, the second still executes once. The existing unchanged
original-client fixture verifies this ordering, optional auxiliary/clipper calls,
current-wrapper retirement, populated registry compaction and wrapper writes
across 5,184 scripted cases; see `drawing-canvas-release-original-20261011.json`.
The scripted interfaces do not model real aliases, shared refcounts, physical
allocation destruction, external mutation or reentrancy.

## Next observation boundary

Observe create, release and returned-lock transitions with process-local logical
wrapper generations, storage epochs and explicit invalidation. Retain entry and
return identities, caller, thread and cached dimensions. Reuse of a numerical
address must not silently retain an old identity. A changed pointer, interface
or pitch invalidates an earlier observed storage epoch. The observations remain
partial: unchanged addresses cannot establish unchanged allocation ownership,
all alias writers, successful upload or complete post-unmap lifetime.

The existing producer cache only changes its pixel address at Lock return and
retires wrappers at create/release return. A release/create in progress can
therefore leave a prior cached address visible. Any new query must expose this
uncertainty or refuse it; pointer equality alone must not admit ownership.

## Pending wrapper transition guard

The additive storage-point flag128 records a matching wrapper with create,
release or Lock in progress. It is exclusive with known/ambiguous/extent-mismatch
association flags and carries no canvas metadata. Existing wire version1 and
historical journals remain readable; current decoders explicitly admit this
additional unknown state. The export compares addresses only, preserving LastError.

The PE32 fixture passes same-wrapper transitions for all three calls, unrelated
wrapper and non-lifecycle frames, retired slots and existing output/cache guards.
Sixteen decoder methods pass. The first new client test supplied a list where
wire metadata uses tuples; its retained negative report records the fixture error.

Original01 closes normally with337API336EGL witnesses. Map-time points contain
795pending transitions,14unknown and491stopped, zero known matches. Later source
submissions retain339known last-map points (all closed maps),1015unbound and
284stopped; client source queries retain145known and1493unknown. 119protected
pairs close through224worker commands. These points do not prove post-unmap
allocation ownership, uploads, complete aliases/writers or unrestricted lifetime.
This corrects admission of earlier map-time cache matches without rewriting
historical reports or claiming complete step2/full47/21 acceptance.

Fresh source-only replay matches105eligible protected-source/worker/EGLphysical
frames with zero differing pixels;232contexts remain excluded. This validates
the admitted cohort after the storage-point transition guard, not whole lifetime
or all routes/actions/full DRAW-02 closure.

## Independent transition journal

Producer checkpoints skip clean or already sampled Lock returns. The new
opt-in `MNM_DRAW_STORAGE_TRANSITIONS=1` recorder therefore observes all hooked
create/release/Lock entries and returns within its bounded source interval,
independent of pixel sampling. It reads wrapper fields and cached scalar state,
never source pixels or private driver layouts. This adds diagnostic work.

`canvas-storage-transitions-0000.bin` uses MNMCST02/version1, a64byte header
and96byte rows,131072rows including the terminal, and512local address identities.
Rows identify invocation, phase, original caller RVA when known, thread, source
prefix/queue/failures, cache canvas/address/extent, readable wrapper interface
addresses/extent and observed Lock-return epoch/address. Process pointers stay
inside the observer; zero caller RVA is explicitly unknown. Every returned Lock,
including null or numerically reused pointers, advances its observed epoch.
Address IDs only represent numerical equality during this process. They cannot
prove allocation generations, exclusive ownership or post-unmap lifetime.
Create/release EAX is unspecified and not exported. Terminal counters retain
open invocations, failures, address count and epochs; capture refuses open calls.
Budget/write/identity failures remain explicit rather than silently truncating.

Six decoder methods validate ordering, null returns, unmatched/changed identity,
open closure, non-Lock raw results and missing/budget/address gaps. The PE32
fixture records six synthetic invocations, three returned Lock epochs, one
reused address across locks, a null result and a new cache canvas at a reused
wrapper/storage address. It does not execute the original hook ABI or real COM.
The standard observer builds; original correlation remains a separate run.

DRAW-02 step2 storage transition journal: six-invocation PE32 fixture and
standard observer build pass. Original01 closes884observed invocations:
60creates15releases809Lock returns119numerical address identities,18unknown
caller rows. Fresh offline binding validates1687known81unknown cache points at
exact producer prefixes, versus668source lock checkpoints. Numerical address15
appears on canvases7/8/9/13; shared backing versus allocation reuse remains
unassigned. No actual ownership/allcall/nativeparity/full47/21closure follows.
Next resolve caller identities and bind observed storage epochs to submissions,
while retaining all other routes/actions and later steps as pending.
