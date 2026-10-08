# Every queue in the bounded startup prefix

`RS.world-startup-queues` covers observation at No-CD build
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
consumer `0x5002a0`. The journal captures queue rows before the observer's sample
limit, skip/interval filter, scene frame ownership or effective raster admission.
Thus an empty or invalid queue, an unselected queue, or a refusal has an explicit
entry. The 1..256-entry bound is intentional; it does not claim unbounded history.

Canvas startup and finite refusal diagnostics automatically enable a queue prefix
of `--samples` entries. `--startup-queues N` can extend it beyond the scene sample
budget. These modes require `--skip-queues 0 --interval 1`. Full scene samples,
effective World records and queue diagnostics remain separate: journal sample
zero means there was no saved scene, while a saved scene can still have a refused
World trace. Refusal reason 8 now includes every actual unsupported row/kind,
rather than only the reason number or the first offending entry.

Reproduction:

```sh
xvfb-run -a -s '-screen 0 1600x1024x24' \
  python3 tools/capture-scene-game.py --canvas-startup --world-frames \
  --world-lifetime --observe-world-refusals --startup-queues 16 --samples 16
xvfb-run -a python3 tools/test-startup-queues.py
python3 tools/inspect-startup-queues.py CAPTURE_DIRECTORY --queues 16 --output NEW_REPORT
```

The synthetic PE32 fixture validates eight consecutive entries, row-byte
preservation (all kinds 0..40, -2, -1 and signed extremes), empty/unreadable and
over-capacity inputs, entries beyond the sample budget, an unknown view, exact
prefix termination, and original EAX/LastError forwarding. Thirteen malformed or
missing-prefix cases are refused. It does not execute original raster routines.

The existing admission kinds are -2, 0..6, 22, 31 and 33. Original dispatcher
semantics are documented only for selected routes in
[native-world-frame-rendering.md](native-world-frame-rendering.md). Other actual
kinds remain numeric observations until their original branches are recovered.
Reason 7 concerns an admitted wave route's displacement state and is distinct
from unsupported queue kinds. No raster equivalence or live replacement is
promoted by this diagnostic work. Previous evidence retains its original hashes;
changes to shared observer/build/capture files leave older execution results
historical until their own scope is rerun.

The retained original execution
[native-world-startup-queues-live-20261008.json](native-world-startup-queues-live-20261008.json)
captured all 16 entries and all 21,888 raw rows, with no skipped queues. Every
entry correlated with its saved scene and effective World envelope. All queues
had view 0. Six entries were refused for these actual observer-unsupported kinds:

| Queues | Kind (decimal / hexadecimal) | Draws per queue |
| --- | --- | --- |
| 3, 4 | 20 / `0x00000014` | 1 |
| 5, 6 | 17 / `0x00000011` | 1 |
| 7, 8 | 16 / `0x00000010` | 1 |

Each report retains the offending row's ordinal, coordinates and frame pointer;
the binary journal retains every row, including the observer-admitted kinds.
Queue 1 separately refused with reason 7 after 473 recorded effective draws and
had no unsupported queue kinds. Queues 2 and 9..16 had no refusal. Original
drawing continued for all 16 entries; this is diagnostic observation, not native
admission or equivalence for the refused frames. Numerical kinds 16/17/20 have
not acquired recovered raster semantics from their observation alone.

The accompanying canvas trace contains 8,475 records and observes 365,338
nonzero pixels at the first queue. Differences from the earlier startup capture
are kept as separate execution evidence. The original manifest passed before
and after the run (2,927 files). Confidence is high for this prefix's raw queue
contents and correlations; other startup modes and unsupported branch behavior
remain pending.

The separately retained integrated run
[native-world-startup-queues-integrated-20261008.json](native-world-startup-queues-integrated-20261008.json)
binds prospective claims for the journal and all four canvas startup behaviors.
It captures all 16 queues and 30,612 rows, with the same unsupported kinds and
queue positions listed above. Its 8,513 canvas records confirm one creation,
one full-zero-fill request, no release/rectangle fill of the World wrapper,
686 returns of the same pixel pointer and identical last-copy/first-World sample
fingerprints. Per-run counts remain separate. The existing PE32 scene fixture
also passes with the journal disabled, preserving XMM/x87/MXCSR and LastError.
