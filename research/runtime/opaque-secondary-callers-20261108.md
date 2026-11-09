# Opaque wrapper second-frame caller admissions

All four remaining ordinary raster continuations in the second-copy branch of
NoCD40209ca7 wrapper `0x57e8e0` are admitted with exact ten-byte signatures:

| Return | Tags | Operation |
| --- | --- | --- |
| `0x57e9ec` | 22, 36 | Direct WORD copy through slot `0x6903dc` |
| `0x57ea02` | 12 | Black sentinel draw through `0x595677` |
| `0x57ea29` | 13, 14 | Indexed copy through slot `0x6c4e3c` |
| `0x57ea40` | 17 | Clipped fallback through `0x57de00` |

The generator pins the complete 128-byte second-frame block from `0x57e9d8`
through the wrapper's RET12, including both indirect slots, both direct targets,
argument pushes, cdecl cleanup, black normalization, indexed AX1 fallback and
low16 AX OR. It retains all four entries even with an empty observed queue.
The ordinary production table now has 52 sites / 61 pairs; the empty-stream
static table has 21 / 23. Unknown PCs, wrong tags, missing or changed signature
bytes, sprite context, owner, readable-span and source checks remain strict.
The generic producer bypass allowlist is unchanged.

The [old-table reproduction](opaque-secondary-results-20261108/negative-02.json)
executes the complete original wrapper over 108,864 authored two-frame cases.
Every case encounters a rejected second-frame pair. All four sites / six pairs
are exercised. A rejected-call sentinel returns AX1 to continue inspecting the
black/indexed fallback route; this fixture records admission failures rather
than launching a failing original game process. Negative01 preserves a test
refusal-sentinel mistake that omitted the fallback branch; positive01 preserves
a fixed-mode run before a failed source-edit assertion had applied the fix.
Neither is a passing implementation result.

The [fixed-table comparison](opaque-secondary-results-20261108/positive-02.json)
passes all 108,864 cases with zero refusals and zero differences across
33,094,656 destination WORDs, including prefix/suffix canaries and row padding.
The domain contains every indexed/WORD first/second combination, both WORD
providers, both indexed providers, nine signed second-frame origin pairs,
two clips, nine X / seven Y placements, shades -127/0/127 and palette-chain
ordinals 0/1. Frames are nonempty 7x4 RLE on a 16x12 RGB565 canvas, stride 20.
The source vectors stay unchanged. Both canvases start from the same authored
seed; original output never supplies native input.

Only existing callee entries in a private mapped image are redirected after
verifying the original executable hash. The full original wrapper bytes remain
unchanged. Native callbacks execute the production pixel kernel through the
actual production caller predicate. Low16 results, cdecl cleanup, wrapper RET12
and EBX/ESI/EDI preservation agree. Another 524,288 executions compare the
unchanged wrapper against the recovered opaque model over all low16 return
values, high-word poisons, black/indexed fallback routing, mixed source types,
call order and the combined AX OR. Volatile registers/flags are outside scope.

The [primary control](opaque-secondary-results-20261108/primary-control-01.json)
preserves the earlier primary `0x57e957` fix with 21,384 original/native cases,
5,184 native fallbacks and 6,500,736 compared WORDs. The affected
[black/sprite controls](opaque-secondary-results-20261108/black-guards-01.json)
retain the complete black normalization domain and production/regenerated
signature, sprite ownership and protected-span refusals. Both production and
selftest PE32 observers compile. Exact sources, tested ELF/DLL/EXE bytes, logs
and failed attempts are SQLite-backed; compiler objects and RAM smoke builds
are reproducible temporary output.

Confidence is high within this finite domain. No original-game start, natural
second-copy witness, World590 replay, full volatile ABI, RGB555 or arbitrary
frame-layout proof is claimed. Historical hashes and failures remain unchanged.
No acceptance row or whole-player decision is promoted. The primary-only proof
remains scoped separately as `NP.world-opaque-fallback-caller`; the new scope is
`NP.world-opaque-secondary-callers`. The committed-history review from
9b0148db6 to 9da49daa7 checks 152 logical parent/current file transitions against
exact receipts with zero missing transitions; this is accounting, not a past
gate-pass or fresh engine-execution claim.
