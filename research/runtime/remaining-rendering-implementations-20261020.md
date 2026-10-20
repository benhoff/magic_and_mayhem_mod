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

## Spectator and single-cell lighting packet

`RE.terrain-spectator-camera-light` implements the complete bounded player=-1
branch. It admits no creatures. Recount resets the cursor, while a mid-cycle
switch without recount preserves quota/cursor/remaining rather than visiting
creatures. Phase/publication still advances when enabled. The camera stamp uses
(column,row,layers/2,index33) after publication and also when creature refresh is
disabled. Static active/requested flags continue to control publication.

`RE.terrain-single-cell-light` implements index1 with its inherited signed kernel
byte. Original initialization sets its pointer/size but never writes the byte;
all 256 inherited values were compared. Native owned storage explicitly defaults
to zero and accepts an explicit inherited byte in TerrainLightingConfig; it does
not read uninitialized host storage. Both maximum and additive stamps work on
single-cell fields. Fixture v2 supplies explicit spectator anchors; v1 continues
to work and now admits static index1. Normal active owner=-1 remains a guarded
refusal because the original non-spectator branch dereferences a null relation
base. This refusal is an intentional native safety policy, not fabricated
neutral-owner semantics.

The pinned-original, native64 and ASan/UBSan outputs agree: 2,304 creature ticks,
all 256 single-cell values/1,536 stamps, 32,768 static admission cases, 4,224 static
stamps and the existing static/combined cycles, plus eight native/sanitized unit
checks. See terrain-light-boundaries-original-20261020.json. Original manifest
bookends pass. Installed entity/static-type input production and full scene/live
acceptance remain separate; the broader lighting group is still partial.

## Explicit external source ownership

Version1 providers implement shared/file/imported/GPU read ownership plus
private binding transfer/restoration (242572f9). Version2 adds direct consumption
for producer-owned nonmappable resources using the user-selected explicit lease/
fence contract. The original binding stays held through exact consumer
completion and verification. Profile2 and pointer-free versioned journals expose
this distinct ownership mode; private-only profile1 remains available. The
producer must enforce its promise across its own aliases/device writers. An
unclassified resource still cannot be admitted. Installed producer adapters and
real driver acceptance remain independent of the generic implementation.

## Camera follow and cell positioning

`RE.terrain-camera-follow-position` implements the whole optional actor-focus
helper (004f7ae0) and cell-to-fine wrapper (004f7a00). A null focus leaves every
field untouched, including with no bound map. Signed actor XY coordinates
normalize into positive fine coordinates using the matching global/receiver map
extents. Z preserves the original unsigned shift/cap and low fractional bits.
The cell wrapper shifts raw 32-bit words with defined wrap before the existing
fine-position setter; negative cells therefore differ from signed actor follow.
Native invalid dimensions are refused before mutation for non-null focus.

`terrain-camera-follow-original-20261020.json` records 4,096 actor follows,
4,096 null-preservation cases and 845 raw cell cases, using unchanged PE32
instructions and guarded private receiver/actor storage, plus native64 and
ASan/UBSan unit checks. The existing reference's renamed unused main now has an
explicit return, preserving its executable behavior and avoiding an included
fixture compiler warning. Original manifest bookends pass. Resource acquisition,
view/rotation and its creature/effect/minimap consumers remain broader work;
this comparison makes no full-scene or live replacement claim.

## Complete bounded palette builders and delivery

The palette group now has all ten numerical modes, owned per-palette append
chains, explicit ordinal/shade selection, profile production and native scene /
terrain application delivery. Component IDs are `RE.palette-effect-modes`,
`RE.palette-effect-chains`, `NR.scene-palette-effects` and
`NR.terrain-palette-effects-preview`. The broad parent stays partial because its
historical milestones do not automatically inherit component execution credit.
No named numerical mode or chain-selection implementation remains missing.

Pinned NoCD functions 00582440,00582a80,00583330,00583bb0,00586660,005870d0,
00585d80,00587e70,00588730,00588bf0 supply modes0..9; 0057d690 is the append
dispatcher. Confidence is high within the finite compared domains. Modes0/8
admit counts2..256; other modes require powers of two. Modes8/9 store DWORD
entries whose values are zero-extended packed RGB555/565 words, and expose only
the dark and neutral wing. Native owned tables retain the whole produced output
and charge allocated capacity. Safe storage replaces original underallocation;
this does not reproduce heap corruption. Shade selection refuses inaccessible
entries rather than reading beyond a table.

Stone and metal reproduce the fixed-seed original noise stream. Metal contrast
is a percentage, with original byte wrapping. Its exact-midpoint branch does
not advance the source RGB pointer; all later loop iterations revisit that
triple and leave the remaining palette tail unchanged. The owned source copy
preserves this behavior and feeds later appended effects. Original shared asset
palettes remain immutable. This unusual pointer branch was confirmed by whole
unchanged original execution, not inferred from a conventional contrast formula.

The chain permits0..4 palettes and unknown-mode/empty-palette successful no-ops.
Each append stages every source/table before publishing an ordinal. Failure is
atomic under native allocation/budget limits; original partial-chain allocation
failure is a separate unsafe baseline branch, not claimed equivalent. The plain
owned profile adapter reads the nine original keys (including `SpecalValue`),
defaults and decimal prefixes. Explicit finite [-1000,1000] scalar bounds are
native policy. Windows profile IO/locale equivalence is not claimed.

The application service binds chains to resident ResourceId/revision identities,
resolves each selected frame's palette, copies table values into SceneDraw and
checks owner-thread lifetime. Reloads require explicit rebind. Terrain preview
supports repeatable `--palette-effect`, `--effect-ordinal`, `--effect-config` and
removes its stale blanket power-only count refusal. Existing ordinary drawing
and lighting algorithms remain suppliers.

Fresh records: palette-effects-original-20261020.json compares3,468 palettes,
63,416,320 words,384 source mutations,24 refusals and506 original underallocated
outputs. palette-chains-original-20261020.json compares40 sessions,352 appends,
128 no-ops,32 refusals and5,263,360 words. Original/native64/ASan-UBSan outputs
agree and native/sanitized units pass, with original manifest bookends. The
private allocator/IAT/SEH adapters are byte/hash checked; original files remain
unchanged. Earlier failed/intermediate reports remain retained without hash
refreshes. scene-palette-native-20261020.json records21 Mesa llvmpipe GPU frames,
revision/budget/thread/retirement controls. terrain-palette-native-20261020.json
records14 GPU frames and15 CLI controls. These headless results do not establish
physical NVIDIA, installed full-game or live replacement acceptance.

Committed-history accounting reviewed269d1c5c..4fb2cb76:16 changed files have
exact parent/current SHA-256 receipts with zero unresolved gaps. The retained
report is working/tests/palette-history-review-20261020.json. This is separate
from current palette execution and makes no claim about a past gate pass.

A final ownership review corrected copy capacity accounting: deep copies reserve
ordinal storage before appending and charge their own nested vector capacities.
A mode8 copy regression checks its shortened allocation, followed by64 staged
copy/append operations and a limit refusal. Fresh capacity-suffixed chain,
scene and terrain records repeat all corresponding comparisons/frames/CLI
controls. Earlier reports and source hashes remain immutable and historical.
