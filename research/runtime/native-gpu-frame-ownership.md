# Native GPU frame ownership and DirectDraw retirement

Requested 2026-10-10. Native GPU history and bounded V4 acknowledgement delivery are proved at their recorded source hashes. Performance and real-game DirectDraw retirement remain incomplete.

The first milestone retains native World output on the GPU while source-only
HUD/minimap/copy operations preserve intervening history. A CPU staging shadow
may retain only independently known pixels; regions modified by GPU drawing
become undefined there. Destination-dependent CPU producers must refuse unknown
pixels rather than reuse stale World output. Native GPU copies preserve GPU
contents without a CPU export. Legacy comparison/writeback mode stays separate.

The second milestone introduces an explicit, versioned acknowledgement-only
original/native session. Owned sources, exact request identity, destination
protection, deadlines, cancellation and actual original return remain checked.
Ordinary delivery performs no World readback, pixel reply or original World
writeback. Diagnostic comparison may explicitly read native pixels in separate
verification runs. Original drawing outside the admitted World body and its
surface services remain active until separately replaced. No general picking
or arbitrary original CPU pixel-read equivalence is asserted.

The third milestone replaces the supported original graphics services and
presentation with a native compatibility boundary: factories, surface storage,
locks, copies, palettes, DC ownership, lifecycle and recovery. Final proof must
disable the real DirectDraw backend and retain independent frame and caller
comparisons, bounded active-gameplay/failure/recovery scenarios, native Qt
presentation, resource cleanup and the original-source manifest checks.
Unknown operations remain explicit refusals and incomplete milestones.

The current initial threshold is nineteen actual admitted World completions and
Qt presentations per second, without changing game speed or the selected
twenty-FPS engine cap. Sixty-FPS presentation is a later separate milestone:
removing transfers does not itself lift the original render cadence or prove
sixty independently rendered frames per second. Simulation/rules modernization
and gameplay balance changes remain outside this workstream.

## Recorded milestones and boundaries

Commit `597005a` retains native GPU canvases, mirrors independently composed CPU source patches, propagates GPU-only copies and refuses stale CPU destination reads. The first32-queue report remains immutable. The later `native-gpu-frame-masked-history-20261010.json` repeats512 synthetic/legacy controls and32 retained input queues against independent native CPU composition:66,529 raster requests and15,360,000 words. Sparse source patches upload a colour plane and coverage plane, discard uncovered texels, and establish validity only where covered. No CPU destination seed or GPU history export enters this update. Malformed coverage and reads of incompletely defined storage refuse.

V4 has magic `MNMCON04`, version4 and immutable header mode1; V3 remains magic `MNMCON03`, version3 and mode0. It retains the two fixed-capacity slots for compatibility and bounds, but only80 reply bytes are written. The ACK is `MNMGACK1`, version1,80 bytes: request ordinal/queue/producer sequence/destination/width/height/stride, descriptor hash, queue and cumulative draw counts, u64 generation, u64 session nonce, owned-submission flag1 and reserved0. The outer slot CRC32C covers all80 bytes. Native and PE owners validate every identity. The flag acknowledges owned, ordered GPU submission; it does not claim a physical scanout or a CPU pixel comparison.

The original destination remains NOACCESS through source admission, native processing and reply validation. The V4 branch restores protection without any pixel copy. Its completion count supplies the local batch ordinal; actual return marker field20 is2, distinguishing resident acceptance from legacy marker1. The native adapter validates the actual original checkpoint and return before releasing the slot. The original CPU World buffer remains stale after this return. Original DirectDraw and drawing outside the selected World body continue; arbitrary original reads, picking and whole-session compatibility remain unproved.

The TITAN RTX/Zink run in `native-gpu-ack-masked-4096-20261010.json` completed4,096 original/native queues,6,770,930 raster requests and4,096 distinct native Qt presentations. It records zero World readbacks, pixel replies and original World writebacks, zero teardown surfaces, bounded caches and8,744,960 bytes of measured RSS growth. This is private accelerated rendering, not physical-monitor scanout. Consumer and producer death at generation34 terminate without a successful completion claim; a new session/nonce completes128 further queues. `world-gpu-ack-test.cpp` exercises512 leases, all80 individual ACK-byte mutations, stale generation/nonce, descriptor mutation, pixel-sized reply, mode mutation and cancellation refusals.

Performance is a failure, not a completed milestone. That long run measured16.1786 original completions/second, a44ms median World duration,58ms p95 and664ms maximum. The19FPS and150ms maximum thresholds remain unchanged. The warm original inter-World gap remains about15ms; earlier blocking traces did not establish its cause. Removing the framebuffer round trip alone does not meet19FPS. Masked patch batching reduced logical native uploads from20,061 to848 over the128-queue probes while preserving zero transfers; further graphics-service integration and profiling are required. Sixty independent World frames/second remains a later milestone.

## First DirectDraw factory foundation

`native-draw-factory-pass-20261010.json` compares128 default factory lifetimes and2,176 HRESULT, identity, per-interface reference and failure-result words with real Wine DirectDraw. The same unchanged PE32 caller probe executes both backends. Querying IUnknown through Draw1/Draw2 resolves to one canonical identity; Draw1/Draw2/IUnknown have separately measured reference counts and retain the shared factory until every interface reaches zero. The native probe disables ddraw, verifies no DirectDraw module loads, and the native DLL imports no DirectDraw symbols. Enumeration of one named native adapter is an intentional native policy.

This is a factory foundation, not game retirement: no surface storage or GPU API service is admitted yet, and unsupported method invocation exits97. The factory is not installed in the original game. Native success preserves incoming Win32 LastError; the Wine/Xvfb factory leaves1400, so LastError equivalence and original-game dependencies remain unproved. The original Wine factory also faults for a null output pointer; that retained failure is excluded from the common valid-caller corpus, while native safe invalid-argument refusal is tested separately. No original Windows behavior is inferred from those Wine-specific results.
