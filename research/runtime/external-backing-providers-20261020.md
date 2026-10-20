# Explicit backing providers for immutable COPY handoff

The user selected explicit leases/fences for shared, file-mapped, imported and
GPU backing. The observer now has a process-local versioned provider API in
`runtime/drawing/backing_lease_v1.h`, exported by `MnmDrawing.dll` as
`DrawingRegisterBackingV1` and `DrawingUnregisterBackingV1`. These structures are
local callback objects, not shared-memory/wire layouts. The adapter still uses
the pinned Wine 11.16 resource layout and selected bounded RGB565 COPY path.

A provider must recognize the exact resource/subresource and texture generation.
Successful acquire supplies a stable CPU-readable span, row pitch and extent,
object identity, object generation, a monotonically increasing token, and a
producer fence/completion pair. GPU fence zero is refused; completion must be at
least the required fence. Fence completion does not exclude future writers:
the provider separately holds read ownership excluding every cooperating CPU,
process, imported alias and device writer, and pins the storage lifetime until
release. A pointer, a successful map or a completed fence alone is insufficient.
The provider owns enforcement through its producer's actual locking/lease/fence
mechanisms. Unknown and uncooperative producers remain refused.

Callback modules are pinned using GetModuleHandleExA with FROM_ADDRESS before
registration publishes the copied API. Callback addresses must belong to committed executable image pages. All five callback modules stay retained
until unregister; unregister refuses a callback or active lease. IDs cannot be
reused during a recorder lifetime, including after unregister. Registration is
bounded to 64 lifetime IDs and handoffs retain the existing 2,048-submit bound.
Providers must register after initialization, outside loader callbacks, and may
not recursively submit a COPY. Tokens cannot wrap or repeat within a provider. A separate bounded table retains 2,048 lifetime provider/object identities; generations cannot go backward or change while older reads are active.

The actual handoff path acquires this provider lease before copying. It copies
only defined pixel rows to the existing private allocation, seals that allocation
read-only, and asks the provider to transfer the reference texture binding.
Before admitting consumption the adapter checks actual Wine fields: private
resource flags, no BO, no resource heap, system-memory-only subresource location,
invalidated GPU locations, the exact private client pointer, and unchanged pixel
CRC. A provider's success return cannot substitute for these postconditions.
The original worker/context/fence and physical-stage protection remain required.
Restoration invokes provider restore and verify callbacks while read ownership
and storage/module lifetimes remain held. Release follows stage retirement and
restoration. A failed callback/postcondition terminates strict admission rather
than returning a partly transferred binding to the caller.

The private allocation path retains its page-protection/callout-free snapshot
logic. External providers exclude original writers through their own lease;
the adapter does not pretend that protecting one mapped alias excludes the
others. Only the selected provider's binding-transfer writer on its own thread
can pass the shared-resource guard during the bind/restore phases. All ordinary
stage mutation/retirement guards continue through restoration.

The pointer-free journal uses MNMSHF02; its decoder still accepts MNMSHF01.
See `research/formats/source-handoff-v2.md`. Journal validity checks provenance
shape, stable identity, token order, required/completed fences, and the existing
transfer/fence/restoration order. It does not establish that an arbitrary external
provider actually implements its promised writer exclusion. Each installed
producer requires its own reviewed implementation and execution evidence.

The reproducible control runner is `tools/test-drawing-backing-providers.py`.
It exercises the actual PE32 observer and real aliased Windows section mappings
for shared/file/imported classes, with a controlled GPU readback/fence producer.
It checks malformed registration, active unregister, all four backing kinds,
incomplete/zero GPU fences, incomplete guarantees, stale tokens, mismatched
kind/generation/pitch, unknown-provider refusal, and bind/restore failures.
Controlled producer/GPU dependencies do not constitute a hardware-driver or
full original-session comparison. Broader production acceptance stays partial.
