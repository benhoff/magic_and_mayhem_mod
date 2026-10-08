# Canvas startup diagnostics v1

This is a private bounded observation file, not a rendering input or public IPC
channel. All integers are little-endian uint32. Raw addresses are diagnostic;
decoded pointer tokens are process-local and do not identify allocation generations.

The 160-byte header begins with `MNMCST01`, then version 1, header size 160,
record size 160, capacity 16,384 and hooked-entry count 12 at word offsets 2..6.
Words 7..39 are reserved zero. Every record is 40 words:

| Words | Meaning |
| --- | --- |
| 0..4 | Sequence starting at 1, operation tag, phase (entry=0/return=1), normalized original caller-return VA, returned EAX (zero at entry). EAX is not a promised HRESULT for void wrappers. |
| 5..6 | Entry ECX/EDX. |
| 7..12 | Six diagnostic entry-stack words. Only the recovered arguments for each wrapper have semantic meaning. |
| 13..15 | Currently bound pixel pointer, stride in words, bind height. |
| 16..19 | Clip left/top/right/bottom. |
| 20 | Alternate canvas pointer `0x6e1f68`. |
| 21..26 | Selected surface wrapper fields +4,+8,+0x0c,+0x10,+0x14,+0x18, or zero when no surface wrapper applies. |
| 27..32 | Sample pointer, width, height, stride in words, nonzero-word count, bytewise row-major FNV-1a. Count `0xffffffff` means no readable admitted sample; a zero count is an observed zero sample. Row padding is excluded. |
| 33..39 | Reserved zero. |

Tags 0..11: create, release, lock, bind, clip, full fill, rectangle fill,
opaque copy, source-key copy, JPEG decode, shared sprite dispatch, selected indexed
raster. Tag 12 marks first World queue entry and ends the trace. Tag 13 reports
return-slot overflow; it makes the trace inadmissible. Record-capacity exhaustion
or an I/O failure leaves no complete terminal marker and also refuses analysis.

Lock-return and bind samples read bounded 16-bit pixels. Fill/copy/JPEG samples
may read a last-known lock pointer, invalidated by observed create/release;
readability does not establish lock ownership or driver completion. Width/height
must be 1..2048 and stride width..4096 words. These diagnostics must never seed
native rendering. The decoder requires a complete entry/return nesting sequence
and a terminal World marker; interleaved arbitrary-thread calls are unsupported.
