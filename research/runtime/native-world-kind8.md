# Kind8 additive object drawing

The No-CD40209ca7 build has a concrete observed class at vtable0x5c7420,
method slot+0xc=0x54ab00. A frozen map1/item15 read-only capture observed432
rows across10queues with that pair and RGB565 format0. The observer retains
row bytes,80committed readable object bytes and five vtable slots; pointer
values are diagnostic identities within that launch. It does not modify them.
Historical capture: `working/tests/kind8/run-le6ejqbj/source/working/experiments/scene-observer/run-07m07xu_`.

## Recovered method and boundaries

Method0x54ab00 writes byte object+0x44=1, advances ECX to object+0x30 and
calls0x545c10 with stack x/y. The shape fields are width,height,red,green,blue
DWORDs at+0x30..0x40. The raster clips a rectangle to active signed
left/top/right/bottom globals and adds each low16 increment to the existing
RGB565 channel, wrapping to16bits before unsigned saturation at31/63/31.
It uses the active destination and stride. This is confirmed within the
selected build/raster; object construction, lifetime, simulation and writers
remain unrecovered. The alternate RGB555 branch remains outside native scope.

The capture allows only this readable class/method with nonzero consumer mode,
format0 and row+28=0x8ad08ad0 (both auxiliary markers inactive). The third
dispatch table also has a virtual path but is not admitted by this change.
The middle kind8 table invokes an auxiliary routine; it is not the same
virtual draw path. Other class pairs, modes, markers and formats retain refusal.
The original method and drawn-byte side effect still run normally.

## Native path

A separately checked hook at0x545c10 observes effective shape/clip/x/y and
serializes operation6/backend11 in the pointer-free closed World record.
No-op shapes produce no record; bounded positive shapes retain original order.
The native worker carries these primitives without SPR lookup or decode.
Scene admission validates the whole list before drawing; malformed primitives
preserve the preceding complete frame. The GPU freezes its owned destination
for the primitive and performs integer addition through the composition shader.
Normal presentation needs no native CPU readback or original destination seed.
The whole-canvas GPU snapshot per nonempty rectangle is a bounded correctness
policy; regional snapshots/batching are a later performance improvement.

## Validation

`tools/test-kind8.py` tests read-only diagnostics through the actual PE32 queue
hook, including unreadable objects/vtables, truncation and malformed refusal.
`tools/test-kind8-raster.py` declares sources/contracts before execution,
tests the asset-free preparation path and strict records, and compares ordered
clipped rectangles plus every RGB565 destination WORD with seven increment
sets against unmodified0x54ab00/0x545c10 in a separate mapped PE32 process.
Native backgrounds are constructed independently; before files are supplied
only to that private original comparison. Fresh results are registered after
their source-stability checks. Live full-queue capture/comparison is a separate
validation boundary. This does not claim full kind8, full rendering, object
simulation or live original replacement.

## Additional observed classes

A later frozen map1/item15 capture admitted a1497-request full queue after
tracing the following particle paths. Its read-only diagnostics also retain
later classes as observations, separate from that sampled queue.

| Vtable | Method | Selected pixel producer |
| --- | --- | --- |
| 0x5c7420 | 0x54ab00 | Additive rectangle at0x545c10; flag+0x44 |
| 0x5c73f0 /0x5c7408 | 0x546540 | Conditional additive rectangle at0x545c10 |
| 0x5c7494 /0x5c74b0 | 0x54ab60 | Owned RGB channel rectangle at0x547d80 |
| 0x5c74cc | 0x54ac00 | No pixels; sets flag+0x40 |

The conditional additive wrapper reads object+0x60 and selected player state;
its original gate still runs. Only reached545c10calls become native records.
The colour wrapper advances to object+0x1c, then invokes547d80. Shape fields
are mode,width,height DWORDs,+0xc/+0xe/+0x10 RGB channel WORDs, and an
unaligned optional plane pointer at+0x12. Plane sources contain three WORDs
per pixel. Capture normalizes valid31/63/31channels into owned RGB565 WORDs.
Uniform sources expand into the same pointer-free plane. Selected modes are
0copy,1quarter-destination,2half,3quarter-source. Shapes are deliberately
bounded to64x64, channel values to canonical RGB565 ranges; larger or invalid
visible sources refuse the complete frame. Native GPU colour planes are
transient and released after each draw. Their uploads are bounded per shape
and execution remains limited by the scene's draw/time budget; these transient
upload bytes are not included in the SPR upload-budget counters.

The flag-only class contributes no raster record. Original execution preserves
its state change; native presentation claims no equivalent object simulation.
Unknown class pairs remain refused even when their draw kind is numerically8.

The original comparison now tests every RGB565 destination WORD with all four
colour modes, uniform and varying sources, plus mixed/clipped primitive order.
It independently exercises enabled/disabled owner/player additive gates and
flag-only field/pixel preservation. Both primitive hooks have active-state ABI
checks. Full captured replay starts the native canvas from its own zero base;
a separate private original replay receives diagnostic before pixels only to
check that the captured requests explain live output. Live/native equality
and equality from the same independent base remain separate recorded facts.

Registered final evidence: [original/native validation](native-world-kind8-validation-final-20261008.json)
and [frozen live observation](native-world-kind8-discovery-20261008.json).
The final comparison reports1048960synthetic pixels and480000captured queue
pixels equal from independently owned matching bases. Native zero differs
from the live before-initialized canvas; no live equivalence or replacement
status is promoted. The initial1497-request capture used the earlier
four-class admission; later diagnostic classes and aliases have isolated
original/active-hook checks in the final source-bound comparison.
