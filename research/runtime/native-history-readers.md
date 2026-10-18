# Post-World reader routing

## Confirmed findings

The first combined launch stopped at a facade source read, rather than a raster
blend. The version 2 CPU-authority record in
`working/experiments/menu-observer/run-6ancl2jz/game/native-cpu-history-refusal.bin`
identifies family 0/route 0, World generation 1 and caller 0x79a939b9. Disassembly of
that launch's actual `MnmDraw.dll` identifies RVA 0x39b9 as the return from
`ns_fast` to `ns_draw`. This is the native facade's BltFast source-copy path.
The family/site tags identify selected adapters; family0 identifies the facade.
Loaded addresses are launch-specific evidence, not reusable hook addresses.
Confidence: confirmed for this launch and these compiled bytes.

The source-only refusal captures in
`working/experiments/menu-observer/run-ubf548pd/game/` identify the original
WORD entry0x596cb8/interior0x596d78 (splice9) and grayscale entry0x589c90/
interior0x589cd3 (splice21). Both use a 596×140 physical plane, stride 596,
logical clip[548,90,597,139] and a 40×40 sprite positioned at[552,94]. The logical
right edge exceeds the allocation width by one, while every sprite write fits.
The source frame is 3,264 bytes, SHA-256
`c606edaedb6fa88fdef428809a980c0858046d03722573eb12b8c1e24b3106a0`.
Its row tables are contiguous and valid; they were not the admission failure.
The comparison executable is pinned to original build SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence: confirmed by captured inputs and independent original/native execution.

## Native ownership policy

`NativeDrawGpuCopyV1(dst, dr, src, sr, flags, baseline, bytes)` is a process-local
PE32 service for the ordered native bridge. It validates owned identities,
matching WORD formats, rectangles, source keys and physical leases through the
same facade implementation. It admits a stale GPU source only for a disjoint,
unclipped, non-stretched copy. It leaves CPU image allocations unchanged and
transfers generation/nonce authority to the destination. A caller must commit
the corresponding native COPY before returning to application code. Missing
admission, session/resource identity or queue publication is terminal.

The bridge now uses `NativeDrawGpuCopyPlanV1` for its GPU-source copies. It
admits current canonical clipping (up to 256 pieces), aliased/self sources and
single source keys. A self-copy freezes the entire GPU source once before all
pieces, matching owned facade CPU policy. Successful pieces survive a later
source-bounds error; their destination CPU backing remains stale. Empty clips
preserve authority. The legacy service above remains the strict full-overwrite
bootstrap baseline service. See [GPU copy plans](native-gpu-copy-plan.md) for
the scalar ABI, bounded scratch lifetime and independent controls.

The bridge uses existing command-stream COPY records, rather than introducing a
wire opcode or returning World pixels. GPU-owned resources cannot be evicted and
re-created from their stale CPU checkpoints. Flips transfer GPU ownership with
physical storage. Full un-clipped opaque CPU copies and fills establish complete
CPU authority; partial writes retain stale authority. Destination-dependent CPU
readers and DC acquisition still require current history.

For a previously uninitialized destination, only a complete opaque COPY may
bootstrap. A synthetic blank CREATE is immediately overwritten by that COPY
before PRESENT. The optional `baseline` output copies the facade's unchanged
private CPU backing into separately owned write-mask storage. Those bytes are
not rendered World pixels, cannot satisfy a CPU reader and never seed the native
image. Later locks retain that baseline solely to identify changed CPU words.
Resource limits, ordered admission, epoch checks and terminal refusal still apply.
This is intentional native policy, not recovered DirectDraw GPU/CPU coherence.

The raster kernel preserves the logical clipping rectangle. Its atomic first
visit validates every actual destination write against physical storage before
mutation. An oversized logical clip alone no longer refuses a safe draw;
a real out-of-bounds footprint still refuses without writes. Pure source-copy
modes also avoid an unnecessary destination read. Row, source, alias, format,
thread and ABI admission remains in place.

## Diagnostics and reproducibility

CPU-authority refusal version 2 contains 11 DWORDs: magic, version, pixel identity,
generation low/high, nonce low/high, refusal count, family, route and caller.
GPU-copy refusal version 1 contains 20 DWORDs: magic, version, admission stage,
target/source identities, validity/bootstrap flags, source/target generations,
epoch and the ten COPY fields. These are private launch diagnostics, not shared
pointer-bearing wire contracts.

Raster refusal version 2 contains 64 DWORDs. It retains the saved register input,
source-frame header and pointer, physical plane dimensions/stride/format, sprite
position, logical clip and kernel mode/admission options. The first refusal per
site may also retain the readable source frame, bounded at 4 MiB. Destination
pixels are excluded. Retained historical version 1 diagnostics remain unchanged.

`tools/test-raster-splice.py --frame-fixture <retained raster-source-09.bin>`
requires the exact source hash above and runs both the full canonical suite and
the captured frame through original and native splices. The latter checks WORD
and grayscale output, nonvolatile caller state, defined flags/FP state and generic
workspace. The existing three stale-reader refusal cases, held flips and actual
GL restart controls run alongside the new GPU-copy/private-baseline probe through
`tools/test-native-completion.py`.

## Current evidence and boundaries

The initial raster regression passes 9,323 canonical native cases and 47 admitted
captured-frame cases across the two affected entry families. Focused facade
controls pass GPU-copy authority transfer, untouched CPU storage, optional private
baseline capture, invalid geometry/key/self/lease refusal and full CPU recovery.
The final source-bound execution records are linked below.

All route installations and native/forwarded counts remain separate from coverage
of unexercised branches. Neither a passing native COPY nor this source-only raster
comparison establishes whole-session original pixel equivalence, arbitrary CPU
reads, physical driver-loss recovery or hardware performance. Clipped/self GPU
source copies now pass 23 actual facade/bridge/GL controls, including poisoned CPU
backings and complete scratch retirement. This is headless owned-policy evidence;
the [current live follow-up](native-gpu-copy-live-validation.md) validates 867
opaque disjoint GPU calls and their native destinations at three engine PCs.
Clipped/self/keyed GPU calls remain unexercised live, and other stale
destination-dependent readers still need their own matching-history validation.

## Explicit CPU readers

After GPU-copy initialization was admitted, launch
`working/experiments/menu-observer/run-beuzoaja` reached a later facade GetDC
reader at generation 586. Its recorded caller 0x795b5f19 corresponds to the
GetDC virtual-call return at RVA 0x15f19 in that staged `MnmRender.dll`; it is not
an address in the facade DLL. CPU history remained stale, so the guard correctly
stopped this reader. This required current native pixels rather than a copy token.

Stream version 5 retains all version 4 operations and adds CPU_READ(21). Its ten
DWORDs are the native surface ID, request ticket low/high, command session,
width/height, physical history generation low/high and nonce low/high. Version1
reply tickets use a nonzero 32-bit lifetime with high DWORD zero, advancing by one.
The consumer validates the existing surface's RGB565 extent, the live World nonce
and generation, the next ticket and the command session before reading pixels.
The read executes after all prior commands, including World import and UI writes.

The adapter atomically replaces `<command-channel>.cpu-reply` with a 64-byte
`MNMCREAD` version 1 header followed by tight RGB565 pixels. The header binds the
request metadata and payload length. Missing, old, malformed, truncated, oversized,
wrong-session or cancelled replies never become CPU input. Waiting is bounded at
10 seconds. The tracker is released while waiting; the ordered copy admission
remains held, with a bounded reader-specific extension for other waiting callbacks.
The ordinary command publication worker continues independently.

`NativeDrawCpuTransferV1` queries a process-local physical identity and restores
only that exact unborrowed backing, extent, pitch, generation and nonce. The base
pointer stays inside the Wine process and is absent from both wire records. A
successful restore makes CPU storage current before the original GetDC facade
method copies it into its GDI lease. The adapter also replaces its private CPU
checkpoint with the returned pixels. This lets later GDI and CPU work consume
current native history and allows normal resource eviction/re-creation from it.

These are explicit native GPU readbacks for CPU readers. They are counted in the
command consumer and campaign record. The per-World transport still returns no
pixels and the World renderer retains its separate zero-readback contract. This
change does not claim that rendering can satisfy arbitrary CPU readers without
synchronization or that full-session original rendering is equivalent.

The actual-GL control reads 960,000 WORDs across two ordered snapshots, including
a World import, masked UI patch, GPU copy and later CPU update. It matches every
WORD against independent expected values, rejects 36 malformed reply variants and
five invalid command requests, and rejects a wrong command session before another
readback. The facade probe rejects wrong nonce/extent restoration and verifies
that only a matching native restore establishes CPU authority. Existing version 4
World import, incremental consumer, frame-recipe and terminal restart controls pass.

## Menu lease during explicit readers

The first version of the GetDC integration reached gameplay, native Mini Cancel,
Quit confirmation No and resumed gameplay with 15,687 native producer calls and
zero original forwards. It then exposed a lease mismatch. At host menu timeout,
the retained command ring contains CPU_READ request 21/sequence 59927, while the
last atomic reply is request20. The two-second host menu-state lease can expire
while the engine is synchronously waiting for the ten-second native CPU transfer.
This is separate from raster or CPU-history admission; no rendering refusal was
recorded. The failed run is
`working/tests/native-campaign/run-tdht45jb/report.json`.

The command decoder now notifies its owner of the specific queued CPU-reader
ticket. A native worker exposes that ticket through a host-local atomic; the
GUI thread updates the menu adapter. A ticket permits at most ten seconds of
reader wait, and completion permits the ordinary two-second window to receive a
fresh engine menu snapshot. Repeating the same ticket does not reset the budget.
The adapter does not update its menu snapshot timestamp or request identity, and
ordinary rendering progress retains the original lease behavior. New launches
clear this wait state. Cancellation clears the renderer's pending notification.
The mapped-menu fixture checks these bounds and unchanged request/state identity;
the actual-GL reply fixture checks pending/completed notification pairs.


## Final executed result

[The combined campaign](native-history-readers-campaign-20261015.json) passes
native gameplay, Mini Cancel, resumed gameplay, Quit No, Quit Yes, native Defeat
Continue, Main Quit and original exit0. All 53 optional routes are installed;
17,209 calls run natively, zero forward to original pixel producers, and 34 routes
remain unexercised. There are no CPU-history refusals. All 1,241 World completions,
imports and verified returns balance at normal retirement. The ordinary World
path performs zero native/RGBA/World readbacks and returns no World pixels.
There are 34 explicitly ordered GetDC snapshots, recorded separately.

This is a software llvmpipe functional run with an explicit 10 FPS floor and
16.18 average engine FPS. It does not establish hardware performance, the older
hardware campaign's 19 FPS floor, casting/combat outcomes or full original pixels.
`complete_replacement` remains false and replacement milestones are unchanged.

[Focused controls](native-history-readers-controls-20261015.json) pass current
CPU authority, GPU copy/private baseline, exact native restore, malformed replies,
bounded menu waits, held flips, legacy stream/consumer/frame regressions and fresh
worker restart. [The raster comparison](native-history-readers-raster-20261015.json)
retains all 9,323 canonical native cases and 47 admitted captured-frame cases.
[The resource regression](native-history-readers-resources-20261015.json) retains
the independent clipping/format/HWND/lifetime checks and zero final native owners.
Original manifests verify before/after consuming original inputs; input and source
fingerprints are retained. Earlier failed/partial runs remain historical.

The incoming-history review covers 720c187b..a7d58fcd, all 61 exact file transitions
and receipt chains, with zero unresolved transitions. It is accounting evidence,
not a retrospective gate pass or fresh engine validation. Current changes have
separate exact reviews and a refreshed source census.

Subsequent [route-target inspection](native-pixel-route-scenarios.md) corrects
the old seven-family/53-route interpretation: ten alleged DC-copy CALLs actually
target ClientToScreen and are removed. The four-case minimap switch selects
camera0,2,0,1. Current six-family camera and spell journeys retain24unexercised
valid routes;23are other caller/scenario gaps, while camera3 is absent from this
reviewed dispatcher. Historical reports, fingerprints and their bounded outcomes
remain unchanged; no retired label receives validation credit.

The [active copy-route follow-up](native-copy-route-reachability.md) now resolves
part of the zero-keyed-GPU ambiguity: 1,828 actual keyed calls use CPU-backed
sources, while 1,666 opaque GPU-source calls have independent destinations. No
active clipped/self call occurs; these branches are not proved dead. The output
observer excludes the CPU-keyed commands, and seven post-retirement callbacks
remain explicit exclusions. No general CPU-coherence or complete replacement
claim is added.

## Idle raw-access discovery follow-up

The opt-in [idle CPU guard](native-cpu-reader-guard.md) now catches raw reads and
writes to stale owned backing outside CPU leases. Its20 actual PE32 schedules
retain active writable-lease reads as an explicit counterexample. This provides
a fault census boundary; it does not materialize arbitrary readers or establish
whole-game coherence.

## Active writable Lock investigation

The [mode2 reader census](native-cpu-reader-guard.md#active-lock-discovery-2026-10-17)
now retains protection through writable Locks, permits selected pure stores with
one owned single-step trap, and terminates loads/unsupported store candidates.
Observed live barriers include metadata-only scene checkpoints, the held-Lock
private Unlock baseline and the pure WORD splice9 permission query. Their routes
preserve stale authority: metadata stays metadata, private snapshots stay write
masks, and the raster's actual instructions remain guarded. The scene's existing
World guard receives a checked loan rather than silently replacing the CPU guard.
Actual pixel readers, DC DIB reads and concurrent escapees retain separate scope;
no transparent coherence or complete replacement follows from these changes.
