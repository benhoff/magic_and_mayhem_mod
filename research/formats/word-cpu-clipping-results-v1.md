# Private CPU clipping comparison records, version 1

Offline inputs use the private seeded fixtures documented in
[word-backend-state-fixtures-v1.md](word-backend-state-fixtures-v1.md). Live wire
records remain unchanged. Expected after pixels are comparator-only data; CPU
drawing and workspace prediction never read them.

`state-results.bin` has an 8-byte `MNMWCT01` header, little-endian DWORD version 1
and record size 256, followed by one 64-DWORD record per input manifest line.
Its fields and check masks follow the state fixture format except:

| DWORD | CPU comparison meaning |
| --- | --- |
| 7 | New native CPU clip admission accepted the input, including empty dimensions. |
| 52 | Host clipped adapter actually handled the positive request. |
| 53 | Host adapter actually forwarded the request to original; empty dimensions retain original floating handling. |
| 60 | Production entry / clipped CPU / recovered workspace check mask. |

Fields 52/53 are mutually exclusive execution observations. Positive corpus
requests must be handled, empty corpus requests must forward, and all ten
original/model and native-entry checks must pass (`1023`). No pointer identity
is assumed stable across launches. All excluded fields and the host-only nature
of the adapter remain as documented in the state format.

The producer is `tools/test-word-clipped.py`; the executor is
`tests/word-clipped-reference.cpp` with `tests/word-backend-probe.S`. Unknown
versions/extents and partial records are refused. Result hashes and source hashes
are retained independently under each experiment's immutable report.
