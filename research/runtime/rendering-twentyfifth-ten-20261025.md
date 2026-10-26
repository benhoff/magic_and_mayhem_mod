# Ten native bitmap rendering domains

This batch extends the partial `AS.bmp` native image service. The existing
resource manager and renderer consume its owned RGB output. These are ten
small native implementation scopes, selected from the decoder's explicitly
unsupported domains; the installed BMP inventory already fits the older 24-bit
path. They do not advance original engine reconstruction or close a full-scene
milestone. The ten stable component IDs are `NP.bmp.` plus:

| Component | Input domain |
| --- | --- |
| indexed1 | 40-byte header, uncompressed, most significant bit first |
| indexed4 | 40-byte header, uncompressed, high nibble first |
| indexed8 | 40-byte header, uncompressed palette indices |
| rgb555 | 40-byte header, uncompressed 16-bit RGB555 |
| bgrx32 | 40-byte header, uncompressed 32-bit BGRX, ignored high byte |
| masks16 | 40-byte header, three external contiguous disjoint 16-bit masks |
| masks32 | 40-byte header, three external contiguous disjoint 32-bit masks |
| rle4 | Bottom-up RLE4 runs, literals, EOL, EOB and delta |
| rle8 | Bottom-up RLE8 runs, literals, EOL, EOB and delta |
| core | 12-byte core header, unsigned dimensions, 1/4/8/24-bit rows |

The format baseline comes from Microsoft's
[bitmap information header](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader),
[compression description](https://learn.microsoft.com/en-us/windows/win32/gdi/bitmap-compression),
[core layout](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapcoreinfo), and
[mask constraints](https://learn.microsoft.com/en-us/previous-versions/dd183376(v=vs.85)).
No original address is assigned to these native policies.

All output remains owned, tightly packed, top-down RGB. Default indexed palette
counts follow pixel depth; explicit counts must fit that depth, with every used
index checked. Optimization palettes for direct pixels remain extent-checked.
Four-byte row padding and both signed-height orientations apply to information
headers; core images are bottom-up. No alpha or transparency is inferred.
Masks must be nonzero, contiguous, disjoint and contained in pixel depth;
channels scale by `floor(value * 255 / maximum)`, an intentional native policy.

RLE uses its nonzero declared image extent, requires EOB, refuses top-down
images, row crossings, invalid indices and out-of-range deltas. Literals consume
word-aligned encoded bytes. An EOL after the final row may precede EOB; further
writing is refused. Zero deltas are allowed and consume finite input. Skipped
cells use palette index zero, an explicit deterministic native policy. Unused
bytes after EOB inside the declared extent remain accepted. Input/output budgets
and complete file/table/mask extents are checked before output allocation.
No failed decode publishes a partial image.

`tools/test-rendering-twentyfifth-ten.py` declares exact contract/source hashes
before controls. Independent Python fixtures emit raw BMP and expected RGB bytes
under ignored, retained `working/tests/`. CPU controls compare all pixels,
exercise truncations and bounded deterministic hostile mutations. Existing BMP,
manager and preparation controls cover regressions. GPU controls use the real
manager/cache/upload/draw path, compare RGB565 pixels, resident reuse and balanced
retirement/reload, using a private software OpenGL display. Release and ASan/UBSan
are separate runs. Leak scanning of external Qt/driver code is disabled; owned
resource counts are checked. A fresh immutable SQLite report is required for
current execution credit. No original artifacts, installed files or game process
are needed. These controls do not establish original loader errors, hardware
behavior, performance, live replacement or complete scene acceptance.
