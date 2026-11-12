# Rendering integration and extended storage closeout — 2026-10-26

This batch combines the colour repair in `346ce885` with master through
`c5137b9c`, including cursor ownership, terrain-light sessions and the later
pixel-caller investigation. The latter adds research and three Python files;
[exact source review](rendering-closeout-results-20261026/incoming-source-impact.json)
finds no change to any fingerprint in the passing rendering comparisons.
No web UI layout or gameplay balance changes are included.

## Current original comparisons

The [fresh colour comparison](rendering-closeout-results-20261026/colour-comparison-01.json)
repeats the previous 4352 CPU pairs and 384 extended original/native cases.
The extended cases exercise all four modes, RGB565/RGB555, constants/planes,
clipping and dimensions through 2048×2048. The CPU matrices compare 90,011,152
physical WORDs and recorded caller/source state. Forwarded or hidden controls
remain separate from native admissions. All 192 ordered RGB565 World fixtures
match the original across 38,199,120 pixels; five refusal controls and ordinary
scene/session regressions pass. This renews the bounded comparison on the
combined source tree, including the recorded World constant-fill repair.

The original image remains pinned to No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The original manifest bookends verify 2927 immutable files. These isolated
fixtures establish neither natural live mode reachability nor live replacement.

## The after-Load capture deadline

The old exploratory acquisition required sixteen World returns in each phase
but allowed only thirty seconds. The controller now accepts explicit bounded
per-phase return counts and deadlines, records actual counts and elapsed time,
and refuses process death, regressed counts or missing returns. Larger deadlines
supply no records and bypass no source/normal-Quit gate. Existing defaults remain
three returns and thirty seconds. The extended profile explicitly selects:

```
--storage-initial-worlds 16 --storage-phase-worlds 16 --storage-dwell-timeout 120
```

Both new original-active profiles complete Save, Load, both display rebuilds,
Main Load, source-session closure and independently observed MainQuit/process
exit. Each records sixteen fresh returns in all four phases and 68 total World
returns, with zero source failures. The
[accepted session summary](rendering-closeout-results-20261026/extended-session-summary.json)
retains exact original/native record hashes.

| Software profile | Source records | Presentations | Native/source/default differences | Physical differences |
| --- | ---: | ---: | ---: | ---: |
| Unmodified-upload baseline | 121343 | 271 | 0 | 126 pixels at the independently explained shrink |
| Explicit corrected-upload policy | 117554 | 262 | 0 | 0 |

The baseline remains a successful negative presentation control, not accepted
physical parity. Physical comparisons use RGB channel tolerance 1 and exclude
alpha. The correction is an explicit upload policy, not an unmodified-original
oracle. Original oracle pixels never seed native replay.

After Load, the sixteen-return dwell takes 39.574/40.665 seconds. After recreated
Main Load it takes 87.117/69.021 seconds. These observed intervals exceed the
old thirty-second deadline. Confidence: confirmed that the bounded harness
needed a longer deadline in these profiles. This does not establish an original
Save/Load defect or change original simulation timing. Historical failed runs
remain failed. Arbitrary longer sessions and different machines need their own
bounds and evidence.

## Natural colour-input coverage

The campaigns record 5204 actual colour operations: 1×1 mode 0 and 2×2 modes
0/1. Neither campaign emits a generic fill inside an active World.

A separate stock map-2/three-item Quick Battle completes normal original Quit.
Its [fresh resumed native comparison](rendering-closeout-results-20261026/natural-quickbattle-04-native.json)
matches all 333 source/default/physical presentations. Its source stream contains
no colour-rectangle calls. The earlier native attempt was stopped by the storage
guard and remains unaccepted; the fresh replay reuses the unchanged, completed
original acquisition and grants no retrospective success to that partial run.

Sixteen finite original World samples from stock map 1 with fifteen items,
after 120 queue calls at four-call intervals, contain 19036 raster draws and no
colour rectangle. Their original capture is accepted; this finite stop is not
normal MainQuit or complete journey evidence. The wrapper's duplicate printed
path error is retained separately from the
[recovered original record](rendering-closeout-results-20261026/natural-busy-02-original.json).
A subsequent kind8-specific comparison refuses this selection because its first
sample contains no required additive/colour operation. That strict fixture
refusal remains negative evidence; no complete native comparison of these
sixteen samples is claimed.

The [natural-input index](rendering-closeout-results-20261026/natural-input-summary-02.json)
is read-only classification of those separately retained executions. It grants
no native CPU splice implementation, comparison or integration credit. Modes
2/3 remain unobserved in these profiles, not proved unreachable. The isolated
original comparisons cover their arithmetic; natural invocation, active-World
large fills, RGB555 World ingestion, all-writer ownership, hardware presentation
and whole-renderer/live replacement acceptance remain separate gaps.

Across the two extended sessions and accepted Quick Battle replay, 866 complete
presentations have matching native/source/default output. The baseline's 126
physical mismatches remain explicit; the other 595 physical presentations match.
The previous 216/224-frame cohorts retain their historical fingerprints. These
new cohorts supply fresh bounded validation rather than rewriting those records.

## Accounting, controls and retention

[Focused controls](rendering-closeout-results-20261026/controls-01.json) pass six
source-closure cases, nineteen storage-controller cases, eleven cadence cases,
three colour-refusal cases, ten frontier guards and sixteen crosswalk guards.
The new deadline controls require actual delayed returns and reject missing
progress, process death and regressed counts.

The [committed-history review](rendering-closeout-results-20261026/history-02.json)
checks 3839 exact file transitions across the incoming master/colour ancestry,
with no unresolved receipts. The initial review found 380 terrain-merge binary,
log and image artifacts outside the ordinary source census. Exact payload
verification and retrospective receipt
`RCL26.retrospective-terrain-merge-artifacts` supply their accounting only; this
asserts no historical gate pass or new terrain validation.

Generated results use SQLite references. Raw successful and failed executions,
exact helper variants and binaries are retained through the raw plan/catalog;
unchanged installation files alias the verified restored fixture archive.
Verified SQLite backups precede temporary RAM teardown. The task-local evidence
store was moved to the secondary drive, then returned to main after main storage
recovered and the secondary approached its100 GiB floor. Exact verified copies
and both durable location catalogs preserve that provenance; final verified
backups use main storage. The user explicitly approved a temporary
88 GiB main-drive floor for this follow-up closeout after the normal 90 GiB guard
stopped the first Quick Battle replay. The secondary 100 GiB floor is unchanged.
No evidence publication or source push is part of this batch.

The [updated frontier](rendering-closeout-results-20261026/frontier-01.json) and
[scenario index](coverage/summary-rendering-closeout-20261026.md) retain790
rendering contracts and675 scenarios, with661 lacking current selected integration
proof at their declared boundary. These overlapping counts include shared-source
staleness and static discoveries; they are not separate defects or a completion
percentage. The read-only crosswalk snapshot passes a reviewed definition to the
existing reconciliation API: twelve surface categories, nine drawing routes,
and the fresh RCL26 CPU subset alongside historical CCF25 evidence. The generic
CLI's prior default definition remains unchanged. Its exact input register and
helper sources are retained; this static snapshot grants no execution credit.

The later screen-ownership merge `bdf87f55` is included in the final combined
tree. The [source-impact check](rendering-closeout-results-20261026/screen-merge-source-impact.json)
verifies that all four retained rendering comparison source fingerprints remain
exact. Its new resource-ownership models retain their independent evidence;
these rendering runs grant them no additional validation. The
[follow-up history review](rendering-closeout-results-20261026/history-screen-04.json)
checks the additional committed versions before the combined-tree census and gate.

That review covers 5648 committed file versions. Thirty-eight incoming screen
logs/images required exact retrospective artifact receipts; every payload was
verified. The initial incomplete result is retained as `history-screen-03.json`;
the final result has no unresolved receipts. This asserts no past gate pass.
