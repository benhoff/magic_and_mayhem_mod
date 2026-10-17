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

## Version 2 relocated caller identity

The pinned hook9 prologue copies `0x58d320..0x58d329`, including the
`CALL 0x58b660` at `0x58d324`. Its observed return address is the installed
trampoline plus9. Version2 maps only that exact address, on Lock events, to
original caller RVA`0x18d329`, marking row word22=1. Other caller addresses
retain normal image-range classification or explicit unknown0. The actual
return continuation pointer is never changed. Neighboring offsets, null
trampoline and arithmetic overflow do not match. Entry/return must retain the
same mapping flag; version1 requires zero and preserves original01unknowns.

The PE32 fixture verifies exact/neighboring/null/overflow/image callers using
the same helper, emits six paired invocations with three Lock epochs, and all
nine decoder controls pass. Fresh original correlation is a separate result.

DRAW-02 step2 exact caller increment: PE32 exact/neighbor/null/overflow
controls and9decoder methods pass. Current version2 original01 closes884
invocations809Lockreturns with18explicitly relocated rows at58d329 and0unknown
callers;118protected pairs222commands333API332EGL remain bounded observations.
Fresh prefix binding passes1687known81unknown points; version1 reports retain
historical unknowns. Next bind observed storage epochs at exact source queries;
all unhooked paths/real allocation/aliases/writers/uploads/routes/actions/full47/21
and later-step closure remain pending.

## Version 3 query-to-epoch binding

Version2 point exports retain the64byte envelope and publish an explicit query
ordinal in word15. Legacy exports remain version1with zero reserved word15.
The opt-in observer chooses V2 only for storage-transition capture. The version3
transition journal records each query under the same producer scope, classifying
matching observed epoch, absent epoch, changed address or unknown point. A known
cache point need not have an owned or known allocation epoch. Create/release
retire earlier observed epoch state; numerical address reuse does not preserve it.

See `../../research/formats/drawing-storage-transitions.md` for the wire layout.
PE32 V2 export controls and corrected seven-invocation/four-epoch recorder fixture
pass. One synthetic client query initially omitted its required pitch-only extent
flag; the strict decoder refusal and raw artifact are retained. Current decoder
controls require exact external query ID/metadata and retain unlinked IDs.

Combined original01 is negative during setup: source9 lease17copy rejects a
foreign tag32source1/destination5frame alongside its owner tag7source9/destination2.
The extra frame is identified; byte overlap/disjoint backing remains unproved.
Its70closed transitions111queries and refusal files remain partial evidence.
Copy admission is unchanged. A fresh bounded retry is separate execution.

DRAW-02 step2 explicit storage-query increment: V2exports retain V1behavior
and publish exact query ordinals into a version3epoch journal. Controlled PE32
export/epoch states and36current decoder/control methods pass. A synthetic
client extent-flag error and original setup source9lease17foreign tag32copy1to5
refusal are preserved; no admission loosening or race-resolution claim.
Original02normal Quit closes884invocations809epochs1869queries482known.
Fresh reanalysis binds every1869external query ID/metadata with0unlinked,
482known1387unknown epochs and2169known1468unknown producer-prefix cache points.
118heldpairs222commands329API328EGL remain separate. Native replay matches104
eligible frames0diff225excluded. Next extend exact epoch/version/ownership
accounting and other routes/actions; all47/21and steps3–7 remain open.

## Version 4 unknown-address candidates

The104matched native frames in version3original02have zero direct submitted
map/client epoch links, although their protected CPU snapshots are known.
`draw02-storage-query-cohort-gap-01.json` retains this distinction. Source/thread
identity and pixel parity do not establish the submitted buffer address.

Version4adds bounded candidate rows after live unknown queries (flags3only).
At most16same-pitch live cached canvases are compared. The probe records relative
address offset/direction, dimensions, current numerical address IDs, observed
epoch and public VirtualQuery state/protection/type plus allocation-base equality.
It reads no pixels/private driver layouts and changes no point admission.
Memory-query failure, stopped/ambiguous/pending points and missing epochs remain
explicit. Equal allocation bases or overlapping numeric spans do not prove
physical aliasing, full ownership or an upload path.

The PE32 fixture uses one owned allocation with a16byte offset to verify numeric
span/region observations. Fifteen decoder controls cover wrong IDs/epoch/region/
span/budget and retain older wire versions. Original address relationships are
a separate run; protected-copy diagnostics are disabled for that isolated probe.

DRAW-02 step2 candidate probe: PE32 owned-memory offset/region fixture and15
decoder controls pass. Original01normal Quit retains884invocations1853queries
267candidate rows:0numeric span overlap247different20equal allocation bases.
Relevant source-client pointer differs1048592bytes from cached canvas2 in a
different allocation; a small offset is excluded within this sample. Actual
physical alias/transfer remains unassigned. Passed source/query reanalysis checks
2172known1449unknown cache points. Analyzer pre-execution duplicate-key failure
is preserved. No protected-source or fresh native comparison for this isolated
probe; previous104matched frames have0direct submitted epochs. Next observe
public copy_sub_resource_region boundaries with pinned ABI/forwarding first.
Allownership/uploads/routes/actions/47requirements21operations remain open;
steps3–7 remain ordered/pending.

DRAW-02 step2 static source/copy reassessment: exact-entry manifest-checked
58d540inspection disproves its provisional buffer-setup label: it queries32byte
pixel format, writes low16RGBmasks and returns a derived bit. No allocator claim.
New bounded58bf40/58d320copy/lock-bind instructions are retained separately.
Retained version3selected104APIcalls each have two original Wine blits: known
mapped source→client texture, then client texture→primary. The initial source
query has the exact held canvas/state/prefix in all104; the final source lacks a
direct CPUepoch. Next validate both command/worker links rather than assuming a
single submission. This is a provisional chain finding, not new execution or
complete upload/alias/ownership/full47/21closure.

DRAW-02 step2 two-stage source chain: four adversarial methods pass.
Current immutable reanalysis verifies raw original/native reports and seven
journal hashes, accepting all104previous eligible frames with0chain exclusions.
Initial mapped-source V2query/observed epoch exactly matches held canvas/state/
prefix; two same-API full blits share an intermediate texture generation. Each
stage has one packet and belongs to the protected lease, with initial worker
return before final worker entry and exact final EGL/native0diff witness.
The final direct sourceepoch remains absent because it is the intermediate
texture. This closes the specific two-stage linkage gap in this cohort, not
whole allocation/alias lifetime/unobserved uploads/scanout/allroutes/actions.
Other225original contexts/full47/21/steps3–7 remain open. Next extend the tested
chain accounting to the longer whole-update reference, then remaining source
lifetimes/routes/actions; no new engine execution is asserted by this analysis.

DRAW-02 step2 current whole120epoch/chain result: attempt01fails atqueue30
source61lease130foreign tag32copy5to1; exact diagnostics retained and strict
admission unchanged. Attempt02normal Quit closes120cadence-bound queues,
631534records170046112bytes2166oracles,151protected pairs290commands,
827API826EGL and1345transitions/1267Lockepochs. Exact reanalysis binds4858
external queries1477known3381unknown epochs0unlinked,4082known3466unknown
producer-prefix cache points. Source-only native replay matches139eligible
frames0diff688contexts excluded. All139pass full two-stage epoch/held-byte/
intermediate texture/both protected commands/ordered worker/final EGLchain.
This extends bounded evidence without fixing the intermittent foreign-frame
race or proving whole allocation/alias/unobserved upload/hardware/scanout,
allroutes/semantic actions/47requirements21operations. Continue remaining
acquisition boundaries in step2; steps3–7 remain ordered and open.


## V5 query inputs and completed Lock returns

The resource32 retirement-fix cohort records 895 live transition-pending map
queries, each inside one active same-thread observed wrapper Lock. The original
cache getter deliberately withholds a completed association while that matching
wrapper transition is pending. V4 query records do not retain the input address
ID for this branch; later return IDs alone cannot reconstruct it. Static V4
analysis therefore retains all 910 query records as unbound legacy inputs.

The V5 collector adds only a local numerical input ID in query word 7 and
keeps known/pending/unknown cache policy unchanged. Known IDs and candidate IDs
are checked against the query; null remains zero and historical versions keep
reserved zero. The independent query/return adapter requires a unique active
same-thread Lock, unchanged known logical canvas and exact returned/cache/input
numerical equality. All absent, null, outside, ambiguous, failed, stopped, changed
and unequal observations remain explicit. This is pointer equality and event
order, with no byte/allocation/view/upload ownership or replay-order claim.

The owned PE32 V5 fixture passes eight calls/five return epochs, seven queries,
a pending numerical input joined to its later return and a null input, without
reading mock address bytes or executing the original engine/hook ABI. Eighteen
strict decoder and six interval/identity guards pass. Fresh original V5 passes 32 queues and actual game exit 0. All 910 live
queries have exact same-thread interval/input/return/cache associations; 805
external points remain inactive. No allocation or source ownership is inferred.
