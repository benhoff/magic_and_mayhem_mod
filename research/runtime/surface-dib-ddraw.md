# Surface2 DC bitmap reload and RGB565 presentation

## Confirmed execution and scope

Two serialized, reserved Wine 11.16 runs exercise real DirectDraw Surface2
GetDC, GDI StretchDIBits and ReleaseDC. Each has 72 cases: six retained original
loader inputs, equal/cropped/larger destinations, RGB565/RGB32 formats, and
system-memory/default requested caps. The inputs include indexed 1/4/8-bit,
24-bit, sequential-offset-gap and installed cursor bitmaps. Original helper
addresses 0x486a80 and 0x58d1a0 identify the already captured input/caller
boundaries; these probes execute no original game instructions.

The [first capture](surface-dib-ddraw-capture-20261006.json) retains native
Lock words and three DC GetPixel samples per case. The
[second capture](surface-dib-ddraw-rgb-capture-20261006.json) additionally retains
every DC GetPixel output before ReleaseDC. Reports, independent outputs,
probe hashes and environment DLL hashes are preserved separately. Immutable
input manifests pass before and after each run. No simultaneous Wine process
is allowed by the shared capture reservation.

Targets start at native word 0x2bab (RGB565) or 0x00556677 (RGB32).
StretchDIBits writes at origin without scaling, crops to the target and leaves
larger borders unchanged. It uses the original submitted usage values and
SRCCOPY. The input parser reads pixels sequentially after the palette rather
than seeking to bfOffBits. GdiFlush precedes DC inspection and ReleaseDC;
final Lock captures owned native words without pitch padding.

## Observed admission and outputs

These exact values are Wine Surface2 observations, not a generic Windows
hardware guarantee. All 72 cases produce the following results:

| Operation | HRESULT/result | Output state |
| --- | --- | --- |
| Create, initial/final Lock and Unlock | 0 | Requested dimensions/masks retained |
| GetDC during an existing Lock | 0 | New non-null DC |
| Fresh GetDC | 0 | New non-null DC |
| Second GetDC | 0x8876026c | Initial 0xabababab output unchanged |
| Lock while DC is held | 0x887601ae | No successful native read |
| ReleaseDC | 0 | DC released |
| Repeated ReleaseDC | 0x8876024a | Refused |
| GetObjectA for selected bitmap | 24 bytes | Target dimensions and bit depth |
| StretchDIBits | Source height, 6 or 280 | Raster submitted |
| GdiFlush | 1 | Success |

Observed caps are 0x840 for system-memory requests and 0x10004040 for default
requests. This does not prove hardware storage. The general
[GetDC documentation](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-getdc)
describes a lock until ReleaseDC; the Surface7 documentation is not evidence
for the measured Surface2 GetDC-during-Lock admission branch.
Fields named `dc_first_rgb`, `dc_middle_rgb`, `dc_last_rgb` and full `dc_pixels`
retain raw COLORREF values (0x00BBGGRR), converted explicitly for comparisons.
The unused RGB32 high byte is retained and excluded from RGB equivalence.

## Native implementation and validation

RGB565 packing truncates RGB channels to their top 5/6/5 bits. The DC expands
those words by bit replication: `(r << 3) | (r >> 2)`,
`(g << 2) | (g >> 4)`, `(b << 3) | (b >> 2)`.
The [full offline comparison](surface-dib-ddraw-rgb-offline-20261006.json)
matches 1,346,928 native words and 1,346,928 DC RGB values. Every 32/64/32
channel level is represented. The previous native floor-scaling policy differs
in 89,860 RGB565 pixels; the earlier three-point sample detected 70 differences.
Seven native-word/admission mutation tests and three full-frame RGB mutation
tests pass, including rejection of the previous floor model.

`GlBlitter::reloadDib` now supports canonical RGB565 alongside RGB24/32.
Both native readback and texture presentation expand RGB565 by bit replication.
Other masks keep their existing policy. Attached clippers, indexed destinations,
RGB555 and unsupported DIB layouts still refuse; only the cropped written
region becomes valid, and unknown borders remain unreadable.
The [native comparison](native-ddraw-dib-20261006.json) matches all 72 independent
native/DC frames in 2,693,856 pixel comparisons, with 24 scripted recovered
cursor compositions and 253 refusals. It also freshly compares 1,456 recovered
C++ dispatch projections, 12 original submitted-buffer cases, 813 bounded stops
and two invalid budgets. The [RGB24/32 compatibility result](native-cursor-after-ddraw-20261006.json)
retains 36 cases, 12 compositions, 137 refusals and 1,346,928 GDI RGB comparisons.

## Reproduction and remaining boundaries

Offline checks need Python only and no Wine or original installation:

```sh
python3 tools/check-surface-dib-ddraw.py
python3 tools/check-surface-dib-ddraw-rgb.py
python3 -B tests/test-surface-dib-ddraw.py
python3 -B tests/test-surface-dib-ddraw-rgb.py
```

Build `renderer` and run `xvfb-run -a python3 tools/check-native-ddraw-dib.py`
with a new `--report` path. Recapture with either matching collector under Xvfb,
using new corpus/report paths. Collectors preserve existing evidence and reserve
one Wine session; native GL checks start no Wine.

Native DC lease/admission implementation, indexed destination palette conversion,
DC clipping, actual-lost-draw/Restore composition, other reload callback bodies,
Windows hardware and live/wire recovery remain pending. The runtime CPU frame
adapter and retained CPU replay preview still use their historical floor-scaling
policy; this change does not claim bridge color equivalence. Their alignment
requires a separate bounded adapter contract review. Prior evidence fingerprints
remain historical when shared renderer source changes.

## Subsequent indexed/DC clipping step

[Indexed reload and DC clipping](surface-dc-palette.md) now record separate
DirectDraw versus GDI region behavior and bounded indexed8 conversion with an
explicitly installed full palette.450 native cases match43,200 independent
native/DC pixels. Canonical indexed reload and attached-clipper refusals above
are superseded within that documented scope. Default palettes, native DC leases,
actual loss and live/wire recovery remain pending; prior report hashes are retained.
