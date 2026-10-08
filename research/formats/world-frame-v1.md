# Owned World raster inputs, version 1

`protocols/include/mnm/world_frame_v1.h` defines a little-endian, pointer-free
input file for bounded World canvas comparison. Magic is `MNMWRLD1`. This is a
closed-file observation contract, not a continuous IPC channel or takeover
protocol. Original before/after RGB565 canvases are separate oracle files and
are never embedded in native inputs.

## Header: 80 bytes

| Offset | Field |
| --- | --- |
| 0 | Eight-byte magic |
| 8, 12, 16 | Version 1, header size 80, exact total bytes |
| 20 | Nonzero capture sequence |
| 24, 28, 32 | Canvas width, height, original stride in WORDs |
| 36, 40 | Raster request count, capture failure code (must be zero) |
| 44 | Pinned No-CD build token `0x40209ca7` |
| 48..60 | Initial left/top/right/bottom clip, signed coordinates |
| 64, 68 | Original consumer mode and auxiliary-pass flag; diagnostics |
| 72 | Original colour format; only zero (RGB565) admitted natively |
| 76 | Original consumer EAX at return; diagnostic, not a native input |

Bounds: width/height 1..2048, stride width..4096, 1..32768 requests, total
at most 32 MiB. A successful trace records effective primitive requests in
their original execution order, including clip changes made by the consumer.
It does not replace the consumer's state changes or compute upstream camera,
visibility, animation or palette generation.

## Record: 64-byte header, encoded frame, owned payload

| Offset | Field |
| --- | --- |
| 0, 4 | Exact record bytes, operation |
| 8, 12 | Signed X/Y anchors before subtracting the SPR origin |
| 16..28 | Effective left/top/right/bottom clip, exclusive right/bottom |
| 32, 36, 40 | Encoded frame bytes, indexed flag 0/1, owned payload bytes |
| 44, 48 | Requested shade/amplitude and original phase; diagnostics |
| 52, 56, 60 | Backend tag, palette-chain ordinal, displacement period |

Encoded bytes use the recovered SPR frame layout. The word at frame offset 28
is normalized to zero, so no palette or host pointer crosses this contract.
Frame extent is 40 bytes..1 MiB. Its normalized bytes and indexed flag identify
a frame in an explicitly pinned installed SPR asset. Every request must bind;
there is no supported-only omission mode. Exact encoded aliases with different
embedded palettes are permitted because this contract supplies actual colours
(or displacement, which uses coverage only). The original snapshot adapter's
unshaded ambiguity refusal remains its default.

| Operation | Payload and meaning |
| --- | --- |
| 0 copy | Indexed: 256 RGB565 WORDs; direct-word: no payload |
| 1 half | Same table layout; integer source/destination half blend |
| 2 quarter-source | Same table layout; selected original quarter-source blend |
| 3 displacement | Sixteen signed DWORD rightward offsets, period 1..16 |
| 4 quarter-destination | Same table layout; selected quarter-destination blend |
| 5 projected shadow | Same table layout; recovered alternating-row coverage and destination darkening |

Non-displacement periods must be zero. Native displacement admits offsets
0..16 and refuses sampling outside its owned canvas. Blend and shadow arithmetic
is described in [World rendering research](../runtime/native-world-frame-rendering.md).
Clip rectangles must be nonempty and wholly inside the native viewport. Unknown
operations/tags, retained pointers, bad extents, trailing bytes, failed captures
and unbound resources refuse the whole frame before scene rendering.

Backend tags 0..10 identify black, selected generic indexed, selected terrain,
generic clipped, half, quarter-destination, quarter-source, wave, direct-word,
selected distortion and projected shadow respectively. Tags identify capture
provenance; they do not expose original addresses to native renderer services.

Capture failure codes: 1 unreadable frame, 2 invalid frame extent, 3 exhausted
record/byte budget, 4 invalid colour chain/table, 5 changed canvas identity or
stride, 6 oracle write failure, 7 unsupported/uninitialized wave state, 8 unknown
queue kind. A failure file is diagnostic evidence and cannot be rendered as a
complete frame. Hook installation/admission failures can instead yield no World
file; the capture tool times out and refuses success.

Normal live shadow delivery may publish a diagnostic-only 80-byte header with
failure 7 or 8 and raster count zero. It excludes partial payload and is never
admitted by `decodeWorldFrame` or finite rendering tools. A separate strict
refusal decoder accepts only this closed envelope with valid build, sequence,
dimensions and colour format. The [channel policy](world-channel-v1.md) controls
waiting and recovery; verification retains fatal capture failures. No header or
record layout changes are involved.

The CLI owns a constant initial RGB565 background (zero by default). It never
loads the original before canvas. The independent comparison must demonstrate
that these drawing inputs construct the entire claimed canvas from that native
base. This format alone cannot prove that every pixel producer was observed.
