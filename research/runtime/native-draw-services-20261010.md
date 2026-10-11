# Native graphics DC, clipper and primary storage admission

The current isolated64-case unchanged caller comparison exercises owned RGB565
DCs and HWND clippers against Wine11.16. It compares2,048 caller fields and
22,464 full COLORREF/RGB565 outputs, including an odd13-pixel width. Native
execution disables ddraw and checks module absence; the native DLL builder
rejects DirectDraw imports. This does not execute the original game.

The supported DC uses an owned top-down RGB565 DIB with four-byte GDI row pitch;
surface storage retains the observed eight-byte pitch. Rows copy explicitly at
GetDC and successful ReleaseDC, with GdiFlush before returning changed words.
DC acquisition during an existing Lock is admitted as measured; a second DC
refuses without changing its output, and Lock while a DC is held refuses.
Repeated ReleaseDC refuses. Bitmap/DC destruction restores the previous selected
bitmap and releases native resources. At most128 DC leases and64 clippers exist.
HWND binding and surface attachment retain/release clipper references. Region
and occlusion APIs and Blt with an attached clipper remain explicitly unadmitted.

Logical primary and one attached backbuffer are native storage policies. A
native-only test verifies two complete117-word histories exchange on Flip,
a duplicate primary refuses, and an independently acquired backbuffer remains
alive after the front is released. All resources return to zero. This is neither
physical scanout nor Wine driver mode/primary/flip equivalence. A logical mode
must be selected before surface creation; selected primary format is RGB565.

A reproducible freestanding builder includes the complete compiler dependency
closure, exports the original two imported names plus NativeDrawAnchor, and
rejects any DirectDraw DLL import. Its signature permits the pinned guarded
import rewrite. Original section bytes and IATs remain unchanged by that rewrite.

Remaining boundaries: real game startup with ddraw disabled, unknown methods,
clipped/occluded window copies, palettes/indexed primary, lost/device routes,
CPU dependencies on native GPU World history, public menus/gameplay, real
presentation and19/60FPS. The isolated proofs do not promote whole-backend
retirement. Static setup and surface wrapper addresses remain scoped in the
[previous discovery](native-draw-device-surfaces.md).
