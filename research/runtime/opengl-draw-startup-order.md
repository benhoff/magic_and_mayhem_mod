# DirectDraw factory startup admission

Continuous-only opt-in `MNM_RENDER_ORDERED_COPIES=1` now admits
DirectDrawCreate through the existing drawing callback lease. The lease spans
RenderStartup, saved-original factory execution and successful returned-interface
installation. This is intentional native scheduling policy, not recovered driver
ordering. Ordinary failed and null-output originals release admission through
scoped cleanup. Original arguments, HRESULT and LastError remain unchanged.
Startup refusal does not prevent original application creation. The existing
50ms admission/8ms tracker budgets and same-thread reentry remain; no tracker is
held while waiting or executing original code.

Timeout forwarding calls the original once, resets uncertain alias provenance
and metadata/pixels on both sides, and leaves the returned interface uninstalled.
The diagnostic records CREATE_FAILED for an original failure, or
INTERCEPTION_FAILED for an unobserved successful result. It does not declare a
returned interface intercepted because its table happens to have been observed
before. A later independently admitted observation is needed for unseen outputs.
Initial startup is attempted only on admitted calls. Failed startup never becomes
permission to invent an initial checkpoint.

The mapped channel still supplies session identity and its versioned claim;
RenderStartup checks producer/transport/capture readiness and starts workers
outside DllMain. That return value does not prove consumer readiness. The existing
application host normally binds the consumer first. A delayed consumer can bind
a claimed empty ring after factory startup, before the producer obtains complete
initial pixels. No resource or incremental packet is emitted by factory creation
alone. The first complete admitted primary Unlock supplies the initial checkpoint.
Factory creation during an existing session does not reset its resources or IDs.
Attachment to already active drawing still requires the separately implemented
complete owned checkpoint contract.

## Validation

The independent first-factory fixture calls no RenderInstallForTest. Returned
DirectDraw1 CreateSurface and surface hooks must be installed by actual saved
factory callbacks. It uses108-byte descriptors and pointer-based Unlock, owns
native fixture buffers independently and poisons exposed rows after original
Unlock. A failed CreateSurface followed by success verifies forwarding and input
ownership when the original poisons the descriptor after consuming it.

- First, delayed-reader, failed HRESULT and null-output retry cases establish a
  complete primary checkpoint, publish a distinguishable update and retire it.
- Nested creation reenters admission on the same thread. The inner admitted
  interface creates a primary and publishes its complete pixels before the outer
  factory returns. A subsequent update and shutdown retain ownership.
- A worker's failed, null and successful factory calls wait for an admitted
  primary Unlock. The original worker marker must remain clear until the outer
  original finishes. Existing session/resource identity survives the factories.
- Forced timeout makes the outer original wait for the worker, forwarding every
  factory result while leaving its new vtable uninstalled. Only the initial
  native presentation survives; pending update refusal and terminal zero storage
  are required.

Factory call counts/results and frame diagnostic fields, exact independent RGBA
hashes, original Lock/Unlock/Release counts, monotonic resource identity,
CREATE/DELETE/END versus GAP, admission diagnostics and consumer storage/readback/
upload cleanup are checked. Consumer readiness is explicit fixture orchestration;
its bounded file rendezvous is outside the hook and changes no production policy.

Original game/driver execution is not rerun here. Original enumeration/load-time
ordering, other creation entry points, delayed attachment during active drawing,
unobserved returned-interface recovery, startup versus externally invoked
shutdown/recovery races, prolonged or terminated owner threads, borrowed CPU/DC
intervals and default enablement remain pending. Historical evidence is retained
without source-hash refresh or new original equivalence claims.

Exact committed history `9e8868e..f86d3fd` was reviewed against parent/current
file/behavior receipts with no unresolved entries, preserving old receipts and
relative order. This accounting review does not assert past gate passes or new
validation of those versions.

Fresh [factory evidence](opengl-draw-startup-order-native-20261006.json) passes7
cases/13 complete frame comparisons (six valid, one deliberately refused).
The [lifecycle regression](opengl-draw-startup-order-lifecycle-20261006.json)
passes11 cases/8 complete frames; the [late checkpoint regression](opengl-draw-startup-order-checkpoint-20261006.json)
passes6 cases/186 complete frames across complete, indexed/recreated and
incomplete/borrowed-state paths. Production and SELFTEST PE32 DLLs build.
