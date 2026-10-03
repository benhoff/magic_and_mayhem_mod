# Neighbor expansion capture: MNMEXP01

Toolkit-designed format; not an original game file format. Writer:
`runtime/shadow/bridge.c`. Readers:
`reconstruction/pathfinding/route_neighbor_replay.cpp` and
`tools/compare-neighbor-shadow.py`. All fields are little-endian; signed values
retain their 32-bit representation. Confidence: confirmed synthetic writer /
reader round trip, real engine capture pending.

| Offset | Size | Meaning |
|---|---|---|
| 0 | 8 | ASCII `MNMEXP01` |
| 8 | 4 | Current cell pointer token (entry ECX) |
| 12 | 4 | Budget before original expansion |
| 16 | 4 | Budget after original expansion |
| 20 | 4 | Candidate count before expansion |
| 24 | 4 | Candidate count after expansion |
| 28 | 70 | Raw input descriptor |
| 98 | 28 | Raw prior movement payload |
| 126 | before count * 36 | Full vector before expansion |
| following | after count * 36 | Full vector after expansion |

Exact file length is `126 + 36*(before_count + after_count)`. A capture has
at most 4096 input candidates and at most 26 appended candidates. Both readers
reject malformed lengths. Candidate comparisons include opaque prefix bytes,
float payload bits and trailing fields, rather than just nodes and costs.

The same-number `world-NNNN.bin` uses the documented
[MNMWLD01 format](route-world-snapshot.md) frozen before expansion. The DLL
does not emit the debugger writer's JSON sidecar; `compare-neighbor-shadow.py`
reads these pairs through the C++ reader and records both SHA-256 values in
`comparison.json`. The experiment-level manifest supplies runtime or synthetic
origin and the staged executable / DLL hashes for runtime experiments. Files
without a manifest are labelled unverified. Pointers are tokens for replay;
the host never dereferences original engine addresses.

A world file without its expansion partner is an incomplete capture. Absence
of samples, snapshot rejection or replay failure does not imply equivalence.
See [runtime procedure and limits](../runtime/pathfinding-neighbor-shadow.md).
