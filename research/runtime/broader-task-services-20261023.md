# Broader task credit services — BTC1 (2026-10-23)

Pinned NoCD40209ca7. This batch replaces supplied task-ID/recipient/accounting
endpoints with concrete shared providers across item, resource, health and
completion callers. It does not introduce gameplay balance changes.

The new `OwnerTaskCreditServices` queries both signed-key task indexes using
416c30/42e610 semantics, follows selected actor source ancestry through522770,
and supplies real416d80 profile valuation. Existing EffectServices,Resource,
Maintenance and SegmentCompletion implementations remain separate and compose
through these services. Owned task/source construction and retirement remain
with their existing owners; the new provider borrows those records.

The compared scope includes activation467c80, item468470 healing/resource/FC
paths, nonlethal514990 damage, positive5076f0 healing, periodic509e50 resource
contribution and one-shot512460 completion. Reward2f and cost33 use DWORD wrap;
duplicate roots receive duplicate updates. This differs from the existing
logical task-counter API's separately documented overflow-refusal policy.

Task-index insertion/unbinding and source forests are authored fixture inputs.
Registration, UI/audio/scenario/effect allocation, item application/removal and
position/environment/animation/terrain providers remain explicit boundaries.
Ordinary inactive maintenance siblings, finite positive resource/health/capacity,
readonly profiles, successful allocation and non-reentrant task storage bound
this comparison. Native tree/source depth and domain guards are policies.

General task installation/scheduler, source split/merge, broader dual-brain
cadence, real item/World backends, lethal/scenario cleanup, arbitrary callbacks,
training, exceptional allocation/Windows SEH and installed/live replacement stay
open.

## Execution evidence

[Trial7](broader-task-services-results-20261023/broader-task-trial-7.json)
compares original32,native64 and ASan/UBSan/LSan across128 authored episodes
and1408 event steps. It prospectively binds all declared/compiler inputs and
checks their stability,original RX text and immutable manifests before/after.
All384 actual fallback tasks construct and retire normally,with guarded storage
and no admitted leaks. Both full packed actor records,task reward/cost DWORDs
and ordered supplied-service calls enter the comparison digest each step.

Task lookup covers2048 queries in both brain indexes,including negative IDs and
nonmatching lower bounds. All eight authored profile kinds,both source families,
absent/duplicate/ancestor recipients,positive healing cap/notify gates,ordinary
ranks0/1/2,wizard CFG/8c1 fallback,associated source/other actor choices,resource
callback mutation,rejected item application,unsigned deadline equality/controller
gates and pending completion consumed once are covered. Counter changes and
wrap counts are recorded in the report. Maximum-health commander-profile
selection and arbitrary rank/overflow domains are not newly validated.

Trials1/2 preserve fixture compilation failures. Trials3/4 preserve an ordered
callback mismatch:the native associated-actor provider incorrectly read owner
record+4;the original uses+12. Trace1 isolated the missing healing display call.
Trial5 corrects that field and passes. Trial6 broadens associated-actor and
maximum-health inputs but times out after activating bonus maintenance without
its backend configuration. An attached debugger found the original random-range
loop40e2ad with ECX0. Trial7 explicitly sets rank0 before periodic maintenance,
keeping its admitted siblings inactive;ranked health remains exercised by the
earlier healing/item steps. No failed result is equivalence evidence. The static
scan preserves one unassigned call encoding instead of inferring a new function
boundary. Exact incoming history593bd0ba..4ccfd582 covers163 gated transitions
with no missing/mutated receipts;that accounting does not assert old gate passes
or fresh validation of unrelated behaviors.

## Incoming whole-brain teardown integration

Master4b1aa12c landed before this batch committed. Its normal whole-brain teardown
implementation is independent of the task-credit compiler closure.
[Source review](broader-task-services-results-20261023/whole-brain-merge-source-review-1.json)
checks all53 prospective execution source hashes plus exact behavior/scenario
contracts after the merge;trial7 remains current execution evidence,not a new
sample. The combined Critical action preserves both WBT1 and BTC1 milestones;
all10 policy tests pass in
[UI check2](broader-task-services-results-20261023/ui-check-2.json).
The first UI result remains immutable history. The interrupted pre-staging
completion exported two exact intermediate receipts,which are retained;it did
not create a commit or pass the staged gate. A new completion reviews the combined
tree against4b1aa12c. Incoming4ccfd582..4b1aa12c adds77 exactly reviewed
gated transitions without missing or mutated receipts.
