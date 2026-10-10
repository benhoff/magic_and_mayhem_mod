# Owned minimap extension to the producer journal

The opt-in `MNM_MINIMAP_OWNED=1` route uses magic `MNMPRO02`, version 2.
The 64-byte envelope, 96-byte records and operation numbers 1..23
retain their V1 definitions. V1 decoders must refuse V2 rather than interpreting
new minimap fields. Native/Python decoders accept both versions. Runtime capture
forwards every original function and owns copies of raw source inputs; no game
pointers enter the minimap payload.

Terrain operation 22 sets record word 16 to orientation 0..3 and word 19 to the
observed object `+0xff` invalidation result `0xfffffffe`. Source words and closed
hidden decisions retain the three-byte cell format; the auxiliary surface ID
identifies native prior producer composition, never an original image oracle.

Operation 24 has 37 little-endian uint32 metadata words (148 bytes), followed
by zero to 1024 twelve-byte creature entries `(x,y,hidden)`:

| Word | Source meaning |
| --- | --- |
| 0 | Kind: cell 0, creature 1, camera corners 2 |
| 1..2 | Global grid width/height |
| 3..4 | Source center X/Y |
| 5..6 | Independent rotation width/height |
| 7..8 | Incoming origin X/Y |
| 9..12 | Orientation, RGB555, fog, flash |
| 13..16 | Cell X/Y, palette index, emphasis |
| 17 | Creature entry count; zero for other kinds |
| 18..19 | Camera viewport width/height |
| 20..27 | Four source corner direction X/Y pairs |
| 28..36 | Nine raw palette words, stored as uint32 |

Record word 21 for cell markers is the original EAX destination pointer normalized
to a destination word offset. Words 19/20 for creature markers are observed grid
refresh results; for camera corners they are the selected object origin values
observed after nested terrain returns. Other original scratch state is outside
this pixel/result contract. Creature row-pointer entries must match the admitted
positive-stride origin; unsupported aliases emit failure 36. Camera outer borders
currently emit failure 37. Nonempty subsequent cell marker lists are independently
captured as operation 24 cell calls. Terrain is emitted and checkpointed before
camera corners; corners are emitted before subsequent cell/creature calls.

The native canvas adapter uses two independently initialized owned planes to
identify all overwritten words, including words equal to a sentinel. It commits
only identified logical pixels after complete native preflight and result checks.
Padding and undefined untouched pixels remain untouched. History reads require
native prior auxiliary pixels to have been defined by captured source producers.
Original completed canvases are retained in separate checkpoint files solely for
comparison. The observer performs no native writeback or original drawing bypass.

V2 permits a bounded262144-record/512MiB source journal; V1 retains its65536-record/128MiB writer bounds. Decoders also accept retained32768/65536/131072-record envelopes. Moving the camera can exceed the previous record bound even within16 World calls. Original completion storage remains separately budgeted and never enters native replay.

For operation24 camera inputs, record word17 names the selected outline entry independently of metadata word9 (stored terrain/marker orientation):0=5527a0,1=552b50,2=552f20,3=5532f0. It is zero for cell/creature inputs. The original caller uses its own dispatch state; ordinal entry order must not be inferred from stored minimap orientation. The fixture deliberately tests distinct values.
