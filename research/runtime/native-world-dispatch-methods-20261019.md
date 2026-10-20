# Native queue routing and kind8 method repair

The original first/auxiliary/software queue branches now have an owned native
row-dispatch model in `reconstruction/rendering/world_dispatch.hpp`. Backends
receive semantic operations and value tokens, never original host pointers.
It preserves unsigned defaults, no-op table entries, signed offsets, literal
wave amplitudes/tint channels, low-AX fallback, normalization and metadata.
Primary and auxiliary terrain bypass the direct/indexed marker test, preserve
the original vertical offset and add two only on the initial terrain call.
The auxiliary terrain fallback retains the original offset. Other auxiliary
branches use their distinct literal-one or retained-offset policies.

`kind8_method.hpp` reconstructs six exact class/method identities as owned
wrapper decisions. The observed `5c73f0/54aaa0` method is distinct from
`5c7408/546540`: both use shape+0x34, only the latter applies the owner/player
gate, and neither writes the drawn flag. `5c7420/54ab00` sets flag+0x44 before
its shape+0x30 raster call. The two colour aliases use shape+0x1c. The flag-only
method writes byte+0x40 without raster execution. Unknown pairs refuse.

The runtime's former5c73f0/546540 whitelist pair could never match the
unchanged original static vtable or the frozen discovery's actual observed
pair. It is replaced by5c73f0/54aaa0. Primitive capture remains at545c10;
format/mode/auxiliary checks remain strict. This fixes admission of the
observed inherited-additive class; it does not bypass original object methods.

`tools/test-world-dispatch.py` verifies the immutable manifest and exact No-CD
hash before original consumption and after execution. It compares2,432
unchanged-consumer cases against the native row dispatcher and960 unchanged
wrapper cases against native methods. Original raster bodies are synthetic
bindings in these tests: method/call arguments, metadata, whole object bytes
except the normalized owner token, gate/flag ordering and low16returns are the
comparison domain. The old queue fixture does not retain the concrete helper
receiver value; that value and auxiliary clip helper calls are excluded.
An active PE32 hook fixture separately tests class admission, malformed refusal,
register/FPU/LastError preservation and pointer-free primitive output. Native
CTest targets are available through the standalone `world-dispatch` directory.

Confidence is high within those matrices. The model is an offline reconstruction;
original queue traversal/prelude/postlude still run during game execution.
Arbitrary initial row/container state, concrete kind9 helpers, object writers,
unknown classes and complete raster/other-mode/live equivalence remain open.
The separate kind8 raster runner exercises the inherited wrapper against the
same native primitive pixels without changing its object bytes. Original pixels
never seed native input. Full live replacement, presented journeys, physical
fault/alias closure and sustained performance remain independent acceptance
work. The existing safe distortion DWORD-read model and native padding policy
remain separate from the uncaptured historical live crash.

Failed setup/sandbox/comparison attempts remain under
`working/tests/native-world-dispatch-*20261019`. They include refusal of a
modified working executable, sandbox32-bit SIGSYS, two terrain parameter
mismatches and the wrong vtable-pair detection. Successful results are registered
under new evidence IDs; historical reports are never rewritten.
