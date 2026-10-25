# Native alternate canvas ownership

`renderer/alternate_canvas.hpp` supplies an explicit native owner for the
previously borrowed alternate dispatch. The recovered dispatcher remains
unchanged. Its original allocator, publisher, initial pixels, source/base
relationship, complete writers and retirement remain unresolved: the retained
[original session search](alternate-ownership-sessions-20261024.md) observed no
publication through sixteen battles and normal shutdowns. This change supplies
the separately authorized native ownership policy, not an original finding.

## Native policy and lifetime

`AlternateCanvas::publish` validates the padded plane, allocates and zeroes it,
then publishes an opaque generation handle. Invalid dimensions, allocation
failure or an outstanding frame lease leave the previous publication intact.
Both video modes are admitted; each plane is at most 4096 by 4096 RGB565 WORDs
(32 MiB). A new publication invalidates old handles. Handles are weak and scoped
to the actual owner/storage identity, so they neither retain abandoned storage
nor admit a different owner's generation.

`acquire` returns an exclusive movable lease. Reacquisition and publication
during that lease refuse. `retire` immediately removes the current publication,
while an existing lease retains the storage through completion. Republication
waits for that lease to end; the owner never accumulates outstanding retired
generations. Destruction of the owner is also safe while a lease exists. Storage
is destroyed after its last owning reference ends. Existing pixels persist
between frames, including World output preceding a failed ordinary Lock.

The layout deliberately chooses a single padded World plane and a centered
visible copy source at `yPadding * stride + xPadding`, where stride is the video
width plus twice horizontal padding. This relationship and zero initialization
are **native choices**. They do not recover the relationship of original slots
`6e1f68/6e1f6c`. Ordinary word pitch/height must equal the selected visible video
dimensions; an arbitrary external pitch cannot silently redefine this layout.

`compat/legacy/alternate_canvas_dispatch.hpp::dispatchOwnedAlternateCanvas`
acquires the lease before constructing the borrowed request and holds it through
World, Lock, copy, overlays and Unlock. The native renderer does not depend on
the reconstruction or game addresses. The compatibility boundary delegates to
the [unchanged recovered composition](alternate-canvas-composition-20261024.md),
retaining World-before-Lock, failed-Lock pixel effects and ordered copy/overlays.
It removes any temporary alternate binding from outgoing dispatch state before
the lease ends, including on failed Lock or early World exception. That cleanup
is native lifetime policy. Clip/pixel effects keep the recovered ordering.

The caller supplies stable ordinary storage and nonthrowing callbacks as in the
borrowed composition. Vector references obtained through a lease are scoped
borrows: no resizing, swapping or pointer escape is admitted. This service is
single-threaded. It does not install an original alternate route into a game
session, replace the ordinary GPU path, close unknown upstream writers or claim
complete-frame/performance acceptance. Original provenance retains its existing
partial status, independently of the new native headless implementation.

## Verification and reproduction

The owner/adapter fixture covers 24 combinations of both video modes, horizontal
padding 0/4/96, vertical padding 0/3, successful/failed Lock and two overlays.
An independent coordinate formula checks every visible output WORD and every
retained alternate-plane WORD. It also checks callback order, stale/cross-owner
handles, lease moves, reacquisition, invalid and busy publication, repeated
retirement, maximum admitted extents, zero initialization, resize/recreation,
retirement during dispatch and owner destruction while a lease remains.
Early native World-exception cleanup is tested as a lifetime guarantee, not
original exception equivalence or arbitrary post-Lock exception handling.

The runner builds the independent Qt-free CMake target, runs CTest, and compares
release with 32-bit and 64-bit ASan/UBSan results. Exact compiler dependency
closures and prospective behavior/scenario claims are recorded before execution.
No original artifact is consumed. Run through the main checkout's storage guard,
using absolute paths because that guard sets its child working directory:

```sh
python3 tools/working-retention.py run --reserve-gib 0.05 -- \
  python3 /absolute/checkout/tools/test-alternate-canvas-owner.py \
  --output /absolute/checkout/working/tests/alternate-canvas-owner-next \
  --report research/runtime/alternate-canvas-owner-results-20261025/next.json
```

The final [native02 SQLite report](alternate-canvas-owner-results-20261025/native02.json)
supplies `ACO2.native-owner-20261025` for
`NP.alternate-canvas-owned-generation` and its headless scenario. Earlier
[native01](alternate-canvas-owner-results-20261025/native01.json) passed before
the maximum-extent and complete alternate-plane checks were added; its hashes
remain historical. Initial exploratory compilation stopped before execution
because the RAM-backed temporary filesystem exhausted its quota; the final
runner explicitly uses bounded owned disk-backed compiler scratch. Source,
logs, binaries and both reports remain retained locally. These are native
execution controls, separate from original comparison and live replacement.
