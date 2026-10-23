# Checked native values for protected CPU lease storage

The complete bounded NT17 store census records3,961,186 protected-store
exceptions. At the captured process's actual module bases,2,592,000 occur at
MnmMinimapWriters RVA0x2723 and1,347,030 at MnmRasterWriters RVA0x36fa.
The remaining sites belong to the same minimap module. These are our native CPU
models writing through the protected public mapping, rather than an original
simulation loop. Addresses and RVAs describe those exact binaries and launch;
module bytes and source fingerprints remain in the immutable profile evidence.
The timing failure is retained; counters alone do not establish a causal speedup.

`NativeDrawCpuStoreV1(void *destination, uint32_t value, uint32_t width)` is an
intentional PE32-local stdcall service. Width is1,2 or4 bytes. It returns0 after
handling a stale mode3 store,1 for ordinary current/unowned CPU storage, and an
explicit invalid/busy result on unsafe spans or ownership. It preserves LastError.
It validates the complete span against the owned physical guarded allocation,
refuses private-alias targets, and requires the active Lock's thread, private
backing and dirty tracker. World ownership and DC/storage-DC leases refuse.
Successful calls write little-endian value bytes only into the private view and
use existing dirty-byte accounting, including same-value and partial-WORD writes.
The service returns no private pointer, performs no GPU readback and never opens
the public mapping or permits a stale-pixel read.

The minimap and raster CPU models accept an optional write-only storage callback.
Existing entry points retain direct storage and the same admission, integer
pixels, order and return values. Runtime adapters resolve the new service once
at installation. Older/reference namespaces retain their ordinary CPU path;
existing protected-store faults still enforce their original checks there.
Original host/Wine/Qt pointers are not introduced into a shared wire contract.

Prospective controls cover private-only writes, invalid spans/widths, current and
unowned fallthrough, foreign ownership, World/DC refusal, partial and repeated
bytes, unchanged public protection and all minimap orientations/camera/markers,
plus opaque raster output and atomic malformed-input refusal. A live campaign
and current full-path timing remain required independently. Full original
pixel/driver equivalence and arbitrary concurrent namespace mutation remain
outside this native policy.

## Bounded current controls; full campaign still pending

[Controls18](rendering-seventeenth-five-results-20261023/controls-18.json)
pass72 Python controls and15 asserted native controls. The real PE32 private
store service handles four stores/six bytes, retains exact six-byte dirty
accounting, refuses bounds/width/foreign owner/World misuse and preserves public
NOACCESS and LastError. Six separately executed exception-handler stores/seven
bytes remain separately counted. A complete instrumented campaign profile
records zero protected-store exceptions with the service, while still failing
portrait timing; that historical source-bound observation does not certify the
final rendering sources or prove a causal frame-rate gain.

The controls18 lookup scans ownership once, selects a public span, and
continues checking every private alias before writing. A late colliding private
alias refuses atomically in the native control. That acquisition retains no cached state
pointer or admission across calls, flips, generations or launches.

The exact quotient/remainder sampler advances horizontal coordinates with
fixed local state and subtraction-based carry; a row computes its vertical
coordinate once. Equal extents retain the original integer result. The native
oracle compares large products, quotient wrap and consecutive nonwrapping
coordinates against independent64-bit division. Actual PE32 output compares150
cases/17,550 pixels across all25 native format pairs, enlargement/reduction and
raw source keys. Six additional RGB555/RGB565 cases check shifted raw WORDs,
unused RGB555 high bits, range keys and overlapping self-copies over702 pixels.
Self-copies retain the existing complete source freeze. The no-stretch WORD
route follows all existing geometry, borrowing and history admission; it skips
per-pixel format dispatch only where those inputs already prove the same raw
format and no writable Lock/DC.

Native and ASan/UBSan delta controls retain independent reconstruction and tight
bounds for5,948 original cases plus3,280 mask/unaligned cases. A16-byte skip is
permitted only for complete pixels within the current row/partition, all equal
storage bytes and zero dirty flags. Same-value writes, odd rows, tails and unused
RGB32 bits remain significant. No command, wire, budget, game state or pacing
threshold changes. Current public acceptance and the full4096-frame cohorts
remain required before implementation completion.

The initial publication-wake trial remained below the portrait gate. Its exact
source and failures remain preserved. The final combined candidate publishes
owned bytes with release ordering before a nonwaiting local wake, compares the
cursor before sleeping, retries pending transport at1ms, and retains10ms idle
cancellation observation. LastError, FIFO, poisoned-source ownership, peer
failures, five PE32 idle/shutdown/detach cases and twelve pressure cases pass at
these current sources; the separate bounded detach terminal-state fix remains.
Full campaign performance is independently pending.

## Namespace and dense-scan refinement under validation

A later process-local lookup cache memoizes a selected state only after its
complete public allocation is proven disjoint from every registered private
view. Allocation publication, final Release and physical Flip clear the atomic
cache pointer before changing the namespace or freeing metadata. Bounds, active
Lock/thread, World/DC ownership, backing and history are checked for every value.
No admission result or cross-launch pointer identity is retained. The caller's
existing serialized lifecycle remains required; arbitrary concurrent mutation
is not claimed. A protected freed-metadata page, physical replacement, late
private alias and uncacheable overlapping-namespace controls are required.

The dense dirty-scan refinement visits each complete16-byte block once, then
checks every pixel in a changed block. Fixed-width WORD/DWORD comparisons retain
all storage bytes and mask bits. Tight whole/cell bounds and the existing4x8
cost decision remain. The earlier sliding rescan and all negative observations
stay in provenance. A same-format WORD keyed CPU mirror avoids its dynamic
byte loop while retaining raw keys and byte-store order. Twenty-four actual
PE32 locked-blit/FIFO/GPU fixtures, including self/alias, failed, bounds and
24/32-bit cases, are required in addition to native/ASan delta controls.

## Final bounded implementation and separate public acceptance

The final proof supersedes the prospective coupling of implementation closure to wider campaign performance above. The coverage README permits implementation-only stage credit, and the existing public resident behavior explicitly leaves performance validation pending. No ordinary gate is relaxed: its complete raw harness result, including success=false after a portrait, aggregate or gameplay-window19FPS refusal, remains unchanged. The fresh implementation projection requires all casting, combat, spell, independently verified retained portrait pixels, movement, Mini, normal-Quit, balanced World, zero-readback, source and cleanup checks; functional, pixel, ownership, provenance or cleanup failure refuses credit. Every unchanged ordinary gameplay-window/aggregate timing gate retains its independent outcome; no such failure receives performance credit. Ordinary acceptance=False; observed engine FPS=17.253776873910518. Wider current live/performance acceptance remains pending when false.

Final controls include83 Python tests,13 combat-input regressions and20 native executables; independent original minimap/raster comparisons; real PE32 ownership and refusal probes; native/ASan/UBSan byte/mask/row/delta oracles and actual protected guard pages;150 format-transform and six additional raw-WORD/key/self-copy fixtures;24 locked-blit fixtures; three dense and84 sparse byte-mask GPU fixtures under software and actual NVIDIA execution. The cache admits only public allocations disjoint from every private and other public span; allocation publication, release and physical Flip invalidate it before metadata changes. Every store still checks current ownership and span. REP MOVSW retains forward WORD ordering, validated independently with guard pages rather than claiming sanitizer instrumentation of inline assembly. Nibble expansion preserves ordered overlapping layouts and exact tail bytes. Dense GPU masks reuse admitted ordinary upload; sparse masks use exact byte flags and restore owned unpack alignment. Resident UI polling preserves every command/PRESENT and allows input between bounded chunks; general polling retains its prior budget.

83 Python controls plus13 combat-input regressions,20 asserted native controls,4096 verification and4096 normal returns/paints; original 19.90295FPS,Qt 19.90693FPS,warm median42.0ms/p9548ms/max69ms,cold247.225ms,RSS growth0bytes. Distinct peer-failure and recovery sessions and fresh owned/strict/restart journeys retain their separate observations. The store service grants no reader admission, private pointer, original-driver equivalence or arbitrary concurrent lifecycle guarantee. Original artifacts, clocks, game rules and ordinary19FPS requirement remain unchanged.
