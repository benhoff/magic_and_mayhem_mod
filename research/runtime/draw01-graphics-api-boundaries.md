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

## Separate forwarding observer and synthetic ABI — 2026-10-11

The [built PE32 observer](drawing-api-observer-build-20261011.json) contains
250 exact original signatures and retains the64refusals. Every site is checked
before any patch; startup refuses protection/allocation/cache-flush failures.
An incomplete installation emits no terminal marker. The separate opt-in DLL
uses the existing read-only phase control and writes `api-calls.bin`; normal
process detach closes its independent journal. Busy detach, dynamic unload,
arbitrary caller-PC-sensitive behavior and timing equivalence are unsupported.
It executes no graphics, GDI or COM calls itself.

[The first synthetic ABI fixture](drawing-api-observer-synthetic-abi-20261011.json)
passes10bodies using absolute-memory/register/register-memory/rel32 calls and
0/4/8/12/16/20/24-byte callee cleanup. It exercises actual dynamic splices and
BEFORE/AFTER stubs, including a copied short conditional branch. The selected
register/flag/x87/SSE snapshots and LastError match baseline; each body executes
once, the pre-CALL stack begins with the actual argument, target values match,
and post-CALL ESP reports the actual cleanup. The chosen negative EAX result
is retained without assigning it an original API meaning.

Frame exhaustion, persistently invalid phase, record cap and IO-failure
bookkeeping states preserve the fixed original continuation and synthetic
results. Nested bookkeeping requires the latest frame on the same thread;
a mismatched older tag records loss instead of assigning a false pair. These
fixtures test selected synthetic bodies, not the250original bodies, complete
installation/shutdown, arbitrary machine contexts or original resource/output
lifetimes. Their original-renderer and original-splice-manifest execution flags
are explicitly false. The stopped fixture prefix was removed after preserving
binaries, generated hooks, logs and hash-bound report. Actual original API
journals and the64refused-site observer boundaries remain pending.

## Exact per-run allocation/module bindings — 2026-10-11

`drawing_api_bindings_v1.h` adds a separate128-byte header/48-byte row/64-byte
terminal map. It records each prepared trampoline's exact original site/CALL
return, physical trampoline CALL return, fixed continuation and stub PCs, plus
API/core/menu module bounds and the named native `census_leave` export. Those
identities are read anew per launch; no address stability is assumed. A closed
map proves prepared allocations, not that every hook installed. The independent
closed API journal and original process exit remain necessary.

[12 mutation guards](drawing-api-bindings-guards-20261011.json) reject changed
extents/tags/instruction identities, reserved/terminal fields, module/stub bounds,
overlapping allocations and alias stubs. Exact trampoline returns map to exact
original CALL sites; other module/allocation membership remains diagnostic and
logical ownership remains unassigned. An exact census leave address retains
pending stack/thread/tail-flow review. Original unassigned caller windows are
not assigned to the nearest discovered function.

The [fresh real writer/decoder round trip](drawing-api-observer-bindings-abi-20261011.json)
passes all10synthetic ABI bodies again and decodes the actual C-written binding
file against an explicit synthetic hook manifest. Its API module is the fixture
EXE and core/menu modules are explicitly absent; it executes no original splice
manifest or graphics API. The [fresh census ABI](drawing-census-named-export-abi-20261011.json)
checks current-source0–24cleanup, site/tail/phase/exhaustion and selected machine
state forwarding after the named export. That ABI fixture does not establish
original logical caller ownership. [The current DLL build](drawing-api-observer-bindings-build-20261011.json)
retains250accepted/64refused sites and additionally pins the immutable approved
splice-plan SHA256. Older source/claim fingerprints remain unchanged and their
current validation limitations are retained. Actual original journals and
all14scenario/four dispatch closures remain pending.

## Corrected export link and first original API journal — 2026-10-11

A post-run export-table inspection found that annotating the declaration of the
assembly-defined census leave stub did not actually export it. The [explicit
limitation](drawing-census-export-declaration-limit-20261011.json) retains the
prior binary/report hashes and exported names: the earlier ABI validates callback
forwarding, not symbol visibility. Core DLL and fixture links now pass
`/export:census_leave` and refuse a missing name in the actual PE export table.
The [fresh export-checked ABI](drawing-census-export-checked-abi-20261011.json)
passes with `named_leave_export_checked=true`. The original-site plan is
[re-exported with current source fingerprints](drawing-api-splices-export-checked-static-20261011.json);
250accepted/64refused sites remain unchanged. Older plans/results are preserved.
The committed dc626c7..6e85d84 history review retains this semantic limitation
separately from its exact receipt matches.

The explicit `--api-observation` staging option enables the separate observer
alongside the core census; menu-only controls reject it. It pins all staged
binaries and preserves original section bytes. The original runner uses the
same phase control and decodes independent core/API terminals and the exact
allocation map. Changed staged binaries or sources invalidate success.

The [first original menu/preferences/setup/shutdown journal](drawing-api-menu-observation-20261011.json)
records81,680core events/40,840paired calls and7,876API events/3,938paired calls
at27selected API sites. There is zero loss/open/IO, zero unassigned API phase or
unreadable stack/target, normal original exit0 and stable staged binaries/sources.
All three loaded instrumentation modules and the actual exported leave pointer
are recorded anew. None of this cohort's112physical core caller PCs belongs to
those prepared instrumentation ranges/returns; that fact does not override the
independent discovered original owner and unresolved caller accounting.

Actual API cleanup includes52bytes. This first wire's eight stack words are
explicit diagnostics and cannot supply all13arguments for such a call.
Formal argument/type/output snapshots, original helper/API semantics, resource
creation/ownership/borrow intervals and failure/recovery remain pending.
The initial phase2 screenshot is black at startup; the reviewed phase14 image
shows the original rendered Main menu. No frame/pixel equivalence is inferred.
All raw journals/maps, decoded projections, screenshots/logs and actually staged
binaries remain retained; only the stopped disposable game/prefix copies are
removed. All14scenario/four dispatch closures and64API admission refusals remain
open.

## Single-byte CALL observation, 2026-10-11

The reviewed static plan now admits 302 of the 314 selected CALL boundaries:
250 existing jump splices and 52 whole-CALL-verified, single-byte INT3 hooks.
Only the CALL's first byte is changed in the latter mode; operand and successor
bytes remain intact. Original branches into a CALL operand and overlapping
original hooks still refuse installation. The twelve remaining overlaps are
retained explicitly in `drawing-api-breakpoints-static-20261011.json`.

The process-private i386 exception handler changes only saved EIP for an exact
owned breakpoint/context pair. Foreign breakpoint and mismatched contexts return
CONTINUE_SEARCH. It does not restore opcodes temporarily, single-step, or issue
OS/graphics calls. This requires Wine/Windows XP+ vectored exceptions; it is an
observation policy rather than a recovered Win98 engine behavior. Dynamic unload,
exception-sensitive timing and arbitrary machine-context transparency remain
outside the validated scope. The actual callee return PC remains a recorded
trampoline address.

Thirty-eight fresh planner/journal/binding guards passed. The real PE32 fixture
passed twenty bodies, ten in each patch mode, with selected register, flags,
x87/SSE, LastError and 0/4/8/12/16/20/24-byte cleanup checks. It also executed an
original-style predecessor directly into the untouched successor, forwarded a
foreign INT3 to the next handler, refused mismatched exception contexts, and
round-tripped the actual native allocation map. These are synthetic native ABI
results, separate from original client behavior.

The original menu cohort `run-opctpvl0` closed 41,380 core pairs and 4,348 API
pairs at 32 sites, including 382 calls at five breakpoint sites. Both journals
have zero loss/open/IO failures; all targets and diagnostic stacks were readable,
phases assigned, sources/binaries stable, and original exit zero. The retained
Main menu screenshot was inspected. Twelve overlapping sites, complete formal
arguments and bounded API outputs, object ownership/borrowing, failure/recovery
routes, all fourteen scenario reviews and four dispatch obligations remain open.

## Complete selected API cohort, 2026-10-11

The two independently pinned plans now name their observation cohort. The
core/API plan retains 302 admitted sites and twelve overlapping refusals. The
separate API-only plan admits all 314 selected boundaries (262 jump splices and
52 single-byte breakpoints), with no core import or core call journal. Seven
prospectively declared native guards require exact boolean mode flags and actual
core module/leave presence or absence. Ordinary menu controls cannot implicitly
enable API hooks. The prepared core DLL remains a hash-checked file artifact;
it is not imported or executed in the API-only cohort.

Use `prepare-drawing-census.py --api-only` for the separate cohort, optionally
with `--movies`. The capture runner checks the plan/cohort agreement and the
actual startup module map, records `call_census_enabled: false`, and creates no
core-census result or inferred physical caller review. These API-only observations
must not be presented as a simultaneous core/API trace.

The original enabled-movie cohort `run-yyj3me1m` closed 4,511 API pairs at 53 sites,
including 19 movie-helper sites, with zero loss/open/IO/unreadable target or stack
rows, stable sources/binaries and original exit zero. The actual binding map has
null core module/leave. The setup/sample HRESULT-producing calls observed on both
startup requests returned zero; `46997a` Update returned `00040003` twice. The
four helper Release calls then ran. These returns supplement the earlier movie
client-contract fixtures and EOF checkpoints; they do not establish full movie
playback, output object values/lifetimes, successful Blt, failure recovery or
execution of every admitted site. The retained final Main screenshot was viewed.

Evidence: `drawing-api-only-movies-20261011.json`, separate static cohort plans,
and `drawing-api-cohort-guards-20261011.json`. The original CALL catalog, fourteen
scenario closures and four dispatch obligations remain independently open.
