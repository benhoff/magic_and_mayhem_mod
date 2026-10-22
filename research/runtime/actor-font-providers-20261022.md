# Font provider and ownership recovery (2026-10-22)

## Scope and confidence

Seventeen original providers extend the actor composition from198 to215 public
routes over one owned store. The selected actor text caller4d9f30 now executes
the recovered raw loader, rectangle layout and glyph raster bodies together.
Confidence is high within the bounded normal-return cases described below.
The Critical actor-service umbrella remains partial. Actual platform/file/heap
backends, installed owner/planner cadence and live replacement are separate.

The pinned NoCD Chaos.exe SHA-256 is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Fresh Ghidra decompilation, direct instruction/ABI inspection, call references
and source inputs are retained under `font-provider-results-20261022/` as SQLite
references. Every original-consuming experiment verifies the immutable manifest
before and after. Six independently retained installed SFT inputs drive both
executions; original output never becomes native input.

## Recovered entries

| Entry | Scope |
| --- | --- |
| 4a58c0 | Walk and free the owned palette chain, preserving next-before-free order. |
| 4a58e0 | Byte contour advance, punctuation remapping and optional trailing-state update. |
| 4a59e0 | Byte draw, cursor/trailing state and tinted glyph dispatch. |
| 4a5ae0 | Raw SFT read, version2/3 pointers, conversion and publication, normal failures. |
| 4a5ce0 | Rectangle text layout, line/source records, wrapping and glyph drawing. |
| 4a6190 | Height-limited text layout and consumed-source publication. |
| 4a6630 | Text measurement, line records and cursor/trailing side effects. |
| 4a6970 | Header ascent plus descent. |
| 581ec0 | Indexed RLE tinted software glyph, RGB565/555 and original coverage table. |
| 4be870 / 4be8c0 | Heading font global setup and teardown at6b0400. |
| 4be910 / 4be960 | Body font global setup and teardown at656680. |
| 4be9b0 / 4bea00 | Tooltip font global setup and teardown at6df1c0. |
| 4bea50 / 4beaa0 | Yellow font global setup and teardown at6a5138. |

The byte and line adapters reuse the existing recovered `font_byte` and
`font_layout` algorithms. Coverage initialization reuses `glyph_backend_state`.
Their historical proofs remain unchanged. The new value-address raster adapter
compares ordinary masked, nearest-rounding extended FP against581ec0; it does
not extend the separate i386 glyph backend's ABI, control-word or fault claims.
The native strict-v3 asset loader and Qt menu font policy are unchanged.

## Ownership and compatibility findings

The raw reader has intentionally literal original behavior. Failed open or
zero length preserves the object; failed allocation publishes a null raw pointer.
A short read retains the allocated raw buffer and earlier pointer fields.
Declared-size mismatch frees that raw buffer and the existing palette chain,
clears fields0/8/12/16/20, but leaves the borrowed palette field+4 untouched.
The magic is not checked. Unknown versions retain existing derived pointers,
then update line height and return success. Version2 writes zero at raw+36,
overwriting its first palette or contour bytes. No-palette versions derive the
table by adding to the object's prior+12 value; those cases compare state only,
without drawing through an invalid fresh pointer.

Palette conversion calls the actual582440 builder and publishes its returned
head into every glyph, even if allocation fails. Reload overwrites the old raw
pointer without freeing the previous allocation. Old palette heads may likewise
become unreachable on a successful replacement. The comparison observes these
retained allocations before explicit fixture-owned residual cleanup; zero final
live allocations does not imply that original reload itself is leak-free.

Global setup clears only the original fields, sets tab advance20, tracking1 and
field234=1, and registers the matching destructor through a named atexit supplier.
It preserves borrowed/stale fields and contour/cursor storage. Teardown frees
raw storage first, then the palette chain, then clears owned/derived fields;
borrowed+4 remains stale. Callback-time object bytes and free order are compared.

## Discovery boundaries

The four setup/destructor pairs were absent from the pinned central function
inventory. A fresh read-only discovery export recovers their exact eight ranges;
execution entry-byte anchors support eight explicit recovered-range exceptions.
The full fresh inventory is retained, including the unrelated55a650 candidate,
but only these eight reviewed ranges are adopted as exceptions. Unknown ranges,
indirect targets and the existing inventory hash remain explicit. 5575b0 font
load callsites are static caller evidence, not recovery of whole-game startup.
The adjacent4a5850/4a5860 widget helpers are outside this font family.

## Evidence and remaining work

The full `font-matrix-3.json` comparison passes1152 cases and2304 checkpoints
across18 roots/callers times64 variants. Each stream has1428014200 bytes, SHA-256
`d8d5524428716ed4069f865dc694bd3a3315e369f67c79ff734c2452ce6a3710`. Each case compares all owned object/global/canvas bytes, initialized
padding, allocation and freed-buffer contents, ordered file/heap/COM callbacks,
meaningful returns and two before/after-teardown snapshots. Original selected
bodies remain byte guarded. Normal original32, native64 and ASan/UBSan builds
must produce identical streams and return with zero residual live allocations.
All earlier failed or limited trials retain prospective exact source text.

The static reconciliation retains43 historical leads as24 concrete routes and
19 embedded helpers. The previously supplied4a5ce0 specification becomes
concrete; atexit registration and owned COM unmap add two named boundaries,
leaving114 supplied endpoint specifications. This count is not a count of
missing original functions. Full indirect target sets remain incomplete.

Remaining actor recovery includes owner/planner and task cadence, support and
postprocess providers, particle/special subtypes, scenario evaluators/actions,
actual file/profile/allocator/platform providers and unresolved indirect targets.
For fonts, arbitrary malformed traversal, unusual FP states, original long-word
unsafe branches, throwing/SEH cleanup, other full callers and actual surface/
font installation transitions remain outside this bounded proof. Installed
sessions, physical driver acceptance and live replacement remain unvalidated.
