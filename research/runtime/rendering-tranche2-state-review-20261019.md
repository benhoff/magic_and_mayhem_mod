# Tranche 2 implementation state and next work — 2026-10-19

Tranche 2 now uses the same four labels as tranches 1 and 3. The source assessment
is **eight implemented within scope, three unresolved aggregates, and zero
concrete partial-code gaps identified**. The retained original-initial-state
mismatch also carries **acceptance failure**. That outcome overlaps an unresolved
aggregate; it does not identify a missing raster implementation.

Implemented within scope means the code covers the declared bounded contract;
validation can remain historical, pending or failed. Partially implemented
requires a named missing required branch, mechanism or supported domain.
Unresolved aggregate requires reconciling delivered services and narrower
contracts before sizing missing code. Acceptance failure records an unmet outcome
separately and requires diagnosis before attributing a defect.

This is a source, scope and UI review. It executes no original fixture, capture,
native rendering experiment or game pilot, and promotes no engine stage. Supplied
new fixtures are source-reviewed controls, not execution evidence. Historical
records and source hashes are retained. Confidence is high for the named code
mechanisms and fixture boundaries; complete causal history, physical ownership
and current-source equivalence remain unresolved.

| Contract | Current state | Next work |
| --- | --- | --- |
| `RS.canvas-create-lock` | Implemented selected wrapper/Lock observer; original creation/fatal-child and separate Lock/Restore/retry fixtures are supplied. This is not a native allocator. | Execute supplied matrices with claims and valid observation leases; distinguish logical epochs from physical allocations/COM aliases. |
| `RS.canvas-bind-clip` | Implemented observation and independent padded stride/height calculation. Original bind/consume/rebind window fixture covers ordinary/padded/null-Lock cases. | Validate the supplied window matrix; retain whole-dispatcher and live alternate-buffer reachability separately. |
| `RS.canvas-startup-clear` | Implemented request observation; original full-fill result/retry fixture supplies scripted completion and padding guards. | Validate that fixture; actual first-startup completion remains unsampled and needs a safe lease or an explicit unavailable result. |
| `RS.canvas-preworld-reuse` | Unresolved aggregate over delivered startup observations, logical generations and DRAW-02 Lock-epoch services. A same-run history checker is supplied. | Crosswalk a closed causal prefix and name unknown writers/aliases; pointer equality does not prove physical ownership. |
| `RS.canvas-producer-sequence` | Unresolved aggregate over delivered recorder/source/native services, including selected uniform `547d80` requests and original clipping/packing fixture. | Validate supplied controls, establish real reachability and reconcile immutable source history. The physical 4x75 residual attribution remains a hypothesis. |
| `RS.world-startup-palette` | Implemented actual palette capture, explicit black and shade -127 fallback distinction. A slow-path original selector matrix is supplied. | Validate table words/pixels and observer refusal independently; selector coverage does not execute the full dispatcher or every primitive. |
| `RS.world-startup-wave` | Implemented post-original lazy-table capture and independent bounded native initializer/offset supplier. Cold/warm/default/phase/clipping references are supplied. | Compare the full table and effective outputs; keep initializer 0..15, effective admission 1..8, ABI, live and FPU claims separate. |
| `RS.world-initial-state` | Unresolved aggregate; retained zero/live differences are an acceptance failure. A source-only same-run initial-canvas comparison checker exists. | Reconcile all defined regions/producers/epochs, execute the checker on valid inputs and diagnose retained differences or separately prove a reset. |
| `NR.canvas-producer-replay` | Implemented admitted source-only CPU composition, definedness/generation refusal, GPU mirror and viewport delivery. Historical comparison matches 1,062 completions. | Reuse supplied order/missing-asset/undefined/retired-generation controls and renew an exact nonempty external comparison; upstream history and GPU World handoff stay separate. |
| `NR.world-startup-sequence` | Implemented finite word-zero World-history policy and comparison workflow; historical 16-queue output comparison matches 7,680,000 pixels. | Enforce all three match booleans, declared contiguous count and workspace provenance; initial/HUD differences still do not reconstruct initialization. |
| `RS.world-producer-bypass` | Implemented selected generic signature/tag guard and recovered zero-EAX fully-inside return domain; clipped/empty requests forward original work. | Validate supplied production admission/body/continuation/refusal controls; retain unsampled scratch effects/callers and separate GPU handoff/live gates. |

## Corrections to the older plan

The [earlier planning review](rendering-tranche2-planning-review-20261019.md)
remains historical. Its requests to author original creation, padded-binding,
full-fill, selector, wave and generic caller fixtures now point to supplied
controls in `tests/rendering-tranche2-reference.cpp` and its runner. The original
binding fixture executes `4fd438..4fd5ad`, with scripted Lock/consumer boundaries;
it does not execute the entire `4fd280` dispatcher. Creation fixtures substitute
COM results and stop fatal children at a UI boundary. Clear completion is
scripted inside caller-owned storage, not real driver completion. Original
reference execution remains pending in this UI review.

`reconstruction/rendering/startup_canvas.hpp` now supplies padded dimensions and
the independent 16x16 wave initializer/offsets. The original approximate double
angle, truncation and bias are explicit. The older `i*13` ABI fixture still tests
post-return copying and state preservation; it is not initializer validation.
The palette fixture exercises slow-path table selection, not all dispatcher
routes or observer invalid-table refusal.

The generic finite bypass now invokes its own signature/tag guard before
request/writeback, separately from `raster_active`. Its selected caller
continuations consume AX; the admitted body domain uses zero EAX rather than
preserving incoming EAX. Production admission and original body return behavior
need fresh execution. Synthetic selftests can bypass caller admission and cannot
supply that proof. `5947b2` remains historically live-unobserved, and unsampled
scratch/state effects remain outside the selected contract.

`tools/test-rendering-tranche2-history.py` supplies initial-canvas comparison
against a matching closed replay and same-run storage journal. It still needs a
valid dataset and reviewed interval. It explicitly does not prove physical
allocation ownership or all writers. Existing startup/World and producer CLI
pipelines remain incompatible; the checker does not make separate captures one
lifetime or provide a compatible capture extension.

Historical initialization diagnostics retain a 7-pixel zero/live gap and four
frames with 15 unchanged retained differences each. Their successful diagnostic
reports are failed pixel-equality outcomes for those scenarios. The later
16-queue startup result matches every completed output while initial and
between-queue states still differ. These are different claims and scenarios;
neither the completed-output success nor the separate manual-history negative
changes the other result. The startup runner's `success=true` means measurement
completed; acceptance must also check all three match fields, exact count and
`current_workspace_sources_match`.

## Evidence and execution boundaries

The catalog records exact code witnesses, direct implementation/test/document
hashes, registered historical evidence and typed validation/scope/diagnosis steps
for all eleven tasks. Source drift flags an assessment for review, without
refreshing old execution hashes. This review changes no recovered contract or
engine implementation/comparison/integration/replacement status. The four labels
now appear for all three tranches in cards, importance filters, behavior tables
and dialogs. Raw historical stages still say Registered partial.

Tranche-1 default output acceptance remains failed after the viewport repair,
with CPU-history transfer refusal stage 13 and a separate timing-altered traced
pass. Dependent integration/replacement stays held on its gate and a separately
budgeted run. This source review authorizes no additional pilot.

The reviewed HEAD remains `0dc91365aed634b16c61f31cc8f34b2746eee62a`, covered by
the existing committed-history review. Concurrent engine/campaign changes are
preserved. UI checks, census, audit and exact review artifacts are retained under
`working/tests/rendering-tranche2-state-review-20261019/`; they are not engine
equivalence evidence. Whole-tree accounting limitations are recorded separately.

Verification: 22 planning/schema tests, 21 UI/HTTP tests, JavaScript syntax and
19 focused Chromium checks pass. The browser checks use production frontend
functions and real planning enrichment, including all eleven tranche-2 states,
typed steps, supplied-control text, labels, four filters and mobile width. Desktop
and mobile screenshots are retained. This focused check does not assert a full
dashboard backend reconciliation pass or execute any engine fixture.

During review, concurrent work linked five new original/native/GPU wave fixture
inputs to `NR.world-producer-handoff`. Their encoded sprite, case enumeration,
hash and renderer-readback roles were inspected. The existing tranche-3 source
assessment gains those additive fingerprints with an incremental-review note;
all previous fingerprints, classifications and historical engine evidence remain
unchanged. The additions record supplied controls, not their execution or renewed
handoff validation. Ongoing engine-source edits can flag these source assessments
historical immediately; this review does not refresh them to hide that drift.
The later linked `tools/test-rendering-tranche2-wave.py` is also recorded as a
supplied control: it pins original/reference/native inputs, requires the full
768-case matrix and compares complete CPU/GPU words and generated tables. Its
additive review grants no execution or status promotion.

The coverage audit completes with no errors; retained historical staleness and
unknown coverage remain visible. Whole-tree gate draft generation encountered
concurrent source-census drift in `tests/rendering-tranche2-native.cpp` and the
new `tools/test-rendering-tranche2-wave.py`; no gate pass is claimed. The full
dashboard reconciliation loader also retains its existing incomplete-path error
for `RS.world-initial-state` after concurrent dependency additions. Focused UI
checks do not clear that backend refresh limitation. These limitations are
independent of the eleven reviewed work labels.
