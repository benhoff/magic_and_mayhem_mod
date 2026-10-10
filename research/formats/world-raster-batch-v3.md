# World raster batch completion V3

The producer stream remains MNMPRO01/V1. Each selected World queue emits its
owned raster sources without a per-raster request/reply. Before the original
World return is exposed to HUD work, a `world-batch-NNNN.request` contains the
64-byte little-endian header and16-byte descriptors defined in
`protocols/include/mnm/world_raster_batch_v3.h`. Each descriptor binds an actual
entry, return and producer sequence to the hash of its complete owned source.

The native consumer admits only the known entry/backend pairs, checks every
source hash, sequence, destination, recovered AX and complete queue count, then
publishes a full native RGB565 reply by atomic rename. The runtime validates
the whole reply before writing any original canvas words. Queued sources never
contain original destination pixels. The original canvas is inaccessible during
the queued traversal and restored before validation/writeback/checkpoint/HUD.

This is an opt-in bounded policy; V1 and V2 remain separate diagnostic modes.
V7 validates the guarded800x600 format0 startup prefix1..16. Wider gameplay,
formats/callers/globals and public launcher integration remain separate gaps.


## Header and closure

All words are little-endian UInt32; no host pointer or C++ layout crosses the
channel. The request magic is `MNMWBQ03`, and the reply magic is `MNMWBR03`.

| Byte offset | Meaning |
| --- | --- |
| 0 | Eight-byte request/reply magic |
| 8 | Version3 |
| 12 | Header bytes64 |
| 16 | Transfer ordinal, starting1 |
| 20 | Original World queue ordinal |
| 24 | Last complete producer sequence before return |
| 28 | Producer canvas ID |
| 32,36,40 | Width,height,tight stride (800,600,800 in this domain) |
| 44 | Payload bytes:16times raster count, or960000 for reply |
| 48 | FNV-1a32 of payload |
| 52 | Guard-held/success marker1 |
| 56 | Raster count in this queue,1..12320 |
| 60 | Cumulative admitted raster skips |

Each request descriptor contains four UInt32 words: producer sequence, actual
original entry, low16AX and FNV-1a32 of the complete96-byte producer record plus
owned payload. Descriptors must cover every raster in the queue in strictly
increasing order. Clip changes remain in the producer stream and are applied
in order. The original caller receives the prescribed AX immediately while the
owned raster executes later on the native canvas.

A reply echoes all identity/count words, changes magic/byte-count/checksum and
contains the tight RGB565 native canvas. It is published by atomic rename. The
adapter restores protection only in the original return callback, validates
length/header/hash completely, and then copies exactly960000bytes. It emits
checkpoint/return only after that copy. The return producer records cumulative
skips in word15, transfer count in word17, this queue's raster count in word19
and the verified guard marker in word20; reserved word23 stays zero.

Protocol checksums are correlation/integrity checks in an isolated session, not
new original-engine behavior. The original page guard is installed after entry
observers finish and before original traversal, retained during source capture,
and verified before restoration. No per-raster request/reply is emitted in V3.
