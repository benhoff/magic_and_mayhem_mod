# Bounded copy-order mismatch

Evidence: `draw02-source-physical-close-copy-order-01.json`, original120
source-close capture and its completed failed native comparison. Confidence:
high for recorded geometry, checkpoint hashes, unique local associations and
worker ordering; provisional for a general cause or correct ordering repair.

Of141eligible source/physical contexts,140match. API3768/EGL430at producer
prefix208533differs in1600pixels, rectangle440310..480350, in both native
source and display. Original CPU checkpoint1334(sequence208514)and1335
(sequence208520)have the same hash as the held source. Native1334matches;
native1335changes after background-copy replay.

The local producer return stream lists background save1→5at208516before
full copy2→1at208517. Unique local API/rectangle/source-destination matching
links those to API3757(thread424)and3756(thread384), respectively. Original
full-copy commands1037/1038finish before background-save command1039enters:
last full worker return record3115, first background worker entry3116. Thus
actual driver order is full→save while producer return order is save→full.

This establishes a missing ordering boundary in a return-only input model.
It does not alone prove every CPU/GPU view, alias or the full cause of the
40x40difference. Next observe explicit request/entry/return/API/worker identity
for copy operations and retain all missing/ambiguous links before a general
source-only replay correction. Do not swap records by hypothesis, discard the
failed context, use original raster as input or refresh older evidence hashes.


## Explicit request observation

`draw02-copy-order-fixture-02.json` passes five owned PE32 cases, including
actual two-thread reversed returns. `draw02-copy-order-original32-02.json`
retains913paired request IDs with actual gameexit0.
`draw02-copy-order-producer-links-01.json` checks every descriptor and emitted
COPYsequence against the unchanged producer stream:913matches,0unmatched,
45adjacent entry pairs with reversed return order. Four decoder and eight
source-cursor frontend guards pass. These observations establish entry and
return identities only; API request/worker binding, driver order, CPU/GPU view
ownership and the safe source-only replay repair remain pending.


## Strict API and worker associations

`draw02-copy-api-worker-bindings-01.json` associates all913requests with913
successful APIcalls and1130completed worker commands. Require same owner,
geometry, opaque/keyed flags, logical source/destination/extent and all three
API point prefixes within the explicit request interval. Unique completed
submission/packet/worker identities remain separate from these source metadata
checks. All1075additional stopped-source APIentries are retained outside the
association. Six guards cover missing phases, descriptor/owner/key/interval
changes, ambiguous/duplicate associations and absent/changed/shared commands.

Forty-two of45adjacent return reversals execute in entry order at the worker;
three execute in return order (requests692/693,837/838and841/842). Thus neither
producer entry nor producer return order supplies a general replay ordering
contract. This is static analysis of the frozen original capture, with no new
game execution, renderer correction or semantic/physical ownership promotion.


## Complete source checkpoints

The source-only unchanged renderer matches all1337checkpoints in the explicit
original32request cohort. The full earlier original120cohort compares2182:2180
match,1335/canvas2differs1600pixels and1336/canvas22differs950. The original
V4 update-end bookkeeping marker is strictly decoded and retained separately
from the native drawing input prefix; every drawing operation/checkpoint remains.
Initial native refusal of that metadata record is retained as nativeexit1/0
compared. No native drawing or replay ordering repair has been made.

The successful32comparison alongside45observed return reversals establishes
that those reversals do not invariably cause different checkpoint output. The
120failures narrow the concrete interval requiring ordering/view investigation;
they do not establish the general cause or a safe record-sorting repair.


## Storage acquisition and primary failure provenance

Three failed storage acquisitions remain retained. The narrower default-display
run reaches original SetDisplayMode800x600x16with0x80004001(E_NOTIMPL), then
retains no source interval. The frontend formerly replaced this acquisition
error with incomplete optional-copy journal refusal; it now preserves the
primary exception/error and recordscopy_order_errorseparately. Four executed
guards retain both failures and prevent false acquisition promotion.

An explicit800x600x24 private-Xvfb retry with identical runtime binaries passes
32queues/906requests/972closed storage calls,897lock-return epochs and118local
address IDs, with actual gameexit0. All906COPYrecords and API/worker associations
match;1251additional APIentries and44return reversals remain explicit. Local
epoch/address observation does not assert storage ownership or stable addresses
across launches. The screen-geometry observation constrains reproducibility;
complete startup causes and CPU/driver views remain pending.


## Step 2 worker execution producer bounds (2026-10-13 UTC)

V5 query identity increment committed as2098ea97with staged/normal hook checks.
Reviewedc83284b6..2098ea97:21exact receipts,0gaps. Optional
--wine-worker-producer-points requires original-active producer and Wine worker
observation, and is refused in controls. It uses the existing null-COM read-only
SceneDrawingSourcePointV1export before/after original opcode42worker bodies.
The separate MNMWPP01 journal retains worker/thread/legacy-entry identities,
unavailable/refused metadata and complete counts; legacy wire stays separate.
Getter and original callback run outside journal_lock, with pending workers
preventing ordinary close; forced midclose remains incomplete. No pointers or
pixels are transported, and no original drawing body is suppressed.

Initial owned111/123guards fail noncanonical FP comparison because stack
snapshot destinations contained unwritten upper-region bytes. Raw before/actual
snapshots and original source hashes remain retained. Initialized comparison
buffers fix the guard without changing the runtime recorder. Seven owned PE32
configurations pass:enabled/disabled/unavailable/refused/midclose/budget/invalid,
argument/error/canonical512byteFP preservation, getter/original cross-thread
journal reentry and postclose forwarding. Budgetexit99and missing midclose
return remain explicit refused evidence. Four decoder/eight frontend/four
diagnostic-cadence guards pass.

Fresh original32passes32queues/173540records/1335oracles/30139986bytes,
910copy requests,2591worker pairs,0unavailable source points.1127pairs are live
and1464stopped;110pairs have changed producer prefixes.583physical planes/
1622consumed inputs and actual originalexit0 pass;2927original-manifest files
verify before/after. Static all910copy/APIjoins account1127commands,1098
additional APIcalls and44return reversals. Among those1127commands,1017have
stable prefixes,109bracket records only on other logical canvases,1brackets
an observed copy return touching its destination. Request714/worker852
(source2/destination1)bounds49777..49778include cursor copy6→1return49778.
Maximum observed producer interval spans518records. All1464outside workers
remain listed; no interval or failed context is discarded.

Reassessment: these are execution bounds, not an atomic byte or complete
replay-order contract. The110intervals contain other producer work; logical
canvas separation does not disprove backing aliases. Resolve the one touching
copy interval and noncopy/checkpoint ordering before a source-only ordering
repair. Earlier120source/physical failures remain failed. Step2and all47/21
remain open; steps3–7stayordered/pending.


## Step 2 delayed producer return attribution (2026-10-13 UTC)

Worker-prefix work committed as `364116d5`; staged gate and commit hook passed.
Reviewed `2098ea97..364116d5`: 23 exact file-version receipts, zero gaps.
New static `draw02-worker-prefix-delayed-return-01.json` preserves all 1127
copy-bound commands and 1464 outside-scope workers. Its one touching producer
record, sequence 49778, binds exactly to cursor request 711/API 3115 and
commands 847/848. Their worker records 2540..2544 finish before full-frame
request 714's commands begin at 2552 (worker 851) and 2555 (worker 852).
The cursor producer return was recorded during worker 852's producer-prefix
bounds 49777..49778, after its associated driver commands had finished.
This is delayed producer journaling in this retained trace, not proof of a
driver mutation during worker 852. Four new guards distinguish prior, later,
overlapping and same-request execution and refuse changed request descriptors,
owners, sequences and incomplete worker intervals; six association and four
worker-prefix decoder guards also pass. No original execution was repeated.

Reassessment: original producer return order cannot stand in for driver order.
The one touching interval is attributed, but stable prefixes and other logical
canvas IDs still cannot exclude backing aliases. Noncopy/checkpoint execution,
source-byte ownership and the earlier 120-window failures remain unresolved.
Keep native ordering unchanged until a bounded execution policy accounts for
all relevant operation and checkpoint boundaries. Step 2 remains open; steps
3–7 remain ordered and pending, with all 47/21 requirements unchanged.
Latest retained storage inventory reports 1268.343 GiB working and 220.933 GiB
free, leaving about 11.657 GiB under the working target. This increment uses
retained journals and no new game installation or large capture.


## Step 2 worker-bound checkpoint comparison (2026-10-13 UTC)

Delayed return attribution committed as `0b21befb`; staged gate and normal hook
passed. Reviewed `364116d5..0b21befb`: nine exact file-version receipts, zero
gaps. Fresh `draw02-worker-prefix-checkpoints-01.json` runs unchanged native
return-order replay against the retained worker-bound 32-update trace. All
1335 independently retained CPU checkpoints match, native exit 0, with source
hashes checked before/after and original manifests verified. Only parameters
and pinned encoded assets enter native input; original planes remain external.
The 1 GiB guarded reservation passed; raw comparison output is retained.

Reassessment: this trace's 44 adjacent copy return reversals and delayed cursor
return do not produce a checkpoint mismatch. Their presence alone is not a
repair trigger. The earlier 120 trace retains both checkpoint failures and its
physical mismatch. A new guarded 120-update capture adds actual worker-prefix
and V5 storage-query identities to investigate larger intervals; its result
must be assessed independently. Step 2 remains open, steps 3–7 pending and all
47/21 requirements unchanged.
