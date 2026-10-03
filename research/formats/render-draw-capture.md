# Native draw capture, version 1

Produced by `runtime/render/draw_capture.h`; consumed by
`tools/replay-render-capture.py`. These are new instrumentation formats, not
original game assets. All integers are little-endian. Addresses are unsigned
32-bit, process-local evidence tokens; never dereference them during replay.
Capture files are created exclusively in each disposable experiment directory.

## `blit-0001.bin`

One completed eligible Blt or BltFast, with a 128-byte header. Rectangles use
signed 32-bit left, top, right, bottom coordinates; right/bottom are exclusive.
Null API rectangles are normalized to explicit full-surface rectangles.

| Offset | Field |
|---|---|
| 0 | Eight bytes `MNMBLT01` |
| 8 | Version = 1 |
| 12 | Operation: 1 Blt, 2 BltFast |
| 16 | Caller return address, after the original call instruction |
| 20 | Original API flags |
| 24 | Source color key enabled: 0 or 1 |
| 28, 32 | Native-pixel source key low/high values; must be equal in this replay subset |
| 36 | Original successful HRESULT |
| 40 | Source RECT, 16 bytes |
| 56 | Destination RECT, 16 bytes |
| 72, 76 | Source width/height |
| 80, 84 | Destination width/height |
| 88 | Native bits per pixel: 8, 16, 24 or 32 |
| 92, 96, 100 | Red, green, blue masks; ignored for indexed 8-bit |
| 104 | Source pixel byte count |
| 108 | One destination pixel byte count |
| 112 | Palette byte count: 1024 for 8-bit, otherwise 0 |
| 116, 120 | Source/destination interface-pointer tokens |
| 124 | Reserved = 0 |

Payload order is source pixels, destination-before pixels, destination-after
pixels, source palette, destination palette. Pixel arrays are full surfaces
with tight rows in logical top-to-bottom order; producer row padding and
negative pitch are removed. Each palette has 256 RGB/flags entries of four
bytes each. Native pixels retain original index values: matching RGB colors
must not collapse distinct color-key indices. Palette data is for previews;
the exact comparison uses native pixel bytes.

Bounds: source at most 256x256, destination at most 2048x2048. The total file
length must equal `128 + source_bytes + 2*destination_bytes + 2*palette_bytes`.
The parser rejects unsupported versions/flags, failed HRESULTs, color-space key ranges,
out-of-bounds or stretched rectangles, bad RGB masks, truncated payloads and
trailing bytes. A file interrupted during writing is inconclusive.

Supported flags are Blt WAIT, DONOTWAIT and KEYSRC; or BltFast WAIT, DONOTWAIT
and SRCCOLORKEY. Only single exact source keys are accepted, as seen in the
inspected engine wrappers; color-space ranges are deferred. Other effects,
destination keys, clipping, alpha, format
conversion and self-copy are excluded by the producer. Source/destination must
have identical native formats. The destination palette must stay unchanged
across the call. The producer checks canonical IUnknown identity to exclude
aliases of the same surface. Successful read-only locks are released before
the original drawing call; a nonblocking post-call lock must succeed to record
its output. A busy asynchronous blit produces no completed sample.

## `events.bin`

Header: eight bytes `MNMDRW01`, uint32 version = 1, uint32 record size = 64.
At most 2048 records follow. Each contains sixteen uint32 values:

| Word | Meaning |
|---|---|
| 0 | Sequence, starting at 1 |
| 1 | 1 Blt, 2 BltFast, 3 Flip, 4 Lock, 5 Unlock, 6 CreateSurface |
| 2 | Caller return address |
| 3 | Receiver interface token |
| 4 | Source/target token, or created surface for CreateSurface |
| 5 | Original flags; zero for APIs without recorded flags |
| 6 | Original HRESULT bits |
| 7 | Rectangle-present bits: bit 0 destination, bit 1 source |
| 8–11 | Destination RECT; for BltFast, x/y/0/0; for Lock, lock RECT |
| 12–15 | Source RECT |

Absent rectangles have zeroed words. Unlock's argument is deliberately omitted:
older surface interfaces take a pixel pointer, newer ones a RECT pointer.
Original flags/results are recorded even for calls outside the replay subset.
Observer locks use saved methods and never enter this log. Concurrent/reentrant
events may be skipped; the sequence numbers describe recorded events only.
This is a bounded startup sample, not a complete command stream or timing trace.

`replay-render-capture.py` writes its JSON comparison and four PPM previews under
`working/experiments/render-replay/`. Exit codes: 0 exact match, 1 mismatch,
2 inconclusive. It compares the entire destination, including untouched borders.
Read captures after exiting the game to avoid reading a file still being written.

With `--backend opengl`, replay also validates this same capture using
`renderer/capture.cpp`, executes the native integer shader, and writes
`opengl-native.bin`: destination-width × destination-height × bytes-per-pixel
raw little-endian pixels, with tight rows and the same logical row order as the
capture. Its dimensions/format/hashes are in the JSON report. The captured-after
pixels are comparison evidence only; they are not supplied to the shader.
The report includes separate CPU/capture, OpenGL/capture and OpenGL/CPU checks.

OpenGL replay also reads the capture's destination palette/RGB masks for its
presentation shader, writes `opengl-presented.png`, and records a raw RGBA hash.
The Python reference checks that hash against independently converted native
output. Palette-entry flags do not supply alpha; presentation alpha is 255.
The capture format itself remains version 1.
