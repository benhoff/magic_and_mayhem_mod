# Actor installed-world dependency cases

The Critical `RS.actor-downstream-services` parent remains partial. Its broad
references to missing task handlers, font rasterization and particles were too
coarse: all fifteen handlers selected by `behaviorCallback` have scoped native
implementation leads, and later batches recovered font, support and emitter
families. Their existing proofs do not compose them into the installed actor
session. This review identifies the remaining integration cases; it supplies no
new original execution or installed-world acceptance.

## Required cases

These stable case IDs belong to the existing parent contract. Dependencies are
supplier leads within their recorded domains, not whole-function endorsements.
`tools/audit-actor-dependencies.py` contains the precise acceptance requirements
and resolves the current register and historical frontier into SQLite.

| Case | Required closure | Existing bounded supplier | Evidence still required |
| --- | --- | --- | --- |
| AD.T1 | World clock and owner allocation | `AI.owner-brain-count-driver`, `AI.owner-task6-scheduler` | Real World order, zero/odd/exhausted allocations, disabled/retained phases and expiry. Count-mode call-site proof excludes timing/rdtsc mode. |
| AD.T2 | World-produced sources and task recipients | Actor/source session and knowledge-cache ownership | One shared World drives registration, cache invalidation, installation, handoff, retirement and protected slot reuse through restart. Authored recipients and observation markers are insufficient. |
| AD.R1 | Installed profiles and ANI/SPR publication | Concrete frontier and profile-transform providers | Declare the supported installed profile set; bind actual file/profile services to activation/events/retirement; compare missing/short/allocation/reload prefixes and ownership. |
| AD.R2 | Font/palette/surface lifetime | Raw font and palette-builder providers | Load and publish installed resources through actual bind/unbind/restore/reload and teardown. Compare partial publication, failed allocation and retained reload buffers with a chosen platform adapter. |
| AD.C1 | Shared-state behavior callbacks | All fifteen selector targets have recovered component leads | Explicit actor/map/profile/resource adapters invoke the bodies from `50aa30`; compare published callback/state, deferred and nested transitions, death and special handlers. Unsupported-kind/null admission stays explicit. |
| AD.C2 | Reentrant retirement and generations | Actor/source ownership and emitting-update composition | Release, append, rejection, eviction/reload callbacks during installed iteration; high-water rereads, cleanup order, protected reuse and eventual conservation of owned generations. |
| AD.C3 | Scenario/spell/subtype targets | Service composition and concrete frontier | Establish targets actually reachable in the selected scenarios, then compose evaluation/action/finalization and effects. Preserve unassigned endpoints and incomplete indirect target sets. |
| AD.P1 | Particle production and cadence | Emitter-family and emitting-update composition | Actual World clock/producers, installed terrain/animation resources, separate RNG streams, reverse ticks, plane allocation failure and child retirement. Preloaded terrain and synthetic controls do not close it. |
| AD.A1 | Audio admission/retirement timing | Value/dependency services and scheduler composition | Actual actor/scheduler/backend sequence through activation, events, release and restart; nullable handles, status/stop/position callbacks and failure lifetime. |

Close each case only with a fresh original/native comparison of its stated
composition and source closure, followed by the declared installed-session
acceptance. Resource or callback admission limits must be explicit. Do not
replace original timing with native lab cadence or treat a scalar/logical
projection as a raw-storage adapter. Preserve statuses independently for
understanding, implementation, comparison, integration and live replacement.

## Callback evidence distinction

The selector's kind/address pairs are:
`1/50acb0`, `2/510010`, `3/50afa0`, `4/50ccd0`, `5/50d580`,
`6/50d840`, `7/50d860`, `8/50d850`, `9/50d870`, `10/50d8f0`,
`11/51c510`, `13/51c230`, `14/521750`, `15/521770`, `16/521780`.
These are pinned NoCD build identifiers, never persistent process addresses.
The supplier families include `AI.behavior-handlers`, `AI.route-displacement`,
`AI.corpse-lifecycle`, `AI.autonomous-behaviors` and the narrower composed
`AI.owned-command-behaviors-chain`. Their input projections and lower World
services retain their documented boundaries. The actor transition router still
calls a supplied `m.invoke` endpoint. Address matching establishes a reuse lead,
not a compatible storage model or an already integrated callback.

The installed native encounter is a separate native policy: two fixed type17
actors on a sealed map crop with explicit empty World services. It does not
provide original dynamic construction, owner cadence or arbitrary profiles.
Likewise the fallback and task6 ownership batches do not close all actor behavior
callbacks. Font and emitter body counts do not prove upstream World production.

## Accounting and reproduction

The immutable [case inventory](actor-installed-dependency-results-20261026/cases01.json)
preserves the reviewed register fingerprint, all 112 endpoint specifications
from the historical 275-route frontier and its deduplicated indirect sites.
Each real endpoint gets current build-specific component leads or an explicit
absence. Authored `1001xxxx` fixture tokens remain distinct. Those specifications
are neither a missing-function count nor an exhaustive current call graph.
Historical execution hashes remain unchanged; component links grant no new
freshness or status credit. Confidence is high for this static selector/link
accounting; full required composition remains pending.

```sh
python3 -B tests/test-actor-dependencies.py
python3 tools/audit-actor-dependencies.py \
  --output research/runtime/actor-installed-dependency-results-20261026/cases-new.json
```

This producer reads source and SQLite research evidence only; it does not consume
original artifacts. Its output is a SQLite reference written directly through
`coverage_evidence.write_report`. Reruns require a new output identity. Platform
adapter acceptance, physical legacy driver equivalence and live replacement are
separate milestones. Malformed/indeterminate original inputs, Windows SEH and
throwing cleanup require explicit exclusions or independent evidence; they are
not silently treated as completed normal-return cases. No balance changes occur.

## Master integration review (2026-10-27)

The initial combined tree starts at master `0357068d` and merges actor commit
`6f06d27c`. The fresh [merged inventory](actor-installed-dependency-results-20261027/cases-merged01.json)
retains nine pending cases and fifteen selected callback implementation leads.
All engine validation/status dimensions and historical source hashes remain
unchanged. The four dependency accounting guards pass on the combined sources.

The [initial history review](actor-installed-dependency-results-20261027/merge-history01.json)
retains the missing exact-receipt findings. Its
[resolved review](actor-installed-dependency-results-20261027/merge-history02.json)
accounts for 12 commits and 8,581 parent/current file transitions over
`8c0c4ea4..0357068d` and `8c0c4ea4..6f06d27c`. Nine retrospective receipts
cover 361 previously missing exact pairs, including inherited source and raw
artifacts. These reviews are separate from uncommitted changes and assert no
past gate pass or fresh execution. Original pointer hashes are preserved;
payload integrity is checked by the full merge gate. Retained alternate
integration backups supply the historical workspace/raw archive payloads
locally, without publication or large duplicate copies.

The unrelated staged native-surface batch in the main checkout is excluded
from this merge. Local SQLite backups retain this merge's evidence and review
workspace; the cold archive is left untouched at its storage reserve.

The integration target was then refreshed to master `432d8b36`;
its additional committed range is reviewed separately before final validation.

The additional master range `0357068d..432d8b36` covers two commits and
1,270 transitions; two pending retrospective receipts account for thirteen
missing exact pairs. The source-reviewed
[initial report](actor-installed-dependency-results-20261027/history-initial-reviewed01.json)
and [additional report](actor-installed-dependency-results-20261027/history-refresh-reviewed01.json)
retain their prior reports and include explicit tooling fingerprints.
An initial audit rejected a draft static-evidence definition lacking the required
source-map pointer; [that failure](actor-installed-dependency-results-20261027/audit01.json)
is retained. The corrected definitions add no engine validation credit.
