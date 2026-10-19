# Rendering critical tranche 2 planning review — 2026-10-19

All eleven tasks in **Original canvas history and native producers** were checked
against their central behavior contracts, linked research, current source and
fixture/runner entrypoints. The revised plan is in
[`apps/coverage-ui/importance.json`](../../apps/coverage-ui/importance.json),
including per-task inputs, dependencies, command templates, existing coverage,
missing fixtures, acceptance and handoffs. This is a static planning review:
no original artifact, engine control or game pilot was executed, and no engine
status or historical evidence fingerprint was changed.

## Prerequisite and execution boundaries

Tranche 1 records ten bounded PASS results and **output-ownership acceptance
FAIL**, with both pilot attempts retained. Source inspection, fixture preparation
and independent component work can proceed. Dependent integration/replacement
execution remains held until that acceptance gate is repaired and passed under a
separate explicit run budget. This review does not grant another pilot attempt.
See [the execution record](rendering-tranche1-execution-20261019.json) and
[ledger](coverage-ledger.md).

There are two existing finite capture pipelines:

- Startup/World: `--canvas-startup --world-frames --world-lifetime
  --startup-replay --startup-queues 16 --samples 16`.
- Original-active producers: `--canvas-producers --samples 16`, optionally with
  its separately validated producer viewer/handoff configuration.

`tools/capture-scene-game.py` rejects producer capture combined with startup or
World-frame hooks. A same-run causal timeline needs a reviewed compatible
observer extension; separate captures cannot be joined through pointer equality.
Existing DRAW-02 query-to-Lock-epoch observations should be extended where useful,
rather than repeating a pointer-only startup trace. Logical wrapper generations,
observed Lock epochs and physical allocation/COM alias ownership remain distinct.
Unchanged/readable storage may already be unlocked.

Command templates identify existing entrypoints, not complete automated task
solutions. Preflight their inputs and tools, implement the declared missing
fixtures, choose new report/evidence identities, declare claims prospectively,
and use the storage-retention wrapper with an estimated output reservation.
Original-consuming work requires before/after original-manifest verification.
Unsupported or missing required cases are BLOCKED; executed mismatches are FAIL.
A report or precise diagnosis alone does not pass the tranche.

## Review of each task

| Order / behavior | Accuracy correction and next executable work |
| --- | --- |
| 1. `RS.canvas-create-lock` | Existing tests exercise synthetic compatible hook bodies. Add pinned original-wrapper COM-result fixtures for null, Lock success, lost surface, Restore and retry failures/success, including fatal-child termination. `0x6f8150` stores initial Lock/Restore, not retry Lock; inspect retry directly. Conditional Unlock and returned-pointer lifetime need separate expectations. Extend the existing transition journal afterward; observer coverage is not a native allocator. |
| 2. `RS.canvas-bind-clip` | Keep word stride, bind height and clip bottom independent. Add original `0x4fd280` padded-dispatch/ordinary-rebind fixtures; scalar setter forwarding does not execute that branch. The retained map-2 capture leaves `0x6e1f68` zero, so live alternate-route reachability still needs its own scenario. |
| 3. `RS.canvas-startup-clear` | Startup requests a full zero fill at `0x4e41f8`, but its initial completion is unsampled. Add original `0x58bc10` driver-result/completion/error fixtures with valid observation leases. Wrapper EAX and last-known readable Lock pixels do not prove driver completion. Keep any native zero-reset policy separate. |
| 4. `RS.canvas-preworld-reuse` | Build on DRAW-02 V3 query/epoch binding, retaining create/release invalidation and reused-address uncertainty. Final `0x58bf40` copies drawing source ECX to the presentation destination, rather than importing presentation pixels. Bind selected producer prefixes to storage transitions; retain unknown allocations, aliases and writer intervals. |
| 5. `RS.canvas-producer-sequence` | Recover the selected producer before composing its initial history. The uniform mode-0/NULL-plane `0x547d80` branch has synthetic source/clipping controls; its attribution to the two 4x75 physical residuals is still a hypothesis. Establish original reachability, original-body semantics and full native completion comparison. Recorder serialization alone proves neither immutable source bytes nor physical-frame causality. |
| 6. `RS.world-startup-palette` | Preserve explicit-black versus real-palette fallback at shade -127. Existing synthetic controls use a constant `0x1234` palette for tags 1..6 and zeros for 0/10; they do not validate all original table selectors. Select one uncovered shade/shift/neutral/ordinal branch and compare all 256 words and selected pixels, with observer invalid-table refusal tested separately. |
| 7. `RS.world-startup-wave` | The implementation observes effective offsets after original lazy initialization; it does not independently generate the native table. The ABI fixture uses synthetic `i*13` entries, amplitude 3 and phase 5. Add original cold/warm initializer and selected explicit/default amplitude/clipping/phase fixtures before claiming generator equivalence. Original initializer amplitudes 0..15 differ from current nonclipped observer admission 1..8; kinds 16/17/20 select 2/3/6 only in finite startup mode. |
| 8. `RS.world-initial-state` | Audit selected original before-state and its causal owned producer history after lifecycle/producer recovery. Add a complete initial-canvas reconstruction comparison or independently prove a reset boundary. `test-world-background.py` intentionally succeeds when the retained live mismatch reproduces. A matching completed World output can hide overwritten initial/HUD differences and cannot establish initialization. |
| 9. `NR.canvas-producer-replay` | Replay source-only producer requests/assets from an original-active complete capture; native storage begins undefined. Require an exact nonempty completion set, independent external comparisons and explicit order/missing-source/retired-generation/undefined-history negatives. The CPU composer plus GPU mirror is distinct from the GPU World handoff. Original copy-entry COM success adapters do not prove pixel-copy/driver equivalence. |
| 10. `NR.world-startup-sequence` | Restore the actual runner to the check list. This is a separate word-zero World-history policy, not assembly of create/menu/loading/HUD producers from task 9. Require all three completion match booleans, the declared contiguous queue count and current-workspace source match. Record initial and between-queue differences separately; `success=true` means the comparison ran, even with mismatches. |
| 11. `RS.world-producer-bypass` | Restore the original-reference and handoff runner links. Recover the chosen caller's consumed return/register/flag effects; preserving incoming state is not a comparison with outgoing original-body semantics. The generic finite bypass does not invoke the later `raster_active` caller whitelist, and SELFTEST stubs that whitelist. Add generic caller admission/refusal fixtures before promising unknown-caller rejection. Offline handoff comparison does not execute a new live bypass; `0x5947b2` remains historically live-unobserved. |

The order now places selected producer recovery before first-World history.
Palette/wave fixtures and the word-zero comparison have independent prerequisites;
they need not wait for original initialization recovery. Producer-seeded replay
and caller bypass cannot use a failed original-history prerequisite. A positive
GPU handoff under `NR.world-producer-handoff` remains an external prerequisite to
the selected bypass; CPU replay cannot supply it implicitly.

## Source basis and confidence

The review inspected `runtime/scene/canvas_startup.c` and its assembly/fixtures,
`runtime/scene/world_trace.c`, `runtime/scene/world_producer_bypass.c`, the generic
selection in `runtime/scene/canvas_producers.c`, producer/recorder fixtures,
`tests/canvas-sequence-test.cpp`, original World references and the corresponding
Python runners. Existing research supplies the pinned original addresses and
historical outcomes; this review did not independently re-export the binary.

- [Canvas startup and ownership limits](native-world-canvas-startup.md),
  [diagnostic format](../formats/canvas-startup-diagnostics.md), and
  [DRAW-02 storage generations](draw02-canvas-storage-generations.md).
- [Producer semantics and historical comparisons](native-canvas-producers.md),
  [producer recorder](draw02-producer-recorder.md), and
  [producer protocol](../formats/canvas-producers-v1.md).
- [Startup palette/wave and comparison policy](native-startup-sequence.md),
  [initial-canvas diagnostic](native-world-live-background-gap.md), and
  [finite handoff/bypass](native-world-producer-handoff.md).

Confidence is high for current CLI constraints, fixture coverage and the source
branches described here. Physical storage ownership, unassigned writers, new
original branch reachability, outgoing caller semantics and broader equivalence
remain pending execution/recovery. No diagnosis is promoted to a confirmed cause.

Planning checks validate the catalog without mutating engine contracts, inspect
templates against source-declared required/optional CLI flags and verify task
dependency ordering. Reports are retained under
`working/tests/rendering-tranche2-review-20261019/`; they are tooling/planning
checks, not engine comparison evidence. The pre-existing local-baseline gate
fails on unrelated accumulated accounting/history/validation debt; the incremental
receipt and audit preserve that distinction rather than resetting its baseline.
The expanded planning test is among the broadly fingerprinted sources of all
eleven tranche-1 reports. Those unchanged historical reports now have a
planning-test source mismatch; their current-code validation is not renewed.
The new receipt retains these explicit stale-evidence/validation gaps. No engine
rerun was performed merely to refresh a changed planning-test fingerprint.
HEAD remains `0dc91365aed634b16c61f31cc8f34b2746eee62a`, already covered by
`coverage/engine-simulation-ui-head-history-20261019.json`; no later committed
range needs retrospective receipts in this review.

After the successful audit/index review, a concurrent change to
`apps/qt-shell/campaign_smoke_test.cpp` replaced obsolete `QOpenGLWidget` lookups
with `GlViewport` lookups. It is preserved outside this change's receipt. Its
new source hash makes the census stale and blocks the final whole-tree gate
snapshot; the retained accounting log records that limitation. The source fix
does not itself replace the historical failed pilot with fresh acceptance.
