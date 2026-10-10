# Direct-word admission diagnostics, version 1

`admission-stats.bin` appends 128-byte little-endian snapshots. Header DWORDs 0..3
are magic `MNMWAD01`, version 1 and record bytes 128. The existing sample and
statistics layouts are unchanged. This file belongs to opt-in clip-shadow mode.

| DWORD | Meaning |
| --- | --- |
| 4 | Seen hook calls |
| 5 | Frame extent/header refusal |
| 6 | Empty dimension forward |
| 7 / 8 | Canvas descriptor / writable extent refusal |
| 9 / 10 | CPU / workspace-model preflight refusal |
| 11 / 12 | Allocation / trial draw refusal |
| 13 / 14 | Busy / stopped forwards |
| 15 | Compared requests with at least one auxiliary offset |
| 16 / 17 | Scalar / forward backend comparisons |
| 18 / 19 | Scalar / forward clipped comparisons |
| 20 | Compared auxiliary-bearing clipped requests |
| 21 | Duplicate eligible auxiliary-bearing sample candidates skipped |
| 22..31 | Reserved zero |

Each fallback is attributed once. Counters are bounded snapshot lower bounds and
not timing measurements. Clipped samples are capture ordinals, drawn originally
with before/after pixels and workspace. Sampling retains two clipped and six interior distinct
frame-fingerprint/backend pairs for auxiliary-bearing requests; fingerprint collisions
conservatively omit candidates. Replay records full frame SHA-256, backend,
auxiliary offsets and dimensions and requires at least four distinct input hashes.
Opaque auxiliary offsets bound main-plane reads but their payloads are not decoded.

In this sampler the existing clip-stat DWORD14 counts all retained records and
DWORD17 counts only the two clipped records; the remaining six are interior.
