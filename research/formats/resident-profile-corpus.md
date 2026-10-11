# Native resident profiling corpus, version 1

This is a new native diagnostic file, not a recovered game format or a runtime
wire replacement. It captures already-owned original producer source packets and
ownedV5 requests for ordered offline replay. It never stores a World destination
pixel reply. Runtime integration is documented in
`../runtime/native-resident-performance-profiling.md`.

The file starts with the eight ASCII bytes `MNMPROF1`. Each following record has
this 16-byte, little-endian header and two consecutive owned byte arrays:

| Offset | Type | Meaning |
| --- | --- | --- |
| 0 | uint64 | World generation, starts at 1 and increments exactly once |
| 8 | uint32 | producer source byte count, at least 16 and at most 16 MiB |
| 12 | uint32 | ownedV5 request byte count, at least 64 and at most 197184 |
| 16 | byte array | source bytes, then request bytes |

There is no terminal marker inside this file. The independent campaign report
records its exact size/hash, captured generation count, truncation flag and normal
Quit completion. EOF alone is not sufficient evidence of a complete session.
Source journals and requests retain their own strict decoders, bounds and hashes.
A partial header/payload, missing or repeated generation, wrong magic or invalid
extent refuses replay. The writer stops capture at 8192 Worlds or 16 GiB and
reports truncation without changing gameplay admission; a truncated corpus cannot
pass the offline harness. Synchronous capture I/O is measured separately and
perturbs the live diagnostic run. File hashing streams bounded chunks.

Confidence: confirmed by the native writer/reader implementation; fresh complete
capture and hardware replay execution remain pending.
