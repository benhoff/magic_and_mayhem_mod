# Rendering source closure integration — 2026-10-25

This owned batch combines the storage-session closure work in `cb3bfd33` with
committed master changes through `7ea9fc19`. It investigates the earlier
failure 18/tag 52 colour-rectangle recorder refusals at original `0x00547d80`,
called by the `0x0054ab60` shape wrapper. Original drawing stays active;
source-only native replay and original GPU/default/physical comparisons remain
independent. No web UI or gameplay rule changes are part of this batch.

## Reproduction and adapter changes

The [verified historical workload context](rendering-closure-integration-results-20261025/old-workload-context.json)
locates the first of 375 baseline03 refusals in World 13, before the old Save
checkpoint at World 15. A fresh short merged baseline reached Save at World 4
and could not exercise that later scene branch. Its 204 native/source/default
frames agree; first-shrink physical ordinal 146 retains the 301992-pixel negative
control. A separate explicit upload correction matches all 213 frames. Those
results precede the final recorder changes and retain their own fingerprints.

The longer [negative acquisition](rendering-closure-integration-results-20261025/extended-04.json)
uses 16 observed returns per phase. Its bounded
[32 refusal samples](rendering-closure-integration-results-20261025/extended-04-refusals.json)
all identify readable 2×2 RGB565 triple-WORD source planes in mode 0. The trace
recorder's uniform-only admission rejected the nonzero plane. These are actual
new source observations, not retrospective assignments to the historical
failure-only rows. The longer acquisition later fails its after-Load dwell;
it grants neither full source closure nor native parity.

Small constants and readable source planes now use the existing owned colour
operation 26, with dimensions 1..64 and modes 0..3. Large uniform mode 0 copies
retain the existing fill representation and 2048 dimension bound. Unsupported
formats, dimensions, channel ranges, unreadable spans and source/destination
plane aliases remain refused. The existing native pixel/replay implementation
already supports the bounded colour operation; it is not replaced here.

An intermediate repaired acquisition closes its source stream with zero source
failures. Native replay then independently
refuses a 1×1 constant represented as generic fill inside World. No pixel
comparison took place in that attempt. Routing small constants through the same
owned colour operation closes that adapter mismatch. The failed result is
retained in [fixed-baseline-05](rendering-closure-integration-results-20261025/fixed-baseline-05.json).

The optional initial dwell now observes 3..16 actual World returns before Save;
other phases retain their ordinary three-return dwell. This changes the test
workload, not original counters, simulation state or engine timing. The initial
all-phase extension and unsuccessful test/build attempts remain separate.

## Validation and remaining boundary

The [final source-bound summary](rendering-closure-integration-results-20261025/final-comparison-summary.json)
requires independent source markers, verified normal original Quit, complete
native comparisons and exact current source hashes for both runs.

| Final software session | Source records | Worlds | Owned colour draws | Presentations | Native/source/default differences | Physical differences |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Baseline 13 | 58826 | 29 | 1218 | 216 | 0 | 301992 at the explained shrink |
| Explicit corrected 14 | 57840 | 29 | 1222 | 224 | 0 | 0 |

Both sessions have zero source failures and verified process exit 0. Live
colour inputs include 1×1 mode 0 and 2×2 modes 0/1. Modes 2/3 and other geometry
retain request/isolated controls, rather than fresh live comparison credit.
Physical comparisons allow RGB channel tolerance 1 and exclude alpha.

The [final controls](rendering-closure-integration-results-20261025/all-controls-12.json)
pass six source-closure refusal boundaries, 38 Python cases, actual PE32
forwarding/ABI checks and eleven colour-refusal fixtures. These include format
refusal before alias admission: diagnostic source planes and shape bytes
intersecting known destination storage are omitted. Unknown bindings omit
source sampling. A source closure marker alone does not establish pixel parity.
Observed software-EGL intervals cannot establish hardware scanout, all-writer
ownership, complete scene/branch coverage or live replacement. The historical
failed streams retain their original hashes and refusal status. The all-phase
16-return experiment still has an unaccepted after-Load timeout; the final
initial-only extension does not claim to repair every longer session.

[Recorder refusal format](../formats/producer-colour-refusals-v1.md) records
source metadata only. At most 32 samples are written per process; sample
exhaustion or missing diagnostics cannot admit a failed session. Aliasing source
planes are refused without diagnostic reads from destination storage.

Each generated result is retained through the SQLite evidence writer. The
[initial committed-history audit](rendering-closure-integration-results-20261025/history-audit-01.json)
checks 1068 exact file-version pairs and identifies two raw-artifact receipts
missing from the earlier commit. SQLite receipt
`RCI25.retrospective-parent-raw-artifacts` adds their exact identities separately;
it grants no past gate pass or new engine validation. The
[review through the final master base](rendering-closure-integration-results-20261025/history-audit-04.json)
checks 1229 committed file versions with no unresolved exact receipts.


## Retention and integration scope

The verified [raw catalog](rendering-closure-integration-results-20261025/raw-catalog-final.json)
retains 30012 unique members and 88210 exact byte aliases in SQLite. The
[execution-tool catalog](rendering-closure-integration-results-20261025/execution-tools-catalog-final.json)
separately restores 14 native/reference binaries and build records; replay,
physical-peer and scene-DLL hashes match the final executions. The host
compiler/shared libraries remain external dependencies. Temporary private RAM
capacity grew 24→32→40 GiB to retain earlier negative attempts and final outputs;
it is not durable evidence storage. Durable SQLite verification and retained
backups precede releasing that namespace.

The screen-recovery merge at 233af3b3 changes none of the source paths
fingerprinted by the final rendering evidence. The later terrain-light merge
at 7ea9fc19 changes seven terrain reconstruction fingerprints. The
[fresh integrated replay](rendering-closure-integration-results-20261025/master-comparison-summary.json)
rebuilds native replay and physical composition against the unchanged, verified
original captures. Both complete cohorts retain the results in the table above.
It records a new native execution; it does not claim a new original run or
refresh the older evidence hashes. The
[final committed-history review](rendering-closure-integration-results-20261025/history-audit-05.json)
checks 3498 exact file versions with no unresolved receipts. This batch supplies
no new original execution validation to unrelated screen, terrain or launcher work.
