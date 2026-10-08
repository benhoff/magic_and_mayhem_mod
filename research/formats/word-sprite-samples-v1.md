# Direct-word sprite diagnostic records, version 1

All words are little-endian uint32 unless identified as signed. Records are owned
files in a fresh experiment directory; pointer values are diagnostic identities,
never host pointers. Partial files are rejected. Native takeover writes at most
eight pixel samples and an append-only statistics log.

`word-NNNN.bin` begins with an 80-byte header:

| Byte | Meaning |
| --- | --- |
| 0 | Eight bytes `MNMWRC01` |
| 8 / 12 | Version 1 / complete file bytes |
| 16 / 20 | Mode (1 shadow, 2 takeover) / admitted call sequence |
| 24 / 28 | Observed frame / canvas addresses, diagnostic only |
| 32 / 36 | Encoded frame bytes / complete canvas bytes |
| 40 / 44 / 48 | Clip-right width / clip-bottom height / positive stride in WORDs |
| 52 / 56 | Signed incoming anchor X / Y |
| 60 | Original backend RVA (`0x196cb8` or `0x197086`) |
| 64 / 68 | Signed clip-left / clip-top |
| 72 / 76 | Zero return / reserved zero |

The header is followed by 64 bytes of workspace before, 64 bytes of workspace
after, encoded frame, complete canvas before and complete canvas after, in that
order. Canvas bytes include stride padding. Workspace covers the original
`0x5f1e50..0x5f1e90` range. Only the two overwritten frame-pointer words at
indices 7 and 10 are rebased during isolated comparison; other untouched words
retain their captured values. Canvas alignment parity is retained when allocating
the oracle destination, because the scalar backend's workspace depends on it.

`stats.bin` appends 64-byte snapshots. DWORDs 0..3 are magic `MNMWRD01`, version
1 and record bytes 64. Indices 4..15 are mode, installed, seen, admitted,
bypassed, originally executed (including fallback), compared, mismatches,
refused/fallback, successful pixel samples, capture/log errors, and stopped.
Snapshots occur at startup, early calls, every 64 calls and process detach when
available. The final logged counter can lag a forcibly terminated process;
it is a lower bound, not an exact total or a performance measurement.
