# Tranche 1 implementation state and UI wording — 2026-10-19

Tranche 1 now applies the same definitions as tranche 3. The source assessment is
**nine implemented within scope, zero concrete partial-code gaps identified, and
two unresolved aggregates** (palette resources and checkpoint attachment).
Output ownership is implemented within its containment scope and separately
retains **acceptance failure**. The original pilot failed with “Native stress
viewport unavailable”; concurrent follow-up repaired that harness and now records
a default-run CPU-history transfer failure. Later evidence and next steps are
preserved separately from the original two-attempt pilot record.

This is a source, scope and planning review. It executes no original game or
rendering experiment and promotes no registered engine stage. Confidence is high
for the named mechanisms and delivered controls. Broad aggregate residuals,
current-source equivalence and the cause of the failed pilot remain unresolved.
Historical reports, source fingerprints and the ten bounded PASS / one pilot
FAIL outcomes in `rendering-tranche1-execution-20261019.json` remain unchanged.

The four definitions are:

- **Implemented within scope:** code covers the declared contract; validation
  may remain pending, historical or failed. Explicit exclusions do not imply
  partial code.
- **Partially implemented:** name a concrete required branch, mechanism or
  supported domain that is missing. Missing evidence alone is insufficient.
- **Unresolved aggregate:** reconcile delivered services and narrower contracts
  before sizing or asserting missing code.
- **Acceptance failure:** retain the failed outcome separately. Diagnose its
  cause before attributing a code defect.

| Contract | Current state | Next work |
| --- | --- | --- |
| `NP.surface-final-word-read` | Implemented two-byte tail in both allocation routes; four boundary reads and 64 comparisons have retained PASS evidence. | Review source drift and renew controls when current validation is required. Live crash resolution is separate. |
| `NP.native-cpu-instruction-resume` | Implemented continuation/retry and protected refusal domains. Full meaningful GPR/flags/FX assertions are supplied. | Reuse the five-class and active-reader/FIFO controls; do not rebuild their architectural-state fixture. |
| `NR.world-source-region-epoch` | Implemented owner-local begin/end/reset, non-owner bypass and current-byte copying. | Reuse the delivered production-shared producer fixture. Permission caching is not content ownership. |
| `NR.resident-bind-admission` | Implemented bounded admission; baseline, tracker, reentry and timeout selectors already exist. | Renew the four current-source schedules and caller/FIFO/cleanup assertions when needed. |
| `NR.callback-publication-isolation` | Implemented worker publication outside callbacks, with delivered thread/ownership traces. | Renew trace and zero-callback-pump controls; arbitrary stop/failure scheduling remains separate. |
| `NR.resident-command-pressure` | Implemented bounded pause/resume, shared deadlines, reentry refusal and original-once forwarding. | Reuse deterministic/sanitized matrices and PE32 pause/stall/cancel/ACK controls. Whole-game latency remains unvalidated. |
| `NR.readonly-description-continuity` | Implemented GetSurfaceDesc observation bypass and conservative mutation invalidation. | Reuse eight supplied schedules; diagnose the older public GAP independently. |
| `NR.palette-resources` | Unresolved aggregate over delivered palette identities, wire/GPU resources and ordered property/lifetime services. | Crosswalk startup/borrow/QI/alias/owner domains before naming missing code. The two-thread collision fixture is already delivered. |
| `NR.command-checkpoint-attachment` | Unresolved aggregate over delivered complete-state checkpoint, command/host/drawing recovery and opaque ownership policies. | Crosswalk sustained recovery/external attachment/destruction domains. Capacity and abandoned-return controls already exist; arbitrary thread death is not proved. |
| `NR.displacement-finite-edge` | Implemented explicit retained-destination World edge policy with strict generic draws. | Renew retained-input/mask controls as needed; unknown original overread behavior remains separate. |
| `NP.native-output-ownership` | Implemented containment/backing admission; default acceptance remains FAIL after a separate viewport-harness repair exposed CPU-history transfer stage 13. | Follow the newer bounded diagnostic next steps; distinguish reply-readiness from tracker reacquisition before changing engine policy. A timing-altered traced pass does not clear the normal failure. |

The planning catalog records each assessment, typed next steps, registered
execution evidence, exact code witnesses and hashes of direct implementation,
test and document inputs. Source drift makes the assessment historical; these
fingerprints are reviewed inputs, not a full compiler dependency closure. The
catalog replaces stale “add fixture” instructions with delivered-control records
and the actual remaining scope or diagnostic questions. It does not refresh any
historical execution fingerprint or declare the aggregates complete.

The web UI now uses these labels on reviewed behavior tables, work summaries,
importance entries and detail dialogs as well as tranche cards. Unreviewed
entries say **Registered partial** and **Scope review pending**. The shared
status guide defines the four labels. Generic partial advice asks for scope
review and named missing code rather than assuming unimplemented branches.
Separate source-assessment filters select reviewed missing code, implemented
within scope, unresolved aggregates and acceptance failures. Historical register
counts remain distinct; an acceptance failure can overlap implemented code.

The prior HEAD history review through `0dc91365` still applies: no commit landed
before this change. Concurrent tranche-2 and campaign work is preserved. New
reports, screenshots, audit/gate results and exact receipts are retained under
`working/tests/rendering-tranche1-state-review-20261019/`. UI/control tests validate
presentation and planning semantics, not engine comparison or live replacement.

Verification: 21 planning/schema tests, 21 UI/HTTP tests, JavaScript syntax and
14 focused Chromium checks pass, including all four source-assessment filters,
the behavior table/dialog, historical outcomes, supplied-control text and mobile
width. The focused browser uses the actual frontend functions and real planning
enrichment; it does not assert a full current-dashboard backend reconciliation
pass. Source-staleness tests now assert correct historical/review-needed flags
rather than requiring all earlier task reviews to remain current after unrelated
source edits. No old fingerprint is refreshed to satisfy these tests.

Concurrent handoff: `native-ownership-viewport-fix-20261019.json` records a
completed viewport repair, passing isolation controls, stable-source normal-run
CPU-history transfer refusals and a separate traced journey. `success` and
`default_campaign_accepted` remain false; status promotions remain false. The
updated catalog points to that evidence and stage-13 diagnosis. Its incremental
source reviews are preserved. This UI change reads that research record only;
it does not claim to have executed those engine experiments or authorize another
run. The original failed attempts and their source fingerprints remain intact.
