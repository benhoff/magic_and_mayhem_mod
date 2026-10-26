# Ten owned native text services

This packet adds an application text session and native RGB565 pixel delivery to
the partial actor/font and SFT work. Existing `compat/legacy/FontText`, raw font
providers, recovered byte/line algorithms and the Qt outline-font menu path are
suppliers or separate consumers; this packet does not rediscover them or grant
them new original comparison credit.

| Native component (`NP.text.` prefix) | Added service |
| --- | --- |
| font-resource | Read-only SFT file acquisition, strict complete metric/glyph admission, immutable ownership and file-independent lifetime |
| byte-advance | Session contour state with optional commit and nonmutating probes |
| byte-placement | Owned glyph plan and atomic pen publication for one byte |
| string-placement | Bounded terminated byte runs, suppression and whole-run rollback |
| rectangle-layout | Application-bound rectangle layout and owned glyph/line results |
| height-layout | Application-bound height-limited layout and consumed-source results |
| measurement | Nonmutating measurement; supplier cursor result remains explicit |
| cpu-pixels | Atomic clipped RGB565 raster from owned glyph planes and signed origins |
| gpu-pixels | Same owned plans through actual GPU uploads and clipped sprite composites |
| gpu-cache | Bounded glyph/palette-value LRU reuse, eviction and explicit retirement |

The application service owns validated strict-v3 SFT data:1..223 glyphs,
0..128 metric rows, complete contours for every glyph, nonnegative ascent/descent
with total1..128, per-glyph128x128 and at most1Mi pixels. File loading limits
encoded input to2MiB. There is no fallback that invents missing contours.
Constructing from a supplied SFT value takes its own copy/move; font access is
immutable. Aliased shared sprite ownership keeps draw plans alive after session
and resource handles are released. No raw file or original object pointer survives.

Default pen tracking is1 and tab advance20. Cursor restores require exact metric
row count. Byte state uses the existing recovered modulo32-bit suppliers.
Strings have a single final NUL and at most65536 bytes; a late unsupported byte
preserves the session. Newline in an unwrapped run is a suppressed byte, not a
paragraph command. Rectangle and height layouts publish only after their complete
supplier result and owned drawing plan succeed. Line height defaults to font
height; explicit values must be1..256. Measurement leaves the session untouched,
while retaining the supplier's result state. Existing original unsafe layout
look-behind/stack/line-table domains continue to refuse.

The native renderer depends only on assets and existing sprite rendering;
build-specific layout stays in the application. Pixel delivery uses embedded
palette/RGB565 colors or a caller-provided indexed color table. Masked opaque
zero is retained. This is native opaque drawing, not original tinted/x87 coverage
blending, Unicode shaping, Qt outline rendering or RGB555 equivalence. Both
raster paths clip signed origins in64-bit coordinates. CPU rendering validates
all input and publishes a candidate image atomically. GPU rendering preflights
logical input and local pixel budgets before drawing; later physical GL or global
allocation failures may leave an accepted prefix. No GPU transaction is claimed.
Destinations use complete RGB565 storage. At most65536 glyph requests and16Mi
source-pixel work are admitted per draw. GPU sources must remain immutable and
plans must match their cache's font identity. Renderer lifetime/thread contracts
apply; GPU operations do not borrow application widgets.

The GPU cache owns two planes per nonempty glyph. Defaults are16 entries and1Mi
color+mask pixels; configured limits are1..223 entries and1..8Mi pixels. Palette
keys are complete values, not caller pointer identities. Local eviction may occur
before allocation; global renderer budgets remain independent. Resident repeats
must add no upload/readback. Clear and destruction release only owned entries.

`tools/test-rendering-twentysixth-ten.py` binds prospective contracts and source
hashes before Release and ASan/UBSan controls. Synthetic SFT files, independent
CPU expected pixels, signed extreme coordinates, sparse/opaque-zero masks,
palette variants, layout supplier composition, pen rollback, ownership, cache
budgets, LRU and retirement are bounded controls. CTest launches the same native
service through a private software OpenGL display. The existing SFT loader control
is a regression supplier. No original artifacts or live game are consumed.
Original layout semantics retain their historical evidence; this packet's
native application/resource/delivery policies receive no original comparison or
full-scene/hardware/performance/live replacement credit. The actor and SFT parents
remain partial.
