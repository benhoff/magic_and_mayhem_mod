# Tranche 3 implementation state and next work — 2026-10-19

Tranche 3 now separates reviewed implementation state from current validation and
retained acceptance failures. The old selection was eleven Critical contracts
registered as partial; it did not identify eleven missing native implementations.
This static review inspected the declared behavior scopes, current code witnesses,
linked research, retained outcomes and task entrypoints. It runs no original game,
GPU comparison or pilot and grants no registered execution-stage promotion.

The task assessments and their reviewed source SHA-256 values are in
[`importance.json`](../../apps/coverage-ui/importance.json). Source drift makes an
assessment historical. These fingerprints describe reviewed inputs, not a complete
compiler dependency closure or renewed engine equivalence. Historical evidence
reports, claims, source hashes and registered statuses remain unchanged. The UI
uses the new assessment on tranche-3 cards and shows the existing registered
status separately in their details. Global stage counts retain that accounting.

The four labels have these meanings:

- **Implemented within scope:** code covers the declared contract. Validation can
  remain pending, historical or failed. Explicit exclusions do not make code partial.
- **Partially implemented:** name the concrete required branch, mechanism or
  supported domain that is missing. Lack of execution evidence is insufficient.
- **Unresolved aggregate:** reconcile a broad contract against delivered services
  before identifying and sizing missing code.
- **Acceptance failure:** retain the failed comparison or acceptance outcome on a
  separate axis. Diagnose its cause before attributing a missing implementation.

The reviewed breakdown is **ten implemented within scope, zero concrete partial
implementations identified, and one unresolved aggregate**. Two implemented tasks
also retain acceptance failures. Zero identified missing implementations does not
assert that the complete renderer is finished or that the unresolved aggregate
contains no gaps. Confidence is high for the cited mechanisms and explicit scope
boundaries; full original behavior, driver effects and current integrated outcomes
retain their own pending validation.

| Contract | Implementation assessment / retained outcome | Next work |
| --- | --- | --- |
| `NR.world-history-channel` | Implemented finite 1..16 journal, identity/reset/sequence refusal and ENDED draining. | Renew admitted/refused bounds and identity controls; map continuous suppliers separately. |
| `NR.world-live-history` | Implemented finite word-zero history and retained GPU completion. Historical prefix passes and zero-history mismatches coexist. | Renew synthetic continuity/reset/drain controls, then bounded installed comparisons when prerequisites permit. Unknown startup/HUD content is excluded here. |
| `NR.world-producer-handoff` | Implemented owned producer-history to GPU World handoff and native completion commit. | Renew independent finite completion comparisons using tranche-2 source-only inputs. Original-body bypass/caller effects belong to `RS.world-producer-bypass`. |
| `NR.directdraw-retirement` | Unresolved aggregate over delivered facade/storage/format/clip/DC/copy services. | Diagnose the failed pilot; reconcile required encountered domains before selecting missing code. |
| `NP.native-only-launch-contract` | Implemented admission, capability/receipt checks, terminal policy and shutdown. | Renew controls and a stable-source media/storage/quit matrix after the upstream live gate passes. |
| `NP.native-only-generation-restart` | Implemented retirement, old-process verification, fresh generation and preserved failure history. | Renew failure/stop/staging controls, then fault → restart → normal Quit. |
| `NP.pixel-forwarding-guard` | Implemented six-family forwarding/install refusal and counters. | Renew actual PE32 controls and report exercised versus installed routes; the historical storage journey exercises 17/43. |
| `RS.drawing-publication` | Implemented selected-route observer/parser; no native publication replacement claimed. | Renew selected-site guards. Scope original alternate-route, failure/Restore and caller-equivalence recovery separately. |
| `NR.drawing-storage-session-comparison` | Implemented comparator; **acceptance failure** at unchanged storage ordinal 170. | Diagnose complete source/destination/physical witnesses without dropping frames or changing tolerance. |
| `NR.recorded-hardware-session-parity` | Implemented explicit backend/driver admission; full parity retains **acceptance failure** from the same unmodified storage negative. | Renew every hardware cohort; separate harness success, matching profiles and failed full parity. |
| `NR.recorded-physical-frame-admission` | Implemented paint → swap → same-image ordinal admission. | Renew delayed/stale/missing identity controls and both backend cohorts; pixel parity remains separate. |

The old instruction to implement the rejected 510×180 default-placement descriptor
was obsolete: the October 11 default-offscreen policy already admits it. October
15 resource-state work and October 18 copy/history repairs further narrow the
old compatibility backlog. The tranche-1 pilot's “Native stress viewport
unavailable” result does not identify another unsupported descriptor or COM call.
A new implementation task needs a still-reproduced required request and an
independent original/native fixture; diagnosis alone remains an unresolved result.

The unchanged 238-frame storage comparison retains 126 GPU plus 126 physical
pixel differences at ordinal 170. Later source-GPU and destination diagnostics and
the separately corrected driver-policy profile do not erase that baseline.
Final software/NVIDIA marker suites retain only the same 252-pixel negative across
1,270,115,312 compared pixels; the regression harness expects that negative, while
full unmodified-reference parity still fails. It is counted on both affected
contracts, not as two independently discovered native defects. Successful frame
identity admission does not imply successful pixel parity or scanout.

Independent source review and component controls can proceed. Dependent live
integration/replacement remains held on the failed tranche-1 output-ownership
acceptance and the relevant tranche-2 source-history results. This review grants
no additional pilot attempt. Future execution must use new prospective claims,
stable source/input snapshots, storage reservations and original-manifest
bookends when consuming original artifacts.

The existing committed-history review through `0dc91365` remains applicable; no
new commit landed before this review. This change preserves concurrent tranche-2
and campaign work. Planning/browser checks validate the labels and independent
states, not renderer behavior. Audit, gate and exact receipt reports are retained
under `working/tests/rendering-tranche3-review-20261019/`; accumulated baseline
accounting debt is reported separately from this change.

Verification: 20 planning/schema tests, 21 UI/HTTP tests and JavaScript syntax
checks pass. Six focused Chromium checks render the production tranche functions
against the reviewed catalog and verify code counts, overlapping acceptance
failures, current-state text, typed next steps, definitions and separate historical
stages. The screenshot/report are retained under `focused-browser/`. The first
full dashboard browser attempt stopped on a task-freshness assertion during
concurrent edits; its retry could not start the server because the unrelated
`RS.world-initial-state` reconciliation no longer covered its changed links.
Those attempts remain failed, and no full current-dashboard browser pass is
claimed. The coverage audit recorded zero errors after indexing; the local
baseline gate retains accumulated history/validation debt.
