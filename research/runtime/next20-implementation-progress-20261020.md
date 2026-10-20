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
