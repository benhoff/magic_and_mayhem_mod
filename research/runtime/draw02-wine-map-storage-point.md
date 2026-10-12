# DRAW-02 public map-buffer storage association

Step 2 remains in progress. The public map descriptor supplies a buffer address and pitches, while the producer recorder retains known canvas storage addresses and scalar state. A separate opt-in export will compare those addresses inside the process and emit a portable point linked to the exact resource map interval. It will read no source bytes or private Wine layouts.

Only a live recorder with a unique address and matching row pitch/extent can report a known canvas. Unknown, ambiguous, stopped and extent-mismatched entries remain explicit. Cached addresses are point associations, not allocation generation or exclusive ownership. PE32 export and forwarding fixtures precede original execution.

The PE32 export fixture uses an intentionally unreadable storage address to
prove the export compares cache metadata without reading bytes. It covers live,
unknown, ambiguous, mismatched pitches/extents, failed/stopped cache, nesting,
output bounds and LastError. The observer fixture forwards 12 original calls
once, exports five successful maps and skips the failed map, while preserving
LastError and the original-return 512-byte FP snapshot through a mutating export.
Four decoder methods validate exact interval/thread/extent and unknown states.

The separate `wine-map-storage-point.bin` journal uses MNMWSP01/version1,
64-byte header and 96-byte rows. Each successful map return has one point linked
to call/resource/interval/thread plus tick, export status and a 64-byte portable
cache point. The terminal counts all points; decoding checks coverage against
the public resource journal. No process address enters the wire.

Original01 completes normally with 1242 successful public maps: 795 known
points across six canvases, 14 unknown and 433 stopped. Of the known points,
760 identify canvas2 at 800x600 with WORD stride800. One map interval remains
open at observer close. The combined run also closes 118 protected pairs and
222 source commands, with 333 API and 332 EGL witnesses. These are separate
observations; no completed upload, immutable source lifetime or complete journey
claim follows from a point association. Historical native parity was not rerun
for this change, and the earlier refusal remains unresolved.
