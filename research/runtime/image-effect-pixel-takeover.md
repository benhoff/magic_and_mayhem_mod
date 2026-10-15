# Image and effect pixel producers

Pinned No-CD build `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The implementation keeps original resource preparation, clipping setup and
surface Lock/Unlock or GetDC/ReleaseDC around native integer pixel passes.
It does not substitute captured original output or alter gameplay rules.

## Recovered boundaries

| Producer | Original entry | Interior boundary | Native scope |
| --- | --- | --- | --- |
| PCX locked surface | `0x54b2d0` | `0x54b31d` to `0x54b4f1` | RLE indices and effective RGB palette to RGB565/555 |
| RGB add | `0x545c10` | `0x545ce9` to `0x545dae` | Prepared positive clipped rectangle; WORD wrap before saturation |
| PCX byte buffer | `0x54b510` | `0x54b53d` to `0x54b5e5` | Raw indices with destination byte stride |
| PCX WORD buffer | `0x54b5f0` | `0x54b618` to `0x54b7f3` | Raw RGB565/555 pixels with destination WORD stride |
| Brighten | `0x58d770` | `0x58d7e7` to `0x58d8a1` | Contiguous width*height prefix, including physical padding |
| Wave wrapper | `0x58d3b0` | `0x58d430` to `0x58d47c` | Row-forward copy; original sine preparation and both Unlocks retained |
| Fade | `0x58ed80` | `0x58edb0` to `0x58ede5` | Existing contiguous physical-prefix kernel, without synchronous captures |
| BMP/DIB helper | `0x486a80` | CALL `0x486aca` | Native painting into the current owned DC lease |

BMP/DIB surface wrappers `0x58d1a0`, `0x58d240`, `0x58d280` and
`0x58d370` all reach the shared helper. The last positioned in-memory wrapper
and both raw PCX routes extend the earlier producer list. Pinned entry anchors and isolated original comparisons confirm these selected
boundaries. The range comparison records5,004 native cases and5,765,312 physical
WORDs, with source/callback/caller-state equality. Fresh final-source evidence is
recorded separately; historical reports retain their fingerprints.

The wave wrapper computes sine tables that its subsequent pixel loop does not
use. Its copy treats the low WORD of each surface's pitch field as a WORD
stride, unlike the byte-pitch PCX surface wrapper. Retain that distinction;
it is not permission to read outside a proved destination/source span.

## Admission and lifecycle

`MnmPixelWriters.dll` checks every entry and tail signature before changing any
site, allocates executable forwarding trampolines, and acquires all affected
page protections before patching. Rejected calls forward once before writing.
The producer owner is one process-local thread; DF-set, malformed/truncated
RLE, row-crossing runs, zero runs, aliased source/palette, invalid geometry, unreadable regions and unbounded extents refuse. These checks are native policy,
not claims about the original's behavior on malformed pointers or files.

The DIB call boundary is versioned and process-local, not a host wire message.
Only currently registered owned HDC leases can admit it. The adapter proves
source header/palette/pixel readability first; the native backend admits
canonical positive-height BI_RGB 1/4/8-bit full palettes and 24-bit BGR,
unscaled SRCCOPY, source origin zero, RGB565 and clipped destination offsets.
Unsupported layouts retain the original GDI call. GetDC/ReleaseDC lifecycle
and the separate owned DIB storage remain. Successful native painting has no
GDI drawing call; DC creation, flushing and disposal still use GDI services.

There is no rendering call limit or per-call diagnostic file I/O. Caller
registers, defined flags, selected floating-point state, LastError and original
Unlock order must be compared independently before any replacement claim.

## Remaining boundaries

JPEG, minimap, glyph, colour rectangles, outside-World raster families and
owned DC copies now have separate native pixel implementations and guarded
launch policies; see [native pixel producers](native-pixel-producers.md).
Original preparation/wrappers, unknown writers and complete presented journeys
remain outside this pixel-loop scope. Integrated gameplay comparison and full
renderer retirement remain separate milestones.
