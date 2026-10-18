# Private COPY ownership and physical aliases

The opt-in pinned Wine11.16 policy uses `--wine-physical-owners` and
`--wine-source-handoff` alongside COPY reservations. Each submitted reference
COPY receives a separate private value, retained through its exact packet,
worker and signaled current-context fence. The original texture binding is
restored before the intercepted API returns. Original pixels remain comparator
inputs; this does not feed them into the native renderer.

This is intentional allocation and synchronization policy. It is not recovered
original allocator behavior or a global host/driver physical-memory census.

## Allocation and transfer

Wine system-memory preparation at RVA `0x204a60` uses a separate zero-filled,
page-aligned VirtualAlloc allocation for each resource, preserving the original
16-byte prefix and alignment. Resource size, heap pointer and pixel pointer are
at `+0x4c`, `+0x54` and `+0x58`. The free import at `0x3396b0` handles ordinary
and compiler-inlined cleanup. Allocation/free hooks remain paired after recording
closes. General heap allocation bodies are unchanged.

Client memory must belong to a live allocation observed through pinned
RtlAllocateHeap/RtlReAllocateHeap/RtlFreeHeap/RtlDestroyHeap entries. Requested
extent, heap identity and monotonic allocation generation are tracked. Exact
lookups use bounded open addressing with backshift deletion; collision controls
cover wrapped probe chains, deletion and generation reuse. Containing-span
queries are independent of resource identity. Heap bodies execute outside the
drawing journal lock. Free/realloc completion retires the captured allocation
generation, preserving a newer allocation that reuses its address before the
older hook returns. Heap destruction likewise retires only generations present
at entry. Deterministic controls exercise all three return races. Equal VirtualQuery allocation bases alone prove nothing
about physical aliases.

After an additional READ map returns, acquisition drains observed heap, VM and
CS operations, dropping the journal lock while waiting. It resolves the live
source again, admits only committed anonymous private guest storage, and freezes
its pages across a callout-free row copy into a new private buffer. The original
page protection is restored immediately. Pixel writes that fault during this
interval fail the capture; another thread's heap metadata/padding access can
wait for the thaw. Unknown, shared/file-mapped/imported/BO storage, unsupported
geometry, multiple subresources, swapchains and active DCs refuse.

After READ unmap, the saved public `wined3d_texture_update_desc` binds the private
buffer to the same logical texture. This invalidates old GPU locations. Admission
checks SYSMEM-only location and absence of allocated RGB/sRGB texture flags.
This is a fresh private source epoch, rather than an assertion that a historical
GPU allocation was disjoint. Public map/unmap/copy/rebind entry guards and cached
GetDC/releaseDC entries prevent exposing or changing the private binding. The
GetDC/releaseDC pinned RVAs are `0x218a80` and `0x220d30`; both checked prefixes
are `55 89 e5 57 56 53`.

The private buffer's page lease lasts through the exact consumer. All 51 pinned
CS dispatch slots at `0x24d420` are wrapped. Reviewed state/read/disjoint writes
may proceed; unreviewed callbacks, draws/dispatch, BO maps and unsupported
writers refuse while a lease exists. Writer checks include every client
subresource and overlapping backing ranges, including different resource
objects. Guest protection/free/write/reset/discard/unmap routes cannot remove
protection or retire held backing; AWE remapping refuses.

After the signaled fence and worker completion, restoration reattaches the exact
old client pointer or original Wine allocation. The paired renderer allocator
only defers the old Wine allocation's cleanup during this binding transition.
Original client allocation retirement is pinned. The private stage is freed
only after restoration, before the outer API return. Old aliases are therefore
allowed to change the old storage after transfer: they no longer name consumer
inputs. Controlled retained-pointer and distinct-resource alias writes check
this property directly.

## Lock order and failed experiments

A read-only thread snapshot of retained diagnostic `run-zss60fsv` established a
lock inversion: thread 436 owned the drawing journal and waited on the scene
producer lock; thread 396 owned the scene lock and entered an observed heap call
that waited on the journal. The heap registry contained 6,635 live allocations,
not a capacity overflow. Lease and held-snapshot scene getters now execute
outside the journal lock. An owned threaded fixture executes all four getter
interleavings; deliberately restoring the inverse order must trip its watchdog.

Normal stop defers recorder closure while a CS callback remains active. This
keeps an unrelated callback return from arriving after its physical journal
terminal. A busy-close regression exercises the common API end path. An opt-in,
pre-opened `drawing-fatal.bin` records the source file, line, thread and exit code
for an observer fatal path; it remains empty in a successful capture.

The earlier long read-only lease on borrowed heap pages is not a live admission
route: pixels can share pages with allocator metadata. Retained failures include
an offset-4 page fault; that is not evidence of a pixel writer. The separate
value handoff avoids retaining those shared pages across Wine/heap callbacks.
A source-stable retry after the lock fix completed 2,171 handoffs but only 30
source updates before its requested-120 timeout. It remains negative evidence,
not a completed interval. Exact heap lookup was subsequently changed from a
full-table scan to a bounded hash lookup; final execution evidence must match
that code.

Automatic approval review rejected a broader proposal to replace general heap
allocation with custom arenas. That proposal was never executed. The implemented
per-submit renderer binding preserves the original general heap bodies.

## Wire and admission evidence

`MNMPOW02` uses a 64-byte header and 128-byte records for allocation, acquisition,
release, free, refusal, CS entry/return and client-generation binding.
`MNMSHF01` uses a 64-byte header and 256-byte records for preparation, freeze,
thaw, private binding, completed consumer, restored original binding, freed
stage, terminal and refusal. No process pointer enters either contract.

The independent handoff analyzer checks unique logical/physical generations,
source geometry and CRC, raw source payload hashes, complete denominators, and
this physical-journal order for every submission:

```
thaw < acquire <= bound <= release <= consumer_done <= restore < free <= retired
```

Reservation acquisition/release ordinals have explicit `reservation_` prefixes
in the combined binding. They must not overwrite physical-journal acquisition
or release ordinals; a regression deliberately separates the two clocks.

The physical analyzer independently joins raw reservation, packet and GPU journals,
including worker/release thread identity. Failed/incomplete journals and mutated
joins refuse. A positive report requires an empty fatal fault artifact. A report
boolean alone cannot manufacture ownership.

The PE32 ntdll SHA-256 is
`6e09e71463d50fcc92259e1add310818e21628bc0a9a706f4e0c0f92540071d0`.
Both installed and prefix images are pinned; WOW64 uses the PE32 syswow64 image.
Every detoured site has a checked RVA and whole-instruction prefix before
mutation. On-disk Wine and original game files are not patched.

## Boundaries

Private-value ownership is distinct from closing all writers to the original
backing. The broad `all_original_backing_alias_writers_closed` gate remains
false; the handoff intentionally stops using that backing for its consumer.
Shared/imported or unknown storage, arbitrary host mappings, direct external GL
writers, malicious observer bypass and a complete driver allocation census
remain outside admission. The current-context fence does not prove arbitrary
other-context completion. Global presentation-route closure and full native
render replacement also remain separate and unproved.

Primary Wine11.16 implementation references are
[resource.c](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/dlls/wined3d/resource.c),
[texture.c](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/dlls/wined3d/texture.c),
[cs.c](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/dlls/wined3d/cs.c),
[heap.c](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/dlls/ntdll/heap.c)
and [virtual.c](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/dlls/ntdll/unix/virtual.c).
These leads supplement the installed-build byte checks; they do not replace them.


## Current execution evidence (2026-10-18)

The opt-in pinned Wine11.16 path now transfers each submitted COPY into private
backing, leases that value through its exact worker/current-context fence, then
restores the original client pointer or Wine allocation before API return. Old
physical aliases can modify the old storage after transfer without modifying
the consumer's private value. Shared/imported/BO/unknown backing and active DCs
refuse. General heap allocation bodies remain unchanged. See
[detailed contract](drawing-physical-alias-ownership.md).

The implementation covers cached public map/unmap/copy/rebind/GetDC/releaseDC
entries,51CS dispatch slots, VM mutations and generation-bound allocation
retirement. It fixes the observed Scene/journal lock inversion, exact heap
lookup cost, address reuse before old free/realloc/destroy return, and recorder
close during an unrelated CS callback. Long read-only leases on shared heap
pages remain controlled-only; live acquisition requires the private handoff.

[Controls](physical-alias-controls-20261018.json) pass36PE32 physical-memory
cases, wrapped hash collisions, deterministic retirement reuse and five mutated
raw joins. [Handoff controls](physical-alias-handoff-controls-20261018.json)
pass16cases including old/distinct aliases, delayed consumer/fence, cached DC
entry preflight and post-close forwarding. The
[threaded regression](physical-alias-lock-controls-20261018.json) exercises all
four production Scene getter interleavings and a deliberate inverse-order
watchdog failure. Eleven physical and nine handoff parser checks, the existing
19reservation/six decoder regressions, and13accounting/admission tests pass.

The [fresh original capture](physical-alias-original120-20261018.json) completes
120source queues and2,444private handoffs with normal original Quit/child exit0,
stable sources, empty fatal journals and2,927-file original-manifest bookends.
The [pixel join](physical-alias-egl-20261018.json) matches426selected EGL regions
and whole-source images at unchanged RGB tolerance1, with zero mismatches.
[First120accounting](physical-alias-accounting-20261018.json) retains2,080inputs
and332primary witnesses, with zero pending source boundaries, source-point gaps
or pixel gaps. Every selected witness joins to its exact private value in the
[independent provenance](physical-alias-provenance-20261018.json).

This closes submitted private-value ownership for the admitted COPY policy.
It deliberately does not close all writers to the old original backing. The
[unchanged strict gate](physical-alias-interval-gate-20261018.json) still refuses
that broader claim and global presentation-route closure. There is no original
allocator/cadence equivalence, arbitrary host/driver or other-context ownership,
whole-journey/native pixel equivalence, or live render replacement promotion.
The optional primitive-lifetime journal exhausted its bounded row budget in
retained retries and is disabled in the final ownership capture; this is not
new primitive-lifetime evidence. Historical results/hashes remain unchanged;
[development provenance](physical-alias-development-20261018.json) preserves
failed/superseded acquisitions and the unexecuted rejected general-allocator
proposal separately.
