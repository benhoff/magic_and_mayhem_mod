# Remaining rendering implementations — 2026-10-20

The user requested all five broader missing implementation areas from the next
twenty contracts, plus shared/imported/GPU backing support. This work starts
from825a1fee in an isolated worktree so concurrent original-recovery work is
preserved. The existing twenty selected algorithms are suppliers, not twenty
missing implementations. No gameplay balance changes are authorized here.

| Work | Behavior | Implementation branches still to supply |
| --- | --- | --- |
| Palette chains | RE.palette-effect-builders | Modes1..9, non-power-of-two behavior, owned append/ordinal selection and native scene binding. |
| Camera | RE.terrain-camera-lifecycle | Additional clamping/movement and recovered rotation/follow/lifecycle branches. |
| Entity maps | RE.terrain-entity-section-rotation | Entity-linked rotation, runtime entity/reference initialization and ownership. |
| Lighting | RE.terrain-spectator-lighting | Spectator/special-source branches and installed entity/type input production. Existing ordinary creature/static cycles remain suppliers. |
| Scene composition | NR.runtime-scene-composition | Production entity/resource/attachment/light/visibility/depth orchestration. |
| Backing providers | NR.external-backing-ownership | Typed supported shared/file/imported/GPU source identities, read ownership, immutable handoff, completion and restoration. |

Understanding, implementation, original comparison, integration and replacement
will be recorded independently. Every new recovered branch needs original
build/address links and confidence. An original caller's admitted inputs do not
prove its allocation safe. Unclassified storage cannot become safe merely by
removing a refusal; each admitted provider needs an enforceable ownership
contract. Hardware/full-game acceptance does not follow from component tests.

All entries above are initially partial. New results and exact implementation
boundaries will be appended as coherent changes land. Original artifacts remain
immutable; raw disassembly and experiments are retained under task-owned working
storage, with manifest bookends for original-consuming experiments.

## First implementation packet

Mode-zero construction now accepts every admitted count 2..256. Shift is zero
for non-powers of two. For count 3 the original produces the dark and neutral
tables and skips the bright wing; other counts produce two floor((count-1)/2)
wings and a neutral table. Counts 5,7,...255 write 508 bytes beyond the original
allocation request. The native vector owns the full produced output. This is an
intentional safe allocation policy, not reproduction of heap corruption.

Camera clamping (004f79b0) and whole-cell movement (004f7b30) now operate on owned
state, including 32-bit addition wrap, the -5 lower bound and extent+5 upper
bound. Equality at the upper bound clears fractions; equality at -5 preserves
them. Invalid native dimensions are refused before mutation.

Confidence: confirmed for these branches by unchanged-instruction execution in
the pinned NoCD build. The reproducible runner is tools/test-rendering-boundaries.py.
The retained result is rendering-boundaries-original-20261020.json: 510 palettes,
16,776,192 exact words, 252 original underallocated outputs and 3,267 camera cases.
The allocation dependency is byte-checked and redirected only in private mapped
memory to padded fixture storage. Original files pass manifest bookends.
The broader six work groups remain partial; this result makes no mode1..9,
rotation/follow, full scene, external-backing or live replacement claim.
