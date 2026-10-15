# Production glyph takeover

The native glyph kernel now has explicit RGB565/RGB555 storage format. The
original format changes the red mask/shift, green mask and tint division, and
the returned ECX shift. Existing default APIs retain RGB565 semantics. A new
format-specific state entry and appended native backend format field preserve
the established incoming/outgoing FXSAVE offsets.

The unchanged original/native RET16 fixture now exercises both formats. A
separate production policy uses `MNM_GLYPH_CAPTURE=0` with takeover: no finite
rendering-call limit, no per-call diagnostic output, frame allocation or full
canvas trial copy. The engine owner thread proves disjoint frame, canvas and
coverage spans, and the native kernel completes its integer preflight before
committing pixels/table/FX/caller state. No original callback occurs inside
that admitted kernel operation. Concurrent source mutation and borrowed lease
ownership outside this synchronous owner-thread scope remain excluded.

Resident menu staging can compose this mode with the existing producer hooks.
Outside the protected World batch, the ordinary producer return hook records
the source request and completion after native RET16. This preserves normal
publication order. Protected World batches retain their explicit drawing
contract; glyphs are not silently added to their whitelist. Installation must
guard the original lazy-table multiplier and truncation helper in either mode.

The production stats use `MNMGLST2`, version2,144 bytes, with budget0 meaning
unbounded and samples0. This is distinct from the historical bounded diagnostic
`MNMGLST1`. RGB565 diagnostic samples keep `MNMGLIV1`; RGB555 samples use
`MNMGLIV2`, version2, with header WORD19 identifying format1. Both have the same
256-byte header and bounded frame/canvas/coverage/FX payload lengths.

Known unsupported glyph FP controls, occupied push slots, malformed RLE,
invalid/aliased spans and unsupported warm coverage still forward before
native writes. Higher text layout and resource lifetime remain original
preparation; they do not themselves paint these glyph pixels. This change is
not evidence of live resident stream equivalence, layout takeover or complete
original renderer retirement. The controlled production admission fixture passes65,545 native/0 original requests with no drawing WriteFile or
full-canvas trial copy. Public native-draw takeover staging selects capture0 in
both standalone and resident modes; shadow diagnostics retain capture1.
The format fixture passes18,760 executions and14,857,536 physical WORDs across
12 precision/rounding controls. Integrated resident publication remains pending.
