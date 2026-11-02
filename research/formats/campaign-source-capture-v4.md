# Campaign source capture V4

This is a diagnostic transport policy, not an original game format. Campaign
capture uses it only when both source-history capture and
`MNM_ACCEPTANCE_CAMPAIGN=1` are enabled. Existing V1–V3 inputs remain readable.
The producer snapshots owned source and request bytes before queuing work;
compression runs on the capture worker. Original destination pixels are never
an input.

The file starts with `MNMPROF4`. Each record has an 80-byte little-endian header:

| Offset | Type | Meaning |
| --- | --- | --- |
| 0 | u64 | World generation |
| 8 | u64 | Source chunk ID; zero denotes a World |
| 16 | u32 | Decoded source byte count |
| 20 | u32 | Decoded request byte count |
| 24 | u32 | Compressed body byte count |
| 28 | u32 | Zero for full key, one for previous-body prefix |
| 32 | u64 | Record ordinal, starting at one |
| 40 | u64 | Zero for a key, otherwise previous record ordinal |
| 48 | 32 bytes | SHA-256 of decoded source concatenated with request |

One standard Zstandard frame follows. Records 1, 129, 257, etc. are independent
keys. Other records use the complete immediately preceding decoded body as the
prefix dictionary, including when lengths differ. Compression level is 3,
the window bound is 64 MiB, and frame content size and checksum are enabled.
Readers check order, prefix identity, key intervals, exact frame consumption,
decoded sizes and SHA-256. Failure is an incomplete acquisition.

The existing producer bounds remain: 8,192 records, 16 GiB stored and decoded
accounting, 256 MiB queued bytes, and bounded source/request extents. The
resident capture report must also declare complete, untruncated capture;
an otherwise valid prefix cannot establish completion.

The optional codec requires Zstandard headers and runtime libzstd 1.4 or newer.
Qt loads the library only for V4; legacy QtCore consumers retain their existing
link contract. Controls used 1.5.7. Optimized and ASan/UBSan executions each
reconstructed 323,511,217 body bytes from 317 records and 12,420,093 stored bytes.
Independent C++ and Python readers agreed on every byte. Six C++ and ten Python
malformed controls were refused, and legacy capture controls also passed.
See `validation-source-controls-03.json` in the progression results directory.
These are synthetic transport controls; live engine comparison is separate.
