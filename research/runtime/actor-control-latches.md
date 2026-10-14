# Actor control latch clients

Pinned No-CD build `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Evidence: `draw02-selection-clients-static-01.json`. Confidence: high for exact
instruction/offset observations, provisional for complete selection semantics.

Inventory-bounded `5073d0` checks type `+a8`, health `+e4`, existing field `+68`,
owner `+174` and global `644520`. It skips types `1b` and, on another branch,
`16`. One admitted branch sets actor `+68` to one, increments `6e984c`, and
sets `6903e8` to whether the incremented count is nonzero. An alternate
owner/type branch sets `+70` to one. Calls that dispose auxiliary objects and
update presentation are separate dependencies; full effects are not recovered.

Resetter `5074f0` clears nonzero `+68`, decrements `6e984c`, writes the same
nonzero-count predicate to `6903e8`, and performs conditional auxiliary/UI
cleanup. If `+68` is zero it conditionally clears `+70`. These are the actual
integer writes, including unsigned wrap; no clamped native policy is inferred.

The bounded Region branch `4ff13a..4ff1ca` tests callback mask bit one while
its earlier mode dispatch has selected mode zero. It visits active pool entries
with either field nonzero and calls the resetter, clears `6df17c`, then, when
`656600` is nonnull, calls `570da0` and the setter on that hit candidate.
`4fdad0` has no discovered whole-function entry. This bounded branch remains
separate from a complete callback function or recovered-range execution claim.

Inventory-bounded control dispatcher `4a5070` consumes records from tail
`641a74` until write `641a88`, advances the tail with 32-entry wrap, performs
virtual hit tests and assembles callback masks. Its reviewed tail uses left
transition/value to add bits 1/2, right transition/value for 4/8, 10 for
movement under an existing captured control, and 20 for a changed control.
Other code calls slot zero with 40 on deactivation. Exact record copies are
passed by value. Full hit-test/control-list lifetime, capture and indirect
event routes remain open.

The V2 Region observer records raw validated actor fields `+68/+70` and count
`6e984c` at callback entry and return. These can establish bounded state changes
when correlated with actual callbacks. Numeric pool IDs and slot validation do
not prove allocations/generations; callback observation alone does not close
persistent selection, tooltip, queue lifetime or physical/native presentation.

Retained original02image3afterclick400265andimage4afterpointermove450350
were manually inspected: the green wizard bracket remains visible. Exact
image/report/action hashes are retained in`draw02-selection-bracket-visual-01.json`.
This is bounded visual corroboration, not exact frame/source synchronization,
allselection/tooltip semantics, generation ownership or native pixel parity.


The step-2 selection continuation introduces a separate whole-pool raw latch
recorder (`drawing-actor-latches.md`). Its independent original campaign run
`draw02-actor-latches-original-02.json` and immutable reanalysis
`draw02-actor-latches-analysis-01.json` observe callback 2 setting wizard slot 0
raw +68 from 0 to 1 and count 0 to 1. The first qualifying subsequent sample
103 has the exact completed original cursor at (450,350), no current hit and
slot 0 still latched. 457 qualifying hover-away samples occur before the first
observed pool/flag/identity boundary. All 674 samples join to exact cursor
journal prefixes. This proves bounded sampled raw-latch persistence independent
of hover for this run, with visual corroboration in
`draw02-actor-latches-visual-01.json`. It does not prove uninterrupted state,
actor generations, all selection/tooltip semantics or exact source/frame parity.

The single-owner recorder now claims its first numeric thread atomically and
counts foreign calls atomically. Earlier reports precede that source change and
remain historical. The current contract also removes an unsupported file-backed
data anchor for the runtime-only pool address: the address remains documented,
while the register anchors the reviewed setter/resetter functions. No inventory
boundaries or historical hashes are changed. Fresh final-source and corrected-
contract execution is recorded separately in the continuation checkpoint.
