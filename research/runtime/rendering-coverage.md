# Rendering coverage against the binary inventory

Reviewed 2026-10-10 for the pinned No-CD build. The
[crosswalk](rendering-coverage-scope.json) reconciles the
[drawing inventory](render-drawing-inventory.md), the
[surface operation matrix](native-surface-operation-matrix.json) and newer
bounded World replacement evidence. It introduces no drawing hook, bypass,
balance change, status promotion or refreshed historical evidence hash.

Run from the repository root, using new output paths:

```sh
python3 tools/report-rendering-coverage.py \
  --json working/tests/rendering-coverage-next.json \
  --markdown working/tests/rendering-coverage-next.md
```

The command reads repository accounting and retained evidence only. It never
opens original media, installed assets or executable bytes. It verifies selected
evidence record hashes, assertions, build identity and bypass outcomes, then uses
the existing coverage audit for source/scope/scenario-bound current validation.
An audit failure still produces an explicitly historical report, suppresses all
current-support counts and exits 1. Invalid crosswalk/evidence inputs exit 2.
Existing reports are never overwritten. A successful report is accounting, not
a newly executed rendering comparison.

## Denominators and interpretation

The twelve required surface categories and nine inventoried drawing families
are separate denominators. Four conditional surface categories remain visible
as discovery backlog. Adding a required matrix row or drawing family requires
reviewing the crosswalk; the tool refuses to silently shrink or widen its scope.
Producer families overlap and share backends, so their counts cannot be added to
surface counts or treated as proportions of runtime work.

Every row names scoped implementation contracts, selected validation evidence,
the evidence domain, any actual live bypass and remaining boundaries. A category
counts as having scoped code when at least one mapped contract has a scoped
implementation. Partial-only code is reported separately. These are **categories
with some support**, not fully completed categories. Full release readiness stays
under `NR.live-drawing-replacement` and DRAW-01 through DRAW-07.

Independent original-executable comparison and standalone Wine driver comparison
remain separate. A driver fixture does not promote original-game comparison.
Captured pixels, shadow presentation and native policy do not count as bypass.
Original retry scheduling comparison does not establish physical lost-draw pixel
recovery. Effect motion/lighting/animation foundations do not establish the
complete effect-to-queue-to-pixel producer route. Native Qt UI and media plumbing
do not establish complete original UI or enabled-movie replacement.

The reviewed historical crosswalk currently gives:

| Denominator | Some scoped code | Independent subset validation | Original-executable subset comparison | Some scoped live bypass |
| --- | ---: | ---: | ---: | ---: |
| Required surface categories | 12/12 (100%) | 12/12 (100%) | 6/12 (50%) | 0/12 (0%) |
| Drawing-route families | 7/9 (77.78%) | 7/9 (77.78%) | 7/9 (77.78%) | 2/9 (22.22%) |

Movies add one partial-code family, bringing any-code support to 8/9. The two
families with scoped live bypass are sprites and UI, both using the same bounded
direct-word route evidence; they are not two independent complete takeovers.
The absence of a live surface-category bypass claim does not negate the distinct
World CPU-raster-body bypass described below. All categories retain gaps.

These figures replace the unsupported overall 50% estimate. They do not estimate
remaining engineering time, full branch coverage or whole-game completion. Use
the generated JSON to inspect the precise proof and remaining scope for each row.

## World replacement reconciliation

The thirteen admitted World raster entry addresses are an explicit experimental
whitelist, not the entire binary raster-dispatch space. Each selected execution
reports its actually exercised addresses from immutable `actual_entry_counts`:

| Execution | Exercised/admitted entries | Complete startup queues | Compared original calls/AX |
| --- | ---: | ---: | ---: |
| Scalar boundary prefix, 2026-10-08 | 10/13 (76.92%) | 1–16 | 36,556 |
| Guarded batch prefix, 2026-10-08 | 10/13 (76.92%) | 1–16 | 23,480 |
| Identity-cache batch, 2026-10-09 | 9/13 (69.23%) | 1–16 | 26,146 |
| Owned-source batch, 2026-10-10 | 10/13 (76.92%) | 1–32 | 63,124 |

The smaller exercised set in the identity-cache run reflects its inputs, not a regression.
Runs retain their individual source versions and are not pooled into a new
current-code claim. The newer owned-source execution compares all 63,124 precise
raster canvases and caller AX values, 1,224 completed checkpoints and 32 live
finals with independent original execution. Its source reuse removes the earlier
128 MiB journal-budget failure; the retained journal is 12.87 MiB. Software Mesa
and finite startup remain its scope. The recorded median queue time is 223.83 ms,
so this is not real-time continuous gameplay evidence. Queues beyond thirty-two, all indirect/alternate routes,
outside-World producers and full supported-session behavior remain open.
Original World traversal/simulation and non-drawing preparation still run.

The initial reconciliation run preserved then-existing workspace limitations:
the pending `world-raster-batch-prefix32-20261010` scenario lacked execution
evidence/test admission and the source census had link drift from concurrent
work. Current readiness was unavailable until those independent accounting gaps
were reconciled. The new queue32 live/replacement records now reconcile that
scenario; the initial failed records and earlier source hashes remain intact.
The bounded [family regression runner](drawing-family-validation.md) separately
executes the available tests for every inventoried family without widening any
producer replacement claim.

## Maintenance

Review mappings and domains against original research before adding credit.
Add new evidence under a new ID, preserve old hashes and require matching scope,
sources and scenarios for current claims. The report never changes the register.
The crosswalk intentionally lists bounded proof rather than treating every
evidence record attached to a broad behavior as proof of every operation.

```sh
python3 -B tests/test-rendering-coverage.py
python3 tools/audit-re-coverage.py
python3 tools/check-re-coverage.py --base HEAD
```

Tests guard denominator changes, native-policy/observation distinctions,
standalone-driver separation, stale/current claims, evidence corruption,
unlinked proof and unobserved/duplicate raster entries. Current workspace audit
failures remain separate from the reporter's synthetic correctness checks.

### Clipped pixel follow-up — 2026-10-10

The reviewed sprite row now includes `RS.word-clipping`: 960 synthetic GPU
placements match both original backends at zero clip globals. This adds bounded
pixel evidence without increasing the number of families with live bypass.
The sixteen reporter guards also protect `original/` and `.git/` when those
directories are symlinked outside an isolated checkout; no input files are
written by that check. Prior reports retain their recorded source hashes.
