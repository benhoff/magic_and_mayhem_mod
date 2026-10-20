# Concrete empty kind8 draw methods

Pinned static vtables `5c7438`, `5c7450` and `5c7470` bind slot `+0xc` to
`54ac30` or `54ac40`. Both complete six-byte bodies are `66 33 c0 c2 08 00`
(`XOR AX,AX; RET 8`), without object loads/writes or raster calls. Confidence is
high for these exact bindings and method bodies in the No-CD build. The two
bodies were absent from the retained discovered-function inventory; explicit
six-byte recovered ranges preserve that inventory and its unresolved ranges.

A shared build-specific C matcher now serves runtime observation and the owned
method model. It recognizes nine exact vtable/target pairs and refuses unknown
pairs. Conditional first-table RGB565 observation still requires inactive
auxiliary markers. The three empty classes need a readable four-byte vtable
pointer and the first sixteen table bytes. The six existing pixel/flag classes
retain their prior 72/100-byte object guards. No new wire operation is needed.

`WEMPTY.original/native-20261020` retain 1,440 original/native method cases,
including 480 empty-class cases. All three empty bindings make no raster calls,
preserve all 26 compared object DWORDs and return low16 zero. Downstream raster
calls in this method runner are synthetic stubs, so this is method/state proof,
not original pixel proof. Native model refusal controls, three native CTests and
actual PE32 hook ABI checks pass. A four-byte object at the end of a readable
page, followed by an inaccessible page, passes only the exact empty-class gates;
a pixel class, changed target, wrong mode/format and active auxiliary markers
refuse. This protected-page test exercises production admission; it does not
execute the original complete queue with that object. Upper EAX and CPU flags
are outside the recovered return contract.

Fresh `K8EMPTY` regressions retain the read-only colour probe, 17 primitive
fixtures/1,048,960 original pixel comparisons, and a new 1,361-draw World trace.
All 480,000 native zero-base, private-original zero-base and captured-after WORDs
agree. Removing nine direct-word records produces 531 mismatches. The strict
historical 1,505-draw reconstruction still matches independent native/original
zero-base pixels and separately original-prior/captured-after pixels; its native
zero-base full canvas still lacks earlier history. Original before/after output
never seeds native rendering. The live class samples contain only the already
known colour and flag-only pairs; natural empty-class reachability is unproved.

Constructor-like literal stores for these tables are static candidates only.
Construction/lifetime, caller writers, other virtual slots/classes/table modes,
full machine state and complete live rendering/release gates remain separate.
The broad kind8 and World scopes remain partial, and full live replacement
remains none.
