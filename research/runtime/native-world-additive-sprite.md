# Indexed additive sprite raster

The pinned No-CD `0x580a90..0x5811ff` leaf adds palette colors to destination
pixels with saturation in each channel. Kind10 calls it directly; kind9's
`0x4df840` helper stamps it along a line. The earlier native dispatch model's
`distortion` enum name for this particular leaf was misleading. The actual
distortion leaf `0x596490` is a separate operation.

Static evidence is the unchanged pinned executable and retained disassembly in
`working/tests/world-leaf-inspection-20261019/0x580a90.asm`. The entry takes frame
in ECX, X in EDX, then Y, shade and palette ordinal; it returns with `ret12`.
The chain at frame+28 selects the ordinal node. Its shift and neutral entries
choose a256-WORD table using the original negative-shade rounding. Frame+12/+16
origins are subtracted before clipping. The original loop consumes row skip/run
bytes and opaque palette indices; index zero remains opaque.

For the selected RGB565 masks `f800/07e0/001f`, the native operation clamps sums
to31/63/31 and repacks. This differs from the additive rectangle's low16 channel
increment wrap. Each native draw freezes the preceding destination on the GPU,
preserving ordered overlap without normal drawing readback. Both indexed atlas
and ordinary sprite resources use the existing source mask and effective palette.

World v1 operation8/backend13 carries an owned indexed frame and512-byte selected
palette. Its scalar diagnostics do not become host pointers or palette-generating
inputs. Old decoders reject the new tag. Malformed combinations refuse before
scene effects. Resource binding, CPU preparation, per-draw clips and native storage
lifetime use the existing scene path.

`tools/test-additive-sprite.py` prospectively binds the contract and compiler
closure before native/original execution. Its corpus covers all65536 destination
words with64 source colors, every pair of channel values, sparse/transparent runs,
opaque zero, signed anchors, complete and narrowed clipping, and ordered overlap.
The independent original process runs actual580a90 instructions, linked palette
nodes and signed shade selection. Original canvases are comparison outputs only.

This contract excludes original returned status, RGB555/other mask configurations,
live capture and queue admission, actual frame/palette writers and allocation
lifetime, and original pixel bypass. Kind9 placement retains its independent
unchanged-original comparison. Its live source ownership remains separate.
