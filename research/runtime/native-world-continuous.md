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
and uses CRC32C for the inner V5 reply pixel guard. Finite batchV3 retains its
historical hash contract and whole reply pixel FNV guard. The host uses checked
8-byte CRC instructions on x86_64; the PE32 producer uses the 4-byte path. Per-request private checksum storage prevents cross-thread descriptor
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

All warm queues, starting at generation64, enter a fixed151-bin latency histogram
with a separate maximum. The last1024 rows provide stage detail only. Completion
waits for the final Qt paint; normal admission requires one paint per queue.

The timing increment uses protocol V3 (MNMCON03), preserving the fixed channel extent and rejecting V2 peers. Completed original pacing entry/commit clocks and the selected final screen-copy duration precede each queue; native rows also record actual inter-World gaps. These observations preserve actual clock values, original wait targets and simulation rules. Direct NVIDIA GLX on the private headless seat failed with NV-GLX BadWindow; that result is retained and accelerated Zink remains the admitted hardware path. Frame attribution and sustained throughput are pending fresh execution.

## V4 GPU acknowledgement checkpoint (2026-10-10)

The explicit `--resident` test path uses `MNMCON04`/mode1 and80-byte identity/nonce/generation/descriptor-bound acknowledgements. It retains GPU canvas history and restores original destination protection without writeback. Actual original return marker2 is checked before slot release. V3 comparison/pixel mode remains separate. See [GPU frame ownership](native-gpu-frame-ownership.md) and the immutable [4,096-queue report](native-gpu-ack-masked-4096-20261010.json). All4,096 queues and native presentations complete with zero GPU readbacks/pixel replies/World writebacks and bounded resources. Both peer-death controls at generation34 and a128-queue fresh recovery execute. Full performance fails at16.1786FPS and a664ms maximum;19FPS/60FPS, real-game DirectDraw retirement, picking and complete gameplay rendering remain pending.


## Fog readability window and current4096 performance (2026-10-11)

Continuous tag45 fog copying resets a checked readable-memory window for each
admission. It reuses region protection proof only within that synchronous loop,
with no original callback or retained cross-queue pointer proof. Fog bytes and
signed comparisons stay unchanged; legacy noncontinuous checks stay per-cell.
The independent memory fixture admits8192 bytes using2 region queries versus
8192 fresh checks, and refuses guard/uncommitted/null/overflow boundaries and
protection changes on the next admission. Assertions and ASan/UBSan pass.

The source-bound [4096 run](native-fog-window-4096-pass-20261011.json) passes all
unchanged budgets on actual TITAN RTX accelerated Zink:19.8825 original and
19.8868 painted FPS,cold451.04ms, all-warm4033 World samples median36/p9540/max67ms,
cache492/512,RSS growth9,388,032 bytes. All4096 returns and paints are checked;
coalescing, pixel replies, original World writebacks and GPU World readbacks are
zero. Real ddraw is disabled/absent. Last1023 region queries median7635; differing
scenes preclude a causal paired speedup claim. The earlier128 run failed its
unchanged500ms cold limit at535.73ms despite warm19.80/19.98FPS and complete delivery.

A subsequent full public campaign separately fails copy-order admission: its
surface_lock holder reached105ms and longer. This does not invalidate World
transport measurements or establish robust public gameplay. Public V4 fusion,
arbitrary-scene19/60FPS, original pixel equivalence and semantic COM Quit remain
separate unfinished milestones. Historical hashes and failures are preserved.
