# Recovery metadata and checkpoint delivery diagnosis

2026-10-06. These are native producer/consumer policies and bounded Wine/Mesa
observations. They do not establish original pixel equivalence or live replacement.

## Confirmed failures and corrections

The early CHECKPOINT refusal is valid. New bounded diagnostics report reason 2,
an 800x600 RGB565 primary with current descriptor/pixel epochs and known layout,
but no owned pixel buffer. The harness now distinguishes this early request from
attachment after the original primary has published a complete frame. Admission
limits, uncertainty checks and borrowed-resource refusal remain unchanged.

The earlier ordinary recovery discarded the observed interface alias graph.
The original game subsequently locked one surface interface and unlocked through
another. The unmatched unlock invalidated the pixel epoch and session. Ordinary
recovery now synchronizes missed operations/retirement and metadata epochs, frees
all owned surface pixels, then retains current lifetime descriptors, properties,
aliases and independently observed palette state. Final Release, recreation and
missed retirement retain their existing invalidation rules. Fresh pixels still
require successful original operations; no observer COM calls or references were
added. CHECKPOINT retains its stricter complete-state admission.

The Qt adapter accepted a 1MiB poll budget but read only one 64KiB ring fragment per
GUI tick. Multi-megabyte checkpoint prefixes consequently took many ticks while
the producer continued drawing. It now reads multiple fragments within the
caller's byte budget, keeping the decoder's 64KiB fragment limit and 32 submitted
commands per call. The two new GPU fixtures check exact ACK bounds, independent
pixels, first-poll presentation at the larger budget and resource cleanup. They
also verify the smaller budget cannot present an incomplete record.

Diagnostics use the existing bounded lifecycle log: at most 128 distinct records,
four variants per reason, opaque identity tokens and owned metadata only.
`checkpoint_admission_refused` stores reason, surface token, admitted surface/byte
counts, current and surface epochs, known layout/data flags, width/height/bits/length,
primary flag and pending pixel/alias uncertainty. Reasons are 1 pending uncertainty,
2 invalid/incomplete surface or surface count,3 alias collision,4 pixel budget,
5 serialized byte budget,6 incomplete palette,7 primary count. The log does not
inspect or dereference original objects.

## Fresh validation

The explicit recovery fixture queries its surface alias once, then performs
canonical-interface locks and alias-interface unlocks across three sessions. It
preserves HRESULT/LastError and compares independently owned original pixels with
GPU output. Alias, repeat, guards and held cases passed 20 frame comparisons.
The complete 20-case CHECKPOINT matrix passed 294 comparisons. Native and ASan/UBSan
channel runs passed the two byte-budget cases, sustained production beyond 64MiB
and 4096 records, and seven PE32 sessions. Separate host and queue regressions,
production PE32/Qt shell builds and three selected CTests passed. A sandboxed
Xvfb CTest attempt failed to start its display; the permitted rerun passed.

The three bounded original-game cases retain distinct negotiation snapshots:

| Request | Observed result |
| --- | --- |
| Early CHECKPOINT | Refused: known primary layout, no owned primary pixels |
| CHECKPOINT after a complete primary frame | READY and one native frame; subsequent traffic overflowed the 32MiB queue, followed by another recovery attempt and fallback |
| Ordinary RECOVER after a complete primary frame | READY, no unmatched alias unlock; no native frame because admitted drawing encountered missing source pixel baselines and invalidated the new stream |

All original processes remained alive with their main menus visible. Consumers
retired their resources; original manifests passed before and after. These runs
deliberately leave the initial reader inactive to test failed-reader recovery.
They are not healthy steady-state gameplay or independent pixel comparisons.
READY, first-frame delivery, continued production and equivalence remain separate
milestones. An exploratory post-fix run produced two checkpoint frames but its
harness failed on a later negotiation; it is retained under working/ and does
not supply the current success claim.

The remaining ordinary-recovery baseline problem is observed as `blit_untracked`
and a primary rejection whose flags show known layout/clipping but no source
pixels. Previously initialized source surfaces need a complete owned checkpoint
or independently observed reinitialization. The adapter must not invent those
pixels. Sustained queue overflow still needs separate scheduling/throughput work;
the finite budgets and explicit invalidation were not relaxed. Real drivers,
gameplay, movies, arbitrary missed lifetimes and original pixel equivalence remain
pending. Evidence records below retain their original source hashes.


- [explicit evidence](opengl-command-recovery-metadata-explicit-20261006.json)
- [channel evidence](opengl-command-recovery-metadata-channel-20261006.json)
- [checkpoint evidence](opengl-command-recovery-metadata-checkpoint-20261006.json)
- [host evidence](opengl-command-recovery-metadata-host-20261006.json)
- [queue evidence](opengl-command-recovery-metadata-queue-20261006.json)
- [game evidence](opengl-command-recovery-metadata-game-20261006.json)
