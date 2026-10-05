# Checkpoints from application-owned GDI surface contexts

Implemented 2026-10-04 for `--capture-locks`. Original drawing remains active.
This observes the application's GetDC/ReleaseDC pair; it does not implement GDI
drawing in the native renderer or establish complete rendering replacement.

## Evidence and confidence

The Forest of Pain difficulty screen was reproduced with the pinned original
working executable in an isolated Xvfb/Wine session. Before this change,
`working/tests/render-menu-delay/run-s1a76e48/` shows Forest of Pain in
`wine-11.png` while `frame-11.png` still contains the previous Main menu fade.
Its frame journal advances during menu fades and then stops during idle.
This reproduces a missing publication, rather than a slow Qt paint operation.

Observation-only run `run-c56ypp9j`, experiment `run-bpt2zjec`, records a successful
application GetDC/ReleaseDC on a 400×280 RGB565 surface. `dc_bitmap` establishes
an 84-byte PE32 DIBSECTION, a selected bitmap, 800-byte pitch and exact
`f800/07e0/001f` masks. Previous code invalidated this surface at GetDC and never
recovered its GDI-written pixels. Later keyed copies from that surface could
invalidate the menu and primary checkpoints. Confidence: high for this observed
context and capture gap; not a classification of every original menu draw.

Final manual reproduction `run-3t7m3kli`, experiment `run-l8dr0b2s`, loads bridge
`dd9026cb6545ff53523792d446a93d2d25a11f13c605273590d391704fe467af`.
Its `frame-7.png` shows Forest of Pain within the three-second screenshot bound
after New Game input; the static title rectangle differs from the simultaneously
observed Wine display by at most one channel level. Publication continues while
the screen remains idle. These are software-X11 live observations; they do not
measure NVIDIA Qt paint latency or complete battle performance.

## Accepted boundary

Surface vtable slot 17 observes the original GetDC result. The tracker invalidates
old pixels and records the application-held HDC, its initially selected HBITMAP,
and owner thread against an already observed surface identity. Slot 26 intercepts
the original ReleaseDC. Only the same HDC, bitmap and thread can provide a copy.
Aliases follow observed QueryInterface relationships; retirement, mutation,
capture misses and generation/epoch changes reject stale work.

Before original ReleaseDC, GetCurrentObject/GetObjectA query the selected bitmap
and GdiFlush completes this thread's GDI batch. Only an 84-byte DIBSECTION matching
the application's observed surface dimensions, RGB bit width and native masks
is accepted. RGB16/24/32 with even row-byte lengths are supported. BI_RGB uses
its standard RGB555 or RGB888 masks; BI_BITFIELDS must match all three masks.
Indexed bitmaps, DDBs, other compression, odd row-byte lengths, replacement
bitmaps and unmatched ownership are rejected conservatively.

GetObject does **not** retain signed bitmap orientation: Wine normalizes its
returned `biHeight` to positive. An initial raw-memory copy failed the real
top-down DIB fixture and was corrected before completion. GetBitmapBits supplies
top-down native rows with 16-bit alignment; admitted even row sizes exactly match
owned tight storage. GDI queries, flush and bitmap copying occur outside the
tracker guard. No bitmap is selected/replaced by capture, and no observer GetDC,
DirectDraw Lock/Unlock or GetDIBits is issued.

The buffer is copied before original ReleaseDC and committed only after success
with unchanged epoch/generation and ownership. Failed ReleaseDC discards the
copy while retaining the borrowed context for an application retry. Successful
but unsupported/mutated release invalidates affected pixels. Primary contexts
may publish their completed checkpoint; normal menu contexts seed existing
blit propagation. Memory reservations share the 64-MiB owned checkpoint bound.
Existing ordered sessions still emit GAP when an already recorded surface is
acquired for GDI; GDI command replay remains outstanding.

API/source references:
[GetDC lifetime](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-getdc),
[ReleaseDC](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-releasedc),
[GetObject](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getobjecta),
[GetBitmapBits](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getbitmapbits),
[Wine DIB_GetObject](https://github.com/wine-mirror/wine/blob/master/dlls/win32u/dib.c),
[Wine NtGdiGetBitmapBits](https://github.com/wine-mirror/wine/blob/master/dlls/win32u/bitmap.c).

## Validation

`tools/test-render-bootstrap.py` cases `dc`, `dc-bottom-up`, `dc-retry`,
`dc-format`, `dc-unmatched`, `dc-swapped`, `dc-rgb24` and `dc-rgb32` create actual
Wine DIBSECTIONs.
An independent fake engine consumes their bits at ReleaseDC and immediately
poisons storage, proving capture precedes release. Original argument, HRESULT,
LastError and call counts are checked. Accepted copies match engine native
pixels, CPU command replay, OpenGL replay and Qt readback; rejected contexts
produce no checkpoint or frame. Report:
`working/tests/render-bootstrap/run-d464ysbi/report.json` (six cases).
RGB24/BI_RGB and RGB32/BI_BITFIELDS also pass in
`working/tests/render-bootstrap/run-xodu0u1l/report.json` (two cases).
Existing opaque, continuous-live and scoped-contention regressions also pass in
`run-1hivj10n/report.json` (eight cases including five GDI fixtures).

`python3 tools/test-render-menu-delay.py` stages the hash-checked original,
disables movies in disposable preferences, uses an isolated display/prefix,
selects New Game and Apprentice with X11 input, compares static title/radio
rectangles with Wine within four seconds, observes ten idle seconds and checks
the live frame through Qt/OpenGL readback. It verifies the immutable manifest
before and after the experiment. This is a bounded live menu test, not a
whole-game equivalence or frame-rate benchmark.

Automated live report: `working/tests/render-menu-delay/run-reir4438/report.json`,
experiment `run-42poztkq`, same production DLL hash above. The Forest title matches
Wine within 2.449 seconds of input; the Apprentice radio change matches within
0.785 seconds. Both rectangles differ by at most one channel level. These bounds
include intentional settling waits and screenshot overhead, and are not exact
first-frame latency measurements. Idle samples advance from count 116 to 310
across 9.995 seconds with no sampled stall; final live Qt readback passes. The
immutable manifest verifies all 2,927 files before and after this experiment.
