# Positioned DIB producer source

Existing producer operation19 retains owned BITMAPINFOHEADER/palette/encoded rows.
Words8/9 now carry signed destination x/y for source wrapper `0x58d370`; the
previous `0x58d240` wrapper continues to record zero origin. No pointer or
destination pixel enters the wire. Width/height/stride remain destination metadata.
Word14 is original DIB usage, word19 header/palette bytes and word20 encoded
row bytes. Source dimensions, bit depth and bottom-up row layout come from the
owned40-byte BITMAPINFOHEADER. Canonical1/4/8/24-bit BI_RGB inputs only.

Static evidence: supported No-CD SHA40209ca7…, wrapper `0x58d370..0x58d3aa`
obtains Surface2 GetDC, calls `0x486a80` with bitmap/x/y, then ReleaseDC.
Helper `0x486a80..0x486ad2` reads object info/pixels, uses source width/height,
zero source origin, equal destination extent, SRCCOPY and usage0/1. Confidence:
confirmed static arguments and live source observation in
`research/runtime/storage-action-original-20261016.json`. The corresponding
238frame native comparison restores the confirmation backgrounds but still
fails ten separate cursor-region presentations; it establishes no general
GDI state/return/ABI equivalence.
The hook verifies eight whole original instruction bytes before patching and
preserves the existing register/flags/x87/SSE entry/return convention. It is
installed only in explicit original-active producer trace mode.
