# Protected source writer tracing

`--source-write-trace` records registered canvas-cache ranges at acquisition of
an existing diagnostic read-only source lease. The selected 800×600 RGB565
source stays protected through its exact emitted workers and paired external
snapshots. Direct CPU stores through the protected virtual pages stop with
exit 99. This is intrusive diagnostic execution with original drawing active.

Each acquisition records all registered caches, including unknown layouts,
relative offsets, geometry, pixel-span overlap, page overlap and shared virtual
reservation. A shared reservation is not a shared allocation. Pixel spans are
bounding intervals and can include row padding; fault classification also
checks the actual pixel columns of each recorded cache. The cache snapshot is
historical metadata, without a claim that it is still current at fault time.

The fault handler records the writing thread, target offset, lease phase and
PE image fields, instruction RVA and 16 instruction bytes when readable.
Offline attribution requires a unique supplied image matching those fields
and exact bytes. Missing or ambiguous matches remain unresolved. The handler
never takes the producer recorder lock or reads mutable cache tables, and no
faulting store resumes. The legacy fault record remains available.

Lease metadata and optional trace acquisition are published before native
page protection changes. A write fault after protection but before return
therefore remains visible as an arming-phase fault. Separate immutable armed
and restored markers distinguish admitted intervals from attempted or faulted
ones. Closed capture analysis requires exact API/lease/source/owner bindings,
completed packet workers, stable paired source snapshots and every marker.

Protection applies to virtual pages, so the page-alignment halo can stop a
write outside the logical source pixels. Another mapping of the same backing
can bypass this view's protection. GPU completion, unrelated virtual aliases,
complete original allocation lifetime and all source writers remain outside
this milestone. Owned and original validation results will be recorded below.

Primary API references: [VirtualQuery metadata](https://learn.microsoft.com/windows/win32/api/winnt/ns-winnt-memory_basic_information)
and [page protection](https://learn.microsoft.com/en-us/windows/win32/memory/working-with-pages).
These define API scope; installed Wine behavior is validated separately.

## Protected source writer tracing — 2026-10-13

The writer tracer snapshots every registered
canvas cache before a selected diagnostic read-only lease and records a faulting
CPU writer's thread, normalized PE instruction identity and target offset. It
keeps pixel-span overlap separate from page-alignment overlap. Fault handling
uses the frozen cache metadata, takes no recorder lock and never resumes the
store. Lease metadata is published before protection changes, covering a write
fault during arming. Default tracing remains off.

[Owned fixture 01](draw02-source-write-trace-fixture-01.json) passes 12 actual
PE32 cases and 10 decoder tests. Equal/partial aliases, both page halos and a
write during arming stop with exit 99; all six traced faults resolve to exact
fixture instruction bytes. Unrelated same-reservation writes, normal restoration,
invalid flags and default-off behavior pass. The existing
[lease regression](draw02-source-write-trace-lease-regression-01.json) also passes.
Two freestanding build failures and a no-display launch failure remain separate
negative records; none establishes an engine failure or current validation.

[Original run 02](draw02-source-write-trace-original-02.json) closes 139 protected
source pairs through 264 exact worker commands, with byte-identical before/after
planes, 136 complete advancing updates and normal original Quit. Immutable
manifests and original sections remain unchanged. Six source cache IDs use
MEM_PRIVATE storage with 48-byte offsets inside their protected first page.
Known cache spans and reservations show no overlap during those leases.
The 2,154 unknown layout entries cover 49 cache IDs, each lacking a recorded
pixel pointer. They are retained, without an allocation or alias classification.
[Fresh source/worker/EGL reanalysis](draw02-source-write-trace-egl-analysis-01.json)
matches 352 whole images containing 168,960,000 RGB pixels.

The owned second-mapping case changes the leased source word from 0x1111 to
0xabcd through another view of the same file mapping. Protection on the first
view does not stop it. This confirms the limit of the page guard: complete
original backing provenance, other virtual aliases, GPU consumption and
unobtrusive acquisition remain open. Original source atomicity, native parity,
full 47-journey/21-operation acceptance and steps 3–7 are not promoted.

The b030d7fc..9a5f14b6 committed range was reviewed against 38 exact file
receipts with zero gaps. Source census, preserved evidence fingerprints and
exact current review accounting accompany this increment.
