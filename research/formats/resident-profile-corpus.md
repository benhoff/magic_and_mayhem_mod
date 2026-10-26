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

## Compressed version 2

The bounded background writer emits `MNMPROF2`. A record begins with the same
16-byte generation/source/request header, followed by uint32 compressed length
at offset16, then a Qt `qCompress(...,1)` byte array at offset20. That array starts
with its four-byte **big-endian** uncompressed length and a zlib member encoding
source bytes followed by request bytes. This prefix must exactly match the checked
source/request sum before inflation. Compressed size is bounded by that sum plus
integer(sum/1000)+128. Expansion limits output to checked length+1 and requires
exact output length, zlib EOF and no trailing/unused compressed bytes. Corrupt
lengths, payloads and trailing members refuse.

The writer admits at most8192 Worlds,16GiB raw bytes and16GiB stored bytes, with
a256MiB pending raw-byte queue; compression scratch is additional bounded
per-record storage. Saturation/failure marks truncation/error without blocking
gameplay on disk I/O. Only completed retirement and matching accepted/written
generation counts permit `capture_complete`. V1 describes the retained initial
failed synchronous experiment and the normalized benchmark input. V2 is expanded
once, outside benchmark timing, into a new V1 file whose exact hash is retained.
Current compressed writer/reader execution is pending.


## Explicit diagnostic completed prefix

The default replay requires complete independently validated gameplay capture.
`--completed-prefix` retains that default and provides a separately named
measurement of contiguous imported/original-returned Worlds from a failed
journey. It validates bounds, generations, exact decompression and EOF for the
**whole** stored corpus, including any excluded pending tail. It does not assert
rendering semantics for an excluded record. The selected prefix alone is copied
to V1 and replayed; the original V2 is preserved and hash-checked against the
post-cleanup campaign record. Its decoded total must equal the accepted worker
snapshot. Full campaign/normal Quit remains explicitly false or unverified.

Normalization reports `worlds` and `raw_bytes` for selected V1, `captured_worlds`
and `captured_raw_bytes` for the full decoded V1 extent, and `stored_bytes` for
complete V2 storage. Both complete decoded storage and stored input retain their
16GiB limits. The selected generation count must be64..8192 for benchmarking.
The source-file final size can exceed a failed in-game report's worker snapshot;
only the post-cleanup corpus hash/size admits that diagnostic input.

## Source-history version 3

Explicit synchronous Save/Load capture requests V6 delivery and emits
`MNMPROF3`. Ordinary profiling retains V2; owned/asynchronous delivery remains
V5. Each V3 record has this 32-byte little-endian header:

| Offset | Type | Field |
| ---: | --- | --- |
| 0 | u64 | Next World generation |
| 8 | u64 | Source-chunk ordinal; zero denotes a World |
| 16 | u32 | Source length |
| 20 | u32 | Request length |
| 24 | u32 | Compressed length |
| 28 | u32 | Reserved, must be zero |

Compression and exact inflation checks match V2. Nonzero chunk ordinals increase
independently from one and require an empty request. Prefixes and their following
World share the next World generation. Prefix application preserves the same
resource/history instance and grants no World completion.

The archive retains 8,192 total records (Worlds plus prefixes), 16 GiB raw/stored
and a 256 MiB pending raw-byte limit. Saturation marks capture incomplete and
never resumes a dropped stream. Retirement checks both accepted/written counters
and source-chunk counts against the actual channel. Source-aware storage replay
validates the whole archive, then applies every prefix before its selected World
draws. Flat V1 benchmark normalization explicitly refuses V3 history; it cannot
silently discard resource changes.

Metadata checkpoints never write oracle pixels and do not consume the finite
9,999-file/byte oracle budget. Finite pixel-oracle capture retains those checks,
while every producer stream retains its existing record/byte envelopes.

Fresh writer/reader controls and a complete captured Save/Load journey are
recorded in the [Save capture recovery](../runtime/native-save-capture-recovery-20261025.md).
Historical experiments and their original status/fingerprints above are retained.
