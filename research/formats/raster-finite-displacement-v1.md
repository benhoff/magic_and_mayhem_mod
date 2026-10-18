# Raster finite displacement counters V1

A208-byte little-endian process-exit record named `raster-finite-displacement.bin`, emitted by the production raster DLL at normal process termination only. No drawing or tracker path performs file I/O. Missing/truncated/overflowed data is unavailable evidence, never inferred zero.

| DWORD | Meaning |
| --- | --- |
| 0..1 | Eight ASCII bytes `MNMFDSP1` |
| 2 | Version1 |
| 3 | Bytes208 |
| 4 | Routes23 |
| 5 | Counter overflow flag; must be0 for evidence |
| 6+2*i | Successful native calls at raster site i that retained one or more unowned-tap destinations |
| 7+2*i | Retained destination WORDs at site i |

Counters change only after successful complete atomic admission on the checked drawing owner. A retained call has at least one retained WORD and cannot exceed its independently recorded native route calls. This is branch observation, not original outside-allocation pixel equivalence. Process termination is required alongside the normal campaign shutdown/source/input/manifest checks.
