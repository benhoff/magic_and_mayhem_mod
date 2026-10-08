# Canvas producer closed-input stream, version 1

A bounded PE32 observation experiment for No-CD build `40209ca7`. Logical IDs
replace runtime pointers. Original canvas snapshots live in separate diagnostic
files and never appear in command payloads. Native storage starts undefined.

The 64-byte header contains `MNMPRO01`, u32 version 1, header size 64, build
`40209ca7`, record capacity (historical 32768 or current 65536), queue target
1..16 and nine zero reserved words. All integers are little endian.

Each record has 24 u32 words followed by an owned payload:

| Word | Meaning |
|---|---|
| 0..4 | Total bytes, contiguous sequence, operation, destination ID, source ID |
| 5..9 | Width, height, pitch in words, signed X and Y |
| 10..13 | Signed rectangle or clip L,T,R,B |
| 14..17 | Four operation-specific auxiliary values |
| 18..20 | Payload bytes, source frame/header bytes, source table/pixel bytes |
| 21..23 | Original return EAX (incoming EAX for a selected native bypass request), diagnostic caller VA, zero reserved word |

Operation IDs are 1 create, 2 release, 3 bind, 4 clip, 5 fill, 6 copy, 7 JPEG,
8 font, 9 raster, 10 checkpoint, 11 queue entry, 12 queue return, 13 failure,
14 RGB addition, 15 bevel, 16 highlight border, 17 whitewash, 18 PCX, 19 DIB, 20 BMP filename, 21 packed-word fade, 22 minimap terrain, 23 point requests.

The optional bounded generic raster bypass publishes its closed request at entry.
Its separate MNMWBP01 request/reply identifies that producer sequence and exact
original entry; record21 preserves incoming EAX because that original body never
returns. Replies contain native reconstructed GPU pixels only. Later destination
checkpoints can include native contributions and require separate original
prefix/exact-entry execution for equivalence; they are not original-only oracles.
See `protocols/include/mnm/world_producer_bypass_v1.h` and
`research/runtime/native-world-producer-handoff.md`.
Copies explicitly name both logical IDs, the source rectangle and destination
position. Auxiliary 0 is `UINT32_MAX` for opaque copies or a native source key.
Creation and release establish generations; IDs never silently inherit bytes.

JPEG payloads contain a NUL-terminated source filename, at most 1024 bytes.
The replay receives separately pinned **encoded source assets**. Source JPEG
extent is cropped to the destination at its top-left, including the observed
600-by-36 image written to a 598-by-36 progress canvas.

Font and raster payloads contain a closed RLE frame (40..1048576 bytes) with
its pointer at offset 28 zeroed. Font appends 272 bytes: format, three RGB565
masks and 64 float coverage values. Auxiliaries 1..3 carry RGB tint. Raster
auxiliaries carry mode, backend, wave period/shade and indexed flag. Indexed
ordinary rasters append 512 effective palette bytes, direct-word copies no
palette, and displacement rasters 64 bytes of effective signed offsets.
Incompatible table/encoding combinations are refused.

RGB addition owns its 20-byte width/height/quantized-channel descriptor. Panel
requests contain their original rectangle and percentage/pressed/reversal inputs;
format 0 is required. The source bevel rectangle uses inclusive bounds and
asymmetric two-pixel edges; border highlights have asymmetric corner ownership.

PCX owns the encoded source file extent plus the 768-byte effective RGB palette.
Source width/height and format are explicit. Native replay refuses padding beyond
the recovered row width. DIB owns its 40-byte BITMAPINFOHEADER, complete RGB
palette and row storage, with source usage recorded. The admitted native policy
is positive-height BI_RGB, one plane, 1/4/8-bit indexed or 24-bit BGR within
2048-by-2048 and 2 MiB pixel limits; unsupported layouts remain explicit errors.

BMP owns a source filename and destination X/Y. The native decoder admits the
same BI_RGB layouts as DIB and clips the decoded source to the destination DC
extent, including the observed 49-by-23 source on a 48-by-22 HUD canvas.

Fade has no payload. Auxiliary0 carries format; RGB565 requires even-width,
packed rows. Each word becomes `(word >> 1) & 0x7bef`.

Minimap terrain owns a row-major array of source-cell RGB565 colors and one-byte hidden flags, separate
from every original destination. Auxiliary0/1 are even grid width and height
(1..256), auxiliary2 is zero for orientation0; auxiliary3 records the fog-enabled flag.
Rectangle L/T carry grid center X/Y; destination X/Y name the first projected
cell. The source wraps around the grid center, while alternating columns and
rows advance through a diamond. Hidden cells read the retained native auxiliary canvas named by source ID,
using its diamond coordinates. Other orientations remain outside this bounded
operation. Point requests own up to16384 triples of u32 X, Y and RGB565 word;
auxiliary0 gives their count. The adapter resolves these outline/marker requests
from original geometry, palette and visibility source state. It never samples
destination words; full native generation of those source states remains pending.

A checkpoint names `producer-oracle-NNNN.565`: tight little-endian RGB565 output,
with identity, extent, reason and queue in the record. The hook only reads a
previously successful lock pointer. Oracles have an independent 1 GiB budget;
input records have a 128 MiB budget. Neither diagnostic bytes nor host pointers
seed native destination storage.

After the final requested consumer returns, `canvas-producers.done` contains
`MNMPDONE`, version 1, completed queue count, record count, failure count,
checkpoint count and input byte count (32 bytes total). A missing marker,
noncontiguous queue, failure, undefined native read or unavailable/mismatched
checkpoint cannot establish complete native equivalence. Unknown writers are
still possible outside the observed scenario and must remain coverage gaps.
