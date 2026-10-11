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

## Separate API wire and direct-splice planning — 2026-10-11

The versioned `protocols/include/mnm/drawing_api_journal_v1.h` defines separate
128-byte BEFORE/AFTER rows with actual ESP, registers/flags/LastError, concrete
register or memory target identities, eight pre-CALL stack words and explicit
read states. AFTER repeats the saved target and never dereferences a released
object. Its actual stack delta is diagnostic; no calling convention is guessed.
The strict decoder checks exact hook/build/phase/thread/target identities,
serial call IDs, per-thread LIFO pairs, active counts and closed loss-free
terminal state. API outputs and borrowed resource descriptors are not included
in this initial wire version.

The [current-source static splice plan](drawing-api-splices-accounted-static-20261011.json)
accepts 250 of 314 selected CALLs alongside the 83 existing census spans.
Fifty-two refuse incoming direct branches into overwritten interior bytes;
12 overlap existing original census hook spans, chiefly the movie SITE probes.
These refusals remain pending observer coverage. The plan verifies file-backed
bytes, selected range containment and source stability before/after execution.
It relocates the original CALL, jumps to an AFTER stub, then runs the copied
remainder and resumes the exact original continuation. Short conditional/jump
encodings expand with their original targets. A return before five bytes refuses
copying into the next function. No original argument/return slot is replaced.
An actual callee would see a trampoline return PC; universal transparency,
caller-PC-sensitive behavior, timing and arbitrary indirect entry are unproved.
This increment only plans those operations; no runtime hook is executed.

[21 fresh guards](drawing-api-journal-guards-20261011.json) cover wire corruption,
negative HRESULT/LastError retention, actual signed cleanup, explicit unreadable
state, register/IAT identities, interleaved/reentrant pairing, relocation and
splice refusal. The [first planning output](drawing-api-splices-static-20261011.json)
is retained unchanged with an explicit [source-mutation limitation](drawing-api-splices-source-mutation-20261011.json):
a guard edit occurred during that process, so its end-of-run hashes cannot
establish the loaded planner version. It is not registered as current evidence.
The fresh plan freezes and rechecks its inputs. Committed historye4be448..f2d858b
has12exact file/14behavior receipt matches, without missing versions.
