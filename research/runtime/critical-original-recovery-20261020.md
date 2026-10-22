# Critical original behavior recovery

This work investigates the six Critical rendering recovery items. Original
No-CD SHA-256 is 40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168.
Original media is immutable; original-consuming experiments require manifest
bookends and fresh prospectively declared execution evidence.

| Parent contract | Recovery target | Boundary retained |
| --- | --- | --- |
| NR.directdraw-retirement | Real interface/caller and alias/writer lifetimes | Physical-driver loss and complete journey/hardware acceptance are independent. |
| RS.canvas-create-lock | Real creation, COM identity and held read lease | Readability or numerical pointer reuse cannot establish allocation ownership. |
| RS.canvas-bind-clip | Alternate-buffer initialization, reachability and writers | Five direct references to 0x6e1f68 are reads; this is not proof that no indirect writer exists. |
| RS.canvas-startup-clear | Actual first driver fill result and pixels under a valid lease | Wrapper EAX is not a driver HRESULT; scripted COM does not prove real completion. |
| RS.world-dispatch-targets | Clip prelude/postlude, queue traversal and concrete helpers | Backend pixel and receiver/lifetime contracts are separate from dispatch arguments. |
| RS.world-kind8-virtual | Known constructors, field writers and lifetimes | Known draw wrappers do not recover arbitrary classes or full simulation. |

## Consumer envelope: prospective domain

The owned consumer model preserves a closed immutable queue and source facts;
backend callbacks cannot mutate queue count or global configuration. The declared
comparison covers 49,248 matrices of 0/1/3 rows, both video paths and dimensions,
padding, signed clipping markers, kinds -2 through 35, incoming metadata/Y
mutation, backend AX returns, concrete helper receiver tokens and finalizer
argument/reset ordering. The original consumer instruction bytes remain intact.
Raster and finalizer bodies are recording boundaries; the original clip setter
executes from a byte-identical private copy. This is not complete raster pixels,
object construction/lifetime, arbitrary backend mutation or live replacement.

Findings and execution results are recorded below as they are established.


## Executed results and confidence

All results below use the pinned No-CD build, private fixtures/prefixes and
manifest checks before and after original execution. Confidence is high only
inside each stated domain. Reports are copied unchanged into
`research/runtime/critical-original-recovery-results-20261020/`; raw binaries,
logs, source declarations and failed attempts remain pinned under
`working/tests/critical-original-recovery-20261020/` and the named capture run.
Original pixels never become native renderer input. No replacement is enabled.

| Evidence | Observed result | Explicit boundary |
| --- | --- | --- |
| REC6.consumer.original-20261020 (`consumer.json`) | 49,248 cases agree; 168,616 events, 68,400 clip updates and 24,624 finalizers; original consumer instructions unchanged. | Concrete helper receiver identity and arguments are compared, while raster/finalizer bodies remain external recording boundaries. Immutable queue/configuration domain only. |
| REC6.kind8.original-20261020 (`kind8.json`) | Six constructors and actual nondeleting virtual destructors agree in 2,592 cases, including all 663,552 object bytes; five unsupported native inputs refuse atomically. | Zero RNG stream, above-terrain inputs and no owned colour plane. Original allocator/deleting free, terrain-cache lookup, other constructors/classes and live field writers remain unrecovered. |
| REC6.startup-lease.live-20261020 (`startup-lease.json`) | Actual initial Blt, completion query, added read-only WAIT Lock and Unlock all succeed; canonical IUnknown identities agree; all 480,000 RGB565 pixels are zero. | One original map2 startup through first World, with explicit added QI/Lock intervention. Wrapper creation count is not a physical allocation generation; no loss/Restore, all-writer, timing or alternate-buffer proof. |
| REC6.driver-recovery.original-20261020 (`driver-recovery.json`) | Four system/video-memory cohorts execute actual original create/fill/Lock/release. Both lost video Lock paths Restore and retry successfully; 384 explicitly filled pixels match; all four original releases clear wrapper storage and empty the registry. | Wine built-in DirectDraw, 8x6 RGB565, private caller-owned wrappers and forwarding interface proxies. Blt succeeds despite IsLost; a real fill SURFACELOST branch is not observed. Restored undefined pixels are diagnostic only. No physical hardware or full game ownership proof. |
| REC6.static-20261020 (`static.json`) | Alternate global 0x6e1f68 starts zero in the PE zero-fill section; five direct loads and no direct stores were found. Six selected class tables, direct constructor callers, assignments and exact constructor bodies are retained. | Static disassembly cannot exclude indirect/address-derived writers or prove unreachable alternate buffers. Uninventoried assignment/reference ranges and unknown classes remain explicit. |

The queue comparison corrected two details: the auxiliary prelude changes clip
**bottom**, and the metadata prelude mutates Y by `oldY + sourceHeight - marker`.
The earlier expectation was wrong; the final prospective run tests the corrected
model. The constructor fixture reserves a verified low padding region in its own
PE32 image before mapping original sections, so Wine's executable mapping cannot
silently occupy the original addresses. It leaves original instructions intact.

The real driver probe also found original creation mutates the selected video
caps from 0x4040 to 0x10004040. The real driver's successful Blt on a surface whose
IsLost reports lost means an IsLost result cannot substitute for the actual Blt
HRESULT. The following Lock returns SURFACELOST, Restore succeeds and its retry
succeeds. Those paths are recorded separately from scripted error controls and
native hard restart policy. The original wrapper retains no outstanding lease
when its original release function is invoked.

Setup failures, callback stack alignment and incorrect early comparison
expectations are retained in consumer-1 through consumer-4, kind8-1 and
real-driver-1. They grant no validation credit. Final complete denominators are
consumer-5, kind8-3 and real-driver-2. Kind8-2 remains unchanged as the earlier
comparison; the repeated kind8-3 result also retains entry-byte anchors for three
constructors outside discovered function bodies. Explicit recovered-range links
acknowledge that inventory gap without changing the binary inventory. The original startup observation is
`working/experiments/scene-observer/run-f_bqvdzq`; the existing 12-hook forwarding
ABI check also passes in `working/tests/canvas-startup/run-awrvlqq6`.

## Remaining recovery, separated from validation

| Critical parent | Recovery supplied here | Still needs original recovery |
| --- | --- | --- |
| NR.directdraw-retirement | Real first-startup aliases/lease and isolated original wrapper loss/Restore/release. | Whole declared caller/indirect-interface inventory, physical backing generations and every writer/alias lifetime. Native failure/restart, full journeys and sustained hardware performance remain separate validation/acceptance. |
| RS.canvas-create-lock | Real create aliases and held lease; real Wine original Lock/Restore/retry and release/registry cleanup. | Physical backing identity/generation, complete aliases and live writer intervals, other real driver outcomes and caller/failure domains. |
| RS.canvas-bind-clip | Alternate direct references/initial zero state and original consumer clip envelope. | Reachable live alternate allocation, its creation/clear/writer interval and indirect writers. Five reads with no direct store cannot close this case. |
| RS.canvas-startup-clear | The named first-startup actual completion and full leased pixel observation is now supplied. | No new original code is needed for this successful first-clear observation. Independent native/current-source comparison and renewed startup/failure profiles remain validation; shared physical ownership remains with create/retirement. |
| RS.world-dispatch-targets | Full bounded queue/clip/prelude/postlude and receiver-identity envelope. | Concrete downstream helper/raster/finalizer effects and complete pixels, mutable queues/object lifetimes and other supported consumer domains. |
| RS.world-kind8-virtual | Six selected default constructors, untouched/default fields and nondeleting lifetimes. | Nonzero RNG and its seeding, below-terrain branches, allocating/deleting/colour-plane lifetimes, other constructors/classes and actual update/writer intervals. |

The broad parents retain partial implementation and their independent milestones.
New bounded children carry only the fresh execution they actually passed. The
startup-clear next-work label moves from recovery to validation because its
specific unsampled first-clear case is now observed; this does not assert full
DirectDraw retirement. No claim is made that all six broad contracts are closed.

Shared observation-source edits leave earlier wide source fingerprints stale.
Their immutable reports and historical statuses are preserved. The accounting
receipts name those pending limitations rather than refreshing old hashes or
claiming the new local tests validate every linked subsystem.


## Dashboard historical-review handling

A real dashboard Refresh failed because the six extended contracts now contain
source paths absent from their immutable historical implementation reviews. The
review loader now treats those added inputs as explicit historical drift when
the contract identity changed. It preserves the old record and grants no new
source review or execution credit. An incomplete review of an unchanged contract,
unpinned authored paths, invalid witnesses and corrupt hashes still refuse.
The new regression exercises source growth without modifying the register,
original review bytes or registered implementation state.
