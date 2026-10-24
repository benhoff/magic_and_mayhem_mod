# New-sighting task/service composition (2026-10-23)

Pinned NoCD SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Recovered contract

At `418860..41896b`, only a newly inserted observation credits a task. The
observed actor's `+48+4*owner` names its observer. The observer's
`+678+4*brainFamily` selects a source. Actual `407920/4079f0` traversal runs
at global mode3, checks whether a first actor exists, drains all remaining
actors and restores the prior global mode. It does not test those borrowed
actors' health/activity. Source signed activity is the eligibility threshold.

`522770` separately follows that selected source's `+39` parents to the
terminal node and reads its task backlink `+5`. The recipient may therefore
belong to an ancestor outside the traversed subtree. Missing observer, source,
eligible actor or terminal task skips credit. The observed actor's CFG kind
selects quantity cap1, or10 for wizard kinds0/24/25/26. `416d80` receives type
in ECX, **health in EDX as level**, min(unsigned health,cap) and three zero
float arguments on the stack. Real `505bf0/505c50` profile accessors and
`59bee0` numeric conversion supply the result, added modulo2^32 to task`+2f`.
Refresh and expiry do not credit; a new observation after expiry does.

## Native composition and boundaries

`owner_observation_tasks.hpp` composes existing logical knowledge, owned
snapshot storage, cache invalidation, source traversal and contribution
valuation with concrete parent-chain task resolution. Checked byte storage
and read-only profiles are non-reentrant. In the admitted one-owner/one-actor
increasing-tick domain, the logical update and owned snapshot commit precede
new-insertion consequences. Source/task/observer pointers are borrowed;
retirement remains with the existing task source owner.

The native ordered snapshot index is a representation policy, as are bounded
acyclic ancestry, depth and positive signed-health admission. The existing
logical task reward API's overflow refusal is separate: this original path
uses unsigned wrap. Forests and preinstalled task backlinks are independently
authored fixture inputs, not original output or task-recipient stubs. General
task installation/scheduler execution, multiple owners/records, whole-brain
World activation/visibility providers, reentry, SEH/allocation failure and live
replacement remain outside this batch.

## Execution evidence

Original32, native64 and ASan/UBSan/LSan agree across384 independently authored
episodes/3456 updates. The immutable
[comparison](observation-task-services-results-20261023/observation-task-trial-2.json)
binds every declared/compiler input,checks source and original text stability
and verifies original manifests before/after. Both families and all eight
authored profile kinds,health values1/2/7/9/10/11/127/1000,missing services,
full-drain free masks and task totals are compared. No original output is an
input to native execution. Exact totals are recorded in the report.

The first trial exposed a fixture-disposal boundary: recovered source
destruction skips negative-activity children,leaving their borrowed parent
unchanged. Before normal forward task retirement the successful fixture
explicitly restores leaf activity3. This does not repair the baseline or
validate arbitrary negative-activity forest destruction. All normal allocations
are then released,with suffix guards and no stale borrow dereference. The
failed result and source bindings are retained. Ten UI policy checks pass after
updating a stale Critical partial-count expectation left by the previous
rendering integration;its first failed report is also preserved.
