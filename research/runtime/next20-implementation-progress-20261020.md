# Next twenty partial implementation contracts — 2026-10-20

The [frozen selection](coverage/next20-selection-20261020.json) chooses the next
twenty raw partial implementations by importance and behavior ID after excluding
the first batch. It contains fifteen Critical rendering and five High animation
contracts. Partial is a coverage boundary, not proof that every selected item
has a missing code branch. Existing implementations and concurrent work are
reviewed before changes; gameplay behavior and balance remain separate.

## Group 1: World queue routing and kind-8 admission

Adopted and reviewed the pending owned row dispatcher and six kind-8 class/method
models. Fixed the actual observed inherited-additive class admission:
`5c73f0/54aaa0`, rather than the impossible `5c73f0/546540` pair. The corrected
class uses shape offset `+0x34`, without the owner gate or drawn-flag write of
other wrappers. The original process still executes its object methods.

Extended the independent original/native row matrix from four to nine backend
return words, including nonzero upper halves with low AX equal to zero, one and
`ffff`. The reference fixture's expected normalization now explicitly examines
AX. Every one of **5,472 cases / 6,084 backend calls** matches the unchanged
consumer. All **960 kind-8 wrapper cases**, the actual PE32 admission/ABI fixture,
and both native CTests pass. The Wine fixture clears inherited Wine overrides.
Immutable original manifests verify before and after; original files were not
modified. Confidence is high within these compared matrices.

This advances `RS.world-dispatch-targets` and `RS.world-kind8-virtual` through
separately scoped supplier contracts `RS.world-dispatch-routing` and
`RS.world-kind8-methods`. The two aggregate implementations remain partial:
consumer clip/prelude/postlude state, helper receivers, downstream raster pixels,
unknown classes and object construction/writers/lifetime remain open. No live
replacement or full rendering acceptance is claimed. The other eighteen selected
contracts remain queued for this task.

[Fresh immutable execution](next20-world-dispatch-20261020.json) is independent
of the retained earlier WORLD results. Their original fingerprints are unchanged
and may be stale after the extended fixture/runner edits. [Incoming committed
history review](coverage/next20-incoming-history-20261020.json) accounts for the
range since the previous batch separately from this new execution.

## Group 2: publication boundary and source permission proofs

Reproduced a final-publication chronology defect: a call labelled with the last
queue could still precede that queue's recorded World return. Admission now
requires its emitted producer sequence to include the final return. All 23
publication/action controls pass; the new negative failed before the fix.

Reproduced unknown protection combinations entering the guarded region cache.
Only the six readable base protection modes can now become reusable permission
proofs. Guarded, execute-only, uncommitted, overflowing and unknown answers are
queried freshly. Three malformed/unreadable modes and all six readable modes
pass alongside the existing 10,000-query, owner-thread, epoch reset and copied
content controls, including ASan/UBSan. This is native permission policy; it does
not recover external permission changes within an admitted epoch.

The new [native execution](next20-canvas-native-20261020.json) also passes actual
PE32 startup forwarding and 57 compatible producer entry sites, 13 complete
raster bypass entries and malformed caller/reply/guard refusal. Native GPU
handoff retains intervening HUD content. The [original branch execution](next20-canvas-original-20261020.json)
passes 40 create, 48 padded binding, 25 full-fill, 33 initializer, 5,355 palette,
180 uniform-fill and 36 caller cases. Scripted external COM/driver boundaries
remain explicit. The independent [wave result](next20-wave-original-cpu-gpu-20261020.json)
compares all 768 full canvases / 1,572,864 words per backend byte for byte with
unchanged original output and a separately generated native initializer table.
No original destination pixels seed native replay.

These controls exercise the eleven selected contracts other than pre-World
reuse and initial-state attribution. For `RS.canvas-preworld-reuse` and
`RS.world-initial-state`, reviewed retained evidence reconstructs all 480,000
first-World words with matching original/native hashes across 1,061 producer
checkpoints. That existing observation remains historical at its own source
fingerprints. This change does not identify all external writers, physical
allocation/COM alias ownership or renew whole-startup original equivalence.
Those are remaining recovery boundaries, not justification to seed native
storage from an original canvas. No raw milestone is promoted in this group.

All fifteen selected Critical contracts have now received a bounded code and
validation review. The five selected animation contracts remain queued.

## Group 3: owned body/attachment operations and animation admission

Implemented the previously documented body null-display branch as `bodyPlacement`:
start sequence288+facing only when no frame is displayed, preserve an existing
controller's phase, apply the inclusive supplied SPR header bound with frame-zero
fallback, and retain recovered footprint/view offset arithmetic. The service
returns owned value descriptors. Source failure or an absent/negative fallback
frame refuses before replacing the previous accepted player.

Implemented `ModeOneAttachment` as an owned native component. It binds configured
asset/base/facing values through a source service, handles same-mode no-op before
source access, performs selected0-to-1 and1-to-0 transitions, retains its binding
on removal, and resolves/restarts on explicit reentry. Health gates visibility
but not the selected child tick; stopped children do not automatically restart.
Other modes and complete effect/entity ownership remain outside this component.

Fixed negative selected SPR records being published by forward start/tick through
`displayedRecord`, before the existing `sprite()` validation. Rejection now occurs
at selection. This is the separately registered native admission policy; it does
not claim original equivalence for negative SPR arguments or source failures.
The regression failed against the earlier native player.

The [native result](next20-animation-native-20261020.json) passes four CTests,
four ASan/UBSan controls and168production-facing cases. The independent
[original result](next20-animation-original-20261020.json) matches:

- 4,608 body descriptor cases:2,304null-display starts and2,304existing displays;
- 347forward traces /22,555states across136ANI assets (including loader-only
  checks for three older-version files);
- 2,140phase-switch traces /70,620states across118installed assets;
- 6,097,440placement helper comparisons;
- 520attachment controller states and same-mode no-ops, plus eight
  removal/reentry pairs using the new owned component directly.

The separate [full controller result](next20-animation-controller-20261020.json)
checks the existing forward/reverse/rewind/phase supplier against unchanged
original bodies and sanitized native execution. Fixture helpers now compile with
-O2 and read the pinned immutable image directly. A missing isolated legacy input
and an interrupted slow unoptimized run are retained as setup/performance
attempts, not engine failures. Original manifests verify before and after.

All twenty selected contracts have now been reviewed. This task delivered four
concrete defect fixes and supplied owned queue/class routing, body fallback and
mode-one lifecycle operations. Broad raw partial milestones remain independent:
physical ownership/all-writer recovery, complete startup/consumer state,
unknown object classes, full effect/targeting/lifetime composition and live
replacement are still not established by these bounded comparisons.

The canvas execution records captured a broad source snapshot. Subsequent
animation edits make those whole-record freshness flags historical, although the
specific publication/cache and wave implementation bytes still match their tested
versions. Their original fingerprints are preserved; no evidence is refreshed to
hide this accounting boundary.
