# DRAW-01 selected graphics API boundaries

## Static evidence — 2026-10-11

The [byte-bound export](drawing-api-sites-expanded-static-20261011.json)
selects 314 CALL instructions from the pinned no-CD executable
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
It preserves 84 rel32 calls, 179 register-memory calls, 12 register calls and
39 absolute-memory calls. Of these, 37 have an exact recorded import identity.
An import slot address is not the actual loaded callee address.

Twenty-seven graphics/movie selections include discovered function ranges,
three bounded instruction graphs and the historical WORD-copy window.
Whole-image scanning additionally selects 15 exact graphics-import CALLs
outside those selections: GDI/DDRAW/media graphics DLL imports and USER32
GetDC/ReleaseDC/BeginPaint/EndPaint. This is an explicit predicate, not a claim
that all USER32 or indirect import aliases are drawing operations. Imports
inside selected wrappers retain their exact names, including non-graphics APIs.

Entries `0058c6a0`, `0058cd50` and `0058cf20` have no discovered function entry
in the retained binary inventory. Their selections follow direct branches,
both conditional arms and ordinary CALL fallthrough, stopping at returns or
other exact registered entries. They contain 187/166/165 instructions,
respectively, within `[0058c6a0,0058c894)`, `[0058cd50,0058cf15)` and
`[0058cf20,0058d0e0)`. These normal-return selections do not recover exception
flows or establish function-wide boundaries. Their anchors remain null.
Containing discovered owners are recorded independently, with empty lists
preserved. No nearest function is substituted and the binary inventory remains
unchanged. The WORD window `[0058d3b0,0058d4a7)` retains its unassigned status.

`tools/export-drawing-api-sites.py` verifies the immutable-input manifest before
and after reading the executable, checks its hash before and after disassembly,
uses wide objdump rows, checks supported x86 CALL encodings against operand
text and instruction extent, and refuses existing output paths. Source hashes
are embedded in the export. Unsupported/prefixed/indexed modes are explicit
unresolved records; none occur in this selection. That zero count does not
resolve other indirect flows in the image. The [12 current-source guards](drawing-api-sites-guards-20261011.json)
exercise malformed extents, operand mismatches, signed addressing, exact IAT
identity, empty ownership, crossed ranges, bounded branch selection and missing
instruction/budget refusal. They validate native tooling, not original API calls.

## Remaining runtime contracts

All exported semantic contracts remain unreviewed. Register-memory displacement
alone does not establish a COM interface or method. The existing census SITE
diagnostic stack is not a formal API argument capture and function EAX results
do not supply inner API outcomes. A separate before/after CALL observer needs
to retain concrete targets, original stack/register values, actual cleanup,
result/LastError, reviewed output slots and explicit loss/open/terminal state.
It must forward original operations without issuing graphics calls of its own.

Lock/Unlock and GetDC/ReleaseDC pairing, borrowed pixel/DC lifetimes, COM and GDI
ownership, release/destruction, clipping/format/pitch, failed operations and
recovery remain pending. Static selection does not establish complete drawing
writers, command-to-pixel effects, scenario closure or live replacement.
DRAW-01 still has all 14 scenario closures and four dispatch obligations open.
