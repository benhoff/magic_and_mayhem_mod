# Actor emitter ticks and manager ownership (2026-10-22)

Compose original546ce0 with545910 reverse manager updates, the unchanged
interpolating-emitter and child/motion kernels, actual493f20 creation, real
5457e0 admission and546270 deleting child retirement. The emitter inlines its
owner-additive child factory through545650/545b70;548650 is a different
terrain-colour constructor and is not substituted here.

All particles use distinct100-byte allocations with checked canaries and live
manager prefixes. Native kernel copies publish parent/child bytes and the R250
stream before allocation, insertion and destruction callbacks. This preserves
callback-visible ordering. Input R250 successor tables use the canonical ring.

Independently authored inputs cover32 ticks, owner presence/activity/axes and
caller rebind, stage interpolation/retirement, child delay/fading/motion, RGB
formats, allocation failures, real manager capacity rejection, and complete
1792-object prefixes. Separately named owned callback controls exercise nested
insertion, self eviction, slot reload and compaction; they do not claim recovery
of an original callback body. No fallback frees may hide remaining ownership.

The older read-only Ghidra project lacks functions546120/546350. Both already
exist in the pinned central inventory; complete decoded ranges and original
execution supply the evidence without project/inventory mutation. Static
inspection failures and frozen prospective execution attempts are retained.

Fresh `emission-matrix-2.json` passes1024 independently authored cases,32 ticks each (32768 manager ticks/build), and7168 snapshots per original32/native64/ASan+UBSan build. All three streams compare 3948080688 identical bytes, SHA-256`a38a22b6dab4f3aa200cfabe2a7605b32c0a7e7a0a26e1b23b8f2d89195b5f67`. All85 source/compiler inputs are stable; protected original bodies and original manifests are unchanged. Borrowed owner/descriptor input is unchanged by engine calls, allocation canaries hold and actual manager/deleting cleanup leaves zero residual owned generations, without fallback frees. Confidence is high within this bounded domain.

The fixture initializes six complete1792-object pool prefixes across emitter, manager and actor cohorts. The logged `full` selection flag also occurs in two child-only cases, which have no manager prefix; the aggregate flag count is not a pool count. Per-build outcomes are `{"control": 128, "created": 3419, "evicted": 2, "failed": 312, "full": 8, "nested": 128}`. Two manager controls exercise self eviction; actual emitted children exercise capacity rejection. The explicit synthetic updater remains separate from recovered original behavior.

A failed small comparison established that an inactive but present owner still supplies descriptor/position facts used to bind emitted children. The adapter now reads these facts for every present owner; inactive state separately gates following. Trial2 and the full matrix validate that correction. Historical attempts retain their exact inputs and errors. Other emitter
families, arbitrary callback mutation, malformed/throwing/SEH paths, original
world/planner production and installed/live/physical acceptance remain separate.
The Critical actor umbrella remains partial.


The final comparison and inspection4 run at reviewed local mastera5ab8270.
Incoming8bf5e736..a5ab8270 contains57 exact reviewed transitions across three
commits, with no missing or rewritten receipts. Optional evidence publication
changes only `tools/coverage_evidence.py` among the85 declared inputs; the other
84 inputs and scope/scenario remain unchanged. Matrix2 repeats original/native/
sanitizer execution and reproduces matrix1's complete byte stream. Matrix1,
inspection3 and the earlier incoming-history report retain their original
fingerprints; their helper-dependent evidence is historical after this merge.
New merged execution/inspection/history IDs carry current-source support.
The83 merged dashboard checks pass; focused Critical/HTTP/client checks rerun
after the final parent assessment fingerprints are updated. No public upload
is needed for local completion or its private backup.
