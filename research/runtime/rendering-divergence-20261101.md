# Current rendering divergence — 2026-11-01

The reported high-resolution failure was caused by **original-reference phase
reconstruction**, not native rendering. Starting at `08f813e1d`, this batch
isolates the first differing operation, corrects the oracle and rechecks both
retained configurations. Native renderer, capture protocol, input, admission,
watchdog and gameplay behavior are unchanged.

The [review](rendering-divergence-results-20261101/final-review11.json) binds the
exact sources, controls and witnesses. The earlier failed reports and their
original hashes remain immutable; this finding supersedes their interpretation.

## Isolation and cause

The [source-only baseline](rendering-divergence-results-20261101/high-baseline01.json)
replays all53 Worlds through separate native CPU and GPU histories. Their full
planes match each other throughout. Both first differ from the older original
oracle at World50, with295 pixels, followed by1,747/1,652/1,638 at Worlds51–53.
Thus the discrepancy is present before Qt viewport composition.

Per-draw SHA-256 checkpoints locate the first difference at World50/draw83,
producer record164, backend9, original entry `0x596490`. The preceding82 draw
checkpoints match. The sprite has nominal top−19 and clip top0, so19 source rows
are skipped. The [selected witness](rendering-divergence-results-20261101/high-selected05.json)
has byte-identical original/native entry planes and493 differing output pixels
immediately after that draw. Later draws hide some differences, explaining why
the final World has295 rather than493 differing pixels.

Both capture paths already normalize the original displacement table to absolute
destination rows:

```
effective[j] = raw[(j + phase + 1 - skipped_top_rows) & 15]
```

The newer helper in `tests/world-raster-reference.hpp` installed that effective
table with phase `UINT32_MAX`, which passes the unclipped controls but fails the
clipped cases here. The original
routine then applied the skipped-row adjustment again. Installing phase
`skipped_top_rows - 1` restores the effective table correctly. The older
`tests/world-frame-reference.cpp` already does this. The repair applies the same
reconstruction in the newer helper; it changes no native sampling rule.

This is a confirmed finding with high confidence within the supplied contexts.
It is supported by the captured draw, existing capture normalization, an older
independent reference implementation and actual unmodified original execution.

## Regression and fresh comparisons

The [before regression](rendering-divergence-results-20261101/phase-before06.json)
compares raw original table/phase calls against replay from independently
normalized wire records. It tests both `0x596490`/`0x5968a4`, all16 phases,
negative/positive sprite origins and nonzero clip tops. All384 clipped cases
fail; all192 unclipped cases pass. The [corrected regression](rendering-divergence-results-20261101/phase-after07.json)
passes all576 cases and2,359,296 compared words.

The [corrected draw trace](rendering-divergence-results-20261101/high-corrected-draws10.json)
matches all1,830 World50 checkpoints. Its selected original/native before and
after planes match exactly. Native witnesses are byte-identical to the pre-fix
witnesses. The corrected original output differs from the older retained oracle,
as expected; the older oracle remains retained as negative evidence.

Fresh original histories are then rebuilt from each complete owned producer
prefix and source assets, without native output or original destination captures
as oracle inputs. The immutable actual Qt swaps are compared again:

| Configuration | Original Worlds / draws | Presented Worlds | Compared pixels | Differences |
| --- | --- | --- | --- | --- |
| [800×600](rendering-divergence-results-20261101/high-recomparison08.json) |53 /97,486 |103 |165,997,240 |0 |
| [640×480 software](rendering-divergence-results-20261101/low-recomparison09.json) |84 /112,200 |148 |212,644,040 |0 |

Original Right down/release acknowledgements and actual fullscreen intervals are
revalidated from the same captures. Native engine/capture sources, assets and
the original executable remain exact. Fourteen normal-admission and10 pilot
controls also pass. This reruns the comparisons with fresh mapped-original
execution; it does not launch a new native game process.

## Boundaries and reproducibility

This closes the reported13,659-pixel high03 discrepancy as an oracle defect.
It does not close the older historical source `(563,433)` writer/definedness
question, the original startup/watchdog questions,18 EP06 operation acceptance
requirements in both configurations, whole-player verdicts or live replacement.
The older one-pixel historical investigation uses the already-correct older
reference helper and is not invalidated by this fix.

The first draw-trace attempt miscounted pre-World raster records against the
World-only descriptor list; the next attempt had a compiler warning in the
diagnostic watcher. Both failures and their exact source preimages are retained.
They are harness failures, separate from the measured oracle defect.

Use `tools/diagnose-rendering-divergence.py` for source-only CPU/GPU and optional
selected original draw tracing; `tools/test-distortion-oracle-phase.py` for the
raw-original regression; and `tools/recompare-rendering-divergence.py` for fresh
independent history over retained captures. Every producer requires new report
and ignored execution paths and uses the machine storage guard. Reports and
raw witnesses are SQLite references. Physical JSON exists only as bounded
producer/consumer interchange under `working/`. No original output seeds native
history, no historical hash is refreshed, and no nominal acceptance is claimed.

The [coverage audit](rendering-divergence-results-20261101/coverage-audit13.json)
has no errors; all new sources are indexed and linked, and all923 new generated
research references resolve through verified SQLite payloads. The
[summary](coverage/summary-rendering-divergence-20261101.md) records current scoped
tool support separately from existing historical validation warnings. The
[history review](rendering-divergence-results-20261101/history-review12.json)
checks2,372 exact transitions in `a4d3a6ee3..08f813e1d` with no missing receipts.
Draw tracing uses32-bit libcrypto for SHA-256; the compilation logs and binary
hashes are retained. Completed offline directories can be restored through
`tools/evidence-workspace.py` before any later physical consumer.
All10 completed offline directories, including failed attempts, passed real
restores from the two verified stores before their physical duplicates retired.
This reclaimed1,252,700,160 allocated bytes. Source checkouts, assets, build inputs,
review databases and the retained payloads remain preserved.
