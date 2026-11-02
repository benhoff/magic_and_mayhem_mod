# Native CPU-read completion counters v1

This is an authored native bridge diagnostic, not an original game format.
`game_cpu_read_counts_flush` in `runtime/render/owned_session.h` writes exactly
48 bytes after application admission closes and publication joins. The fields
are twelve little-endian u32 words:

| Word | Value / meaning |
| --- | --- |
| 0–1 | Eight ASCII bytes `MNMRDC01` |
| 2 | Version 1 |
| 3 | Header/file bytes 48 |
| 4 | Last completed monotonic CPU-read ticket |
| 5 | Kind0: non-Lock materialization for GetDC or panel preflight |
| 6 | Kind1: explicit owned-Lock reader |
| 7 | Kind2: implicit owned-Lock reader |
| 8 | Kind3: explicit reader without a Lock |
| 9–11 | Reserved zero |

The file carries no command session, nonce, process identity or caller addresses.
Bind it through one immutable acquisition and independently check requests,
full reply identities and completion totals. Aggregate counts do not prove
per-ticket physical restoration or a particular original caller. The producer
omits this file when no completed ticket exists.

The [four-case retained binding](../runtime/storage-cpu-read-binding-20261102.md)
validates 212 requests/replies against this structure and preserves sparse
lifecycle/caller boundaries. Confidence is high within this diagnostic version.
