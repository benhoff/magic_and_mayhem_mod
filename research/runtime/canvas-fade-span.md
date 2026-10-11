# NoCD contiguous fade span

Recovered pixel loop: pinned NoCD `0x58ed80..0x58edf7`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Static disassembly is retained under
`working/tests/pixel-producers-fade-discovery/fade.asm`; execution evidence is
registered separately. Confidence is high only within the bounded pixel scope.

The routine locks the surface through `0x58b660`, reads width at object+0x10
and height at +0x14, and iterates width/2 DWORDs for each positive height.
Its pointer advances continuously; it never consults pitch. For positive sizes,
exactly `2*floor(width/2)*height` WORDs are modified, including physical padding
when that padding falls inside this span. With odd widths it does not omit the
last pixel of each logical row: the untouched tail lies at the end of the
contiguous span. The format selector at `0x6e1f88` chooses DWORD masks
`0xfbef7bef` for zero and `0xfdef7def` for any nonzero value. The resulting
independent WORD transformation is `(word >> 1) & 0x7bef` or `0x7def`.

`reconstruction/rendering/fade_span.hpp` models the initialized physical plane.
The native canvas accepts the resolved physical span, stride and WORD mask and
projects its visible pixels; padding has no cross-WORD influence after masking.
The legacy producer adapter supplies the recovered span. Native admission is
positive width/height<=2048 and width<=stride<=4096 with an exact physical plane.
Undefined touched visible words refuse before any mutation. Untouched words do
not require a destination read. This admission policy is not a recovered engine
limit. The default renderer operation still supports a complete logical fade.

The isolated reference maps the unchanged hash-pinned PE privately, checks entry
signatures, and substitutes only its external Lock entry with fixture storage.
A private COM Unlock adapter checks one invocation. Both-side guards protect the
full physical plane. Tests compare every physical WORD and the logical projection
for odd/even widths, padded strides, zero and nonzero selectors, and exhaustive
WORD values. Separate native refusal tests run with ASan/UBSan. No real driver,
lock failure, wrapper ABI/return, live writeback or full-session claim is made.

The final [comparison](canvas-fade-span-comparison-20261010.json) passes 972 cases,
877,072 physical WORDs plus 861,520 visible WORDs, ten atomic native refusals
and ASan/UBSan. Entry anchors, corpus/output hashes and prospective source/scenario
bindings are retained in the immutable report.
