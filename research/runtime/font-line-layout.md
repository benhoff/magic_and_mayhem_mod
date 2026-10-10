# Higher byte text line layout

Reviewed 2026-10-10. `RS.font-line-layout` owns the selected NoCD
rectangle draw (`0x4a5ce0`), height-limited draw (`0x4a6190`) and measurement
(`0x4a6630`) contracts. Pinned executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

Static exports: `working/decompiled/font-text-yczpauwn/`, produced read-only by
`tools/export-font-text-support.py`. Decompilation and disassembly identify
32-DWORD contour resets, line triples at object `+0x238`, count at `+0xe38`,
unsigned horizontal wrap tests, flags 2/4/8/16, and separate final-byte
measurement. The initial isolated comparison matches 16,352 cases across all
six installed fonts: 133,955,584 RGB565 words and full cursor/contour state,
line triples and normalized consumed pointers. Source, object and padded canvas
guards pass. The final source/scenario-bound [comparison](font-line-layout-comparison-20261010.json)
passes 16,480 cases and 135,004,160 RGB565 words, including retained contour rows
above 32 and wrapped tracking boundaries; all native refusal/sanitizer checks pass.

The object stores x/y at `+0x218/+0x21c`, line height at `+0x220`, tab advance at
`+0x224`, tracking at `+0x230`, 256 triples `{x,count,source}` at `+0x238`, and
line count at `+0xe38`. Each line resets exactly 32 contour DWORDs, retaining
active rows above 32. Width accumulation and comparisons are unsigned modulo
32 bits; half-rectangle distances use signed division toward zero. Center flag
4 takes precedence over right flag 2. Flag 8 centers the block vertically using
font ascent+descent, independently of the supplied line height. Flag 16 wraps
using a one-byte lookahead, saved advances and separator backtracking.

Rectangle draw returns the last measured line width; measurement returns the
maximum. The measurement helper accepts a null source or a rectangle shorter
than ascent+descent by clearing line count only. Rectangle draw similarly clears
line count for the empty string. Height-limited draw preserves its special first
x, uses line height for the unsigned cutoff and has a distinct final-byte
look-behind on that cutoff. Final width measurement adds the last byte again;
newline handling has a separate preceding-byte advance. The long-word branch
is sticky across subsequent lines. Rendering skips unknown/control bytes except
tab/newline; consumed pointers still advance, and line source pointers are
mutated during rendering. Native glyph requests retain original byte advance
and suppressed space/underscore behavior from `RS.font-byte-consumer`.

`compat/legacy/font_layout.*` binds owned SFT data and stages the entire canvas
until all requested glyphs succeed. Nine admission/refusal cases pass in Debug
and ASan/UBSan, including a later invalid glyph after an earlier valid draw.

Native admission bounds source reads, stack-equivalent advance arrays and the
256-line table. Refusing original out-of-bounds cases is an intentional native
policy. This work does not establish font allocation/destruction, arbitrary
machine state, original malformed-input behavior or live text entry bypass.
