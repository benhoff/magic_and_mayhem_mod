# Clipped direct-word sprite grayscale

Original No-CD build40209ca7, routine0x589c90, is observed as producer
backend11. It clips RLE rows and converts drawn16-bit source words to gray.
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
