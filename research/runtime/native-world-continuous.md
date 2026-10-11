# Continuous guarded original source delivery

This increment replaces the accumulating diagnostic journal only in an explicit
`MNM_WORLD_CONTINUOUS` session. Legacy finite captures retain their protocol.
The pinned original traversal/caller checks and 800x600 RGB565 destination guard
remain mandatory. Native CPU history and GPU resources persist across queues;
original destination pixels never seed native rendering.

The fixed little-endian V2 channel has a128-byte header and two slots, each with
128-byte metadata,16MiB source capacity,197184-byte batch descriptor capacity and
960064-byte reply capacity. Total35869312 bytes. The source block is an owned
local V3 packet whose queue ordinal starts at1; the independent session generation
is64 bits. Local ordinal adaptation occurs only after checking that generation.

Slot ownership is `FREE -> WRITING -> REQUEST -> PROCESSING -> REPLY -> WRITING
-> RETURNED -> FREE`. The second WRITING state belongs exclusively to the original
producer while it appends actual closing records. Publication uses aligned
lock-free32-bit acquire/release operations;64-bit generation words are read only
under the state lease. Native role and original role are claimed once; nonzero
nonce, version, bounds, reserved fields, exact descriptor/source hashes, caller
entry/AX, destination extent and actual original return are checked. A slot cannot
be reused until native checks the unchanged prefix and actual checkpoint/return.
No supersession, dropped queue or implicit resume is permitted. Cancellation or
timeout is terminal; recovery requires a new file/session.

Original source definitions, observer counters and checkpoint ordinals reset per
block after the real original return, while surface identities and native history
persist. This resets observation bookkeeping, not original simulation state.
Source/record/descriptor overflow refuses the session. Diagnostics do not read or
write original destination oracle images, checkpoint image files, or disk replies.
Verification mode retains the independent final native CPU/GPU comparison.
Normal mode omits CPU reference raster composition; required GPU readback, CPU
history adoption and original caller writeback remain active in both modes. Qt presents the retained GPU canvas directly.

`--continuous-queues N` explicitly bounds automated private-Xvfb experiments;
the channel target0 permits streaming until cancellation, without an accumulating
report (last1024 timing rows only). A bounded4096-queue original run and separate
native/original peer-death runs are the intended validation domain. They are
pending before execution. Correct final native CPU/GPU words do not establish
independent original raster equivalence beyond the separately compared32 prefix.
Menus, HUD writers outside the selected World body, uncaptured branches, combat,
hardware desktop/input/physical scanout and unlimited-duration reliability remain
separate boundaries. Normal full-path pacing must be measured, not inferred from
this transport implementation.

The initial64-queue probe refused at the first native request: concurrent startup
producers interleaved record emission (508 before507, then509), while the owned
source checksum and extent remained valid. Continuous mode now serializes
source-definition/reference emission and sequence/byte allocation as one bounded
critical section. This is an observation transport ordering fix; original
simulation/drawing state and legacy finite mode are unchanged. The failed
source-bound execution is preserved separately from fresh validation.


The performance increment is a pending milestone. The initial4096 attempt
reached observed generation3710 before the600s deadline and failed. Later128
normal probes completed correctly but failed the declared full-path FPS budget;
these do not count as performance readiness. Preserve these results in
[native-world-continuous-performance-experiments-20261010.json](native-world-continuous-performance-experiments-20261010.json).

V2 uses standard reflected CRC-32C (Castagnoli, init/final xor0xffffffff) over
all source/request/reply bytes. The CPUID-gated SSE4.2 path matches an independent
portable bitwise reference, including known vector123456789, unaligned starts,
tails and varied extents. V1 sessions are refused. Continuous batchV5 binds the
canonical96-byte raster header to little-endian CRC32C(exact owned payload),
while finite batchV3 retains its historical hash contract and whole reply pixel
FNV guard. Per-request private checksum storage prevents cross-thread descriptor
hash reuse. Native guards finish before rendering or reply publication.

The original destination remains NOACCESS while native rendering completes.
Cooperative Sleep(0) channel waits retain the real uptime deadline. Normal mode
uses the existing guarded actual WinMM precision clock and disposable native
DEBUG draw-skipping policy; it does not change selected game speed or fabricate
timestamps. x86 REP MOVSD copies nonoverlapping owned buffers. One bounded private
raster scratch allocation is exclusively leased through immediate request,
emission and descriptor acceptance; legacy/raw requests retain their allocations.

Native queue preparation consumes the validated owned stream directly. Draws
retain immutable shared source leases through completion instead of duplicating
encoded frames. Frame-local lease/tag/extent identity memoization expires with
that frame; the retained identity cache still owns and checks complete encoded
bytes. Original pointers never become native semantic identities. The private
hardware runner uses an unattached, no-input X server and checks accelerated
Mesa Zink on the NVIDIA GPU, including the actual Qt GL renderer; it rejects
software fallback. This measures full Qt paint completion, not physical scanout.

Current-code4096 verification,4096 normal delivery, peer deaths beyond32,
fresh recovery, all-warm performance accounting, normal/reference equivalence,
and an impossible0.001ms budget control remain pending. The limits remain50ms
median,75ms p95,150ms maximum warm whole-World time,500ms cold delivery,19 FPS
original/Qt paints and32MiB retained memory growth. Final integrated public native
casting/combat/portrait readiness remains a separate source-bound rerun.
