# Clipped direct-word sprite grayscale

Original No-CD build40209ca7, routine0x589c90, is observed as producer
backend11. Its full-horizontal-width and vertical-only clipping paths convert
drawn16-bit source words to gray. Its horizontal clipping paths copy colored
source words at0x589f54..0x589fca. Transparency and row padding are retained.
Static disassembly0x589c93..0x589cb4 chooses channel shifts11/6 for the
zero-global format branch;0x589e80..0x589ee4 masks channels, shifts red and
green to five bits, divides their sum with blue by3, and packs the mean at
11/6/0. The resulting green value has its low bit zero. This is a recovered
RGB565 branch; RGB555/global-format variants and indexed inputs remain open.

Fresh original120 source record228745 on canvas22 is backend11 at0,96,
clipped to0,96..37,137. Its next Lock checkpoint1541 (sequence229375) differs
from unchanged native replay by950pixels within0,96..35,134. For all950
differing pixels, the original equals the recovered grayscale conversion of
the native colored value. The next backend8draw restores colored source words
and checkpoint1542matches again. The original and failed native plane hashes
repeat the earlier120canvas22failure exactly. These observations narrow a
missing conversion; they do not establish general ordering or byte ownership.
Original disassembly and immutable-input verification logs are retained under
working/draw02-raster11-disassembly-01.txt and the adjacent before/after logs.

Native acceptance is restricted to backend11/mode0/direct16-bit frame words
with the existing RGB565 canvas contract. Unsupported indexed/mode variants
must refuse before canvas writes. Synthetic full-word coverage and complete
retained-checkpoint replay will validate the implementation independently;
original planes remain external comparison oracles.

Validation: `draw02-grayscale-unit-01.json` passes all65536word inputs,
clipping/transparency and unsupported-input refusal.
`draw02-grayscale-current120-01.json` matches all2278checkpoints;
`draw02-grayscale-prior120-01.json` matches2181of2182, repairing canvas22
but retaining the separate1600pixelcanvas2failure. This is scoped native
execution against independently retained original output, not isolated original
full-word validation, complete physical presentation or live replacement.

## Isolated original horizontal-clip branch

The first sandboxed32-bit process ends withSIGSYS without results; its old
original_body_executed field meant process launch and cannot prove entry.
The runner now records successful body execution only after completed checks.
An escalated run passes the full-word and HUD cases, then fails a combined
left/right/top clipping expectation. Debug retains376different words and raw
original/expected planes. Original disassembly0x589f54..0x589fca confirms
these horizontal loops copy words directly, bypassing the gray arithmetic.

The corrected independent scalar fixture passes eight original cases and
530432destination/padding words with zero differences: all65536source colors,
HUD placement, negative and shifted origins, transparent runs, combined and
pureleft/right clipping, vertical-only clipping and no overlap. Its PE image
bytes remain unmodified; fixture globals live only in private mapped memory.
This validates the selected zero-format RGB565 branch against authored scalar
expectations. Original callerABI, RGB555 and indexed variants remain open.
Native backend11 now applies gray only when the horizontal frame extent fits
the clip rectangle, reproducing the original colored clipping branch.
