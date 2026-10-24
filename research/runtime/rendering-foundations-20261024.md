# Rendering chunks 1–3: dispatch, displacement and alternate ownership

The two missing helper implementations now have current isolated original
comparisons. Alternate-canvas ownership remains unresolved after a stronger
initializer publication experiment and a renewed static candidate review.
These are rendering contracts, independent of task scheduling or AI recovery.

All original addresses refer to the No-CD image with SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The immutable manifest passed before and after each original-consuming run.
Confidence is high within the declared execution boundaries.

## 1. Minimap selector

`reconstruction/rendering/minimap_camera_dispatch.hpp` implements the exact raw
unsigned owner `+0xa4` dispatch at `555c3c..555c6e`. Values `0,1,2,3` select camera
`0,2,0,1`. Every other DWORD skips the camera call. The common creature-marker
call at `553850` runs afterward in either case. Camera 3 is not selected here;
this does not establish its global unreachability.

The comparison executes the unchanged original selector, jump table and CALL
instructions. Only the three camera callees, common marker endpoint and following
continuation have byte-checked adapters in a private mapped image. The fixture
uses independent owner and drawing-receiver objects and checks both remain
unchanged. All 8,232 selector cases compare ordered callback events, including
the four valid values, adjacent invalid values, high-bit DWORDs and negatives
interpreted as unsigned. Existing camera pixel kernels are not revalidated by
this callback-boundary experiment. Physical input-to-selector production and a
new native whole-minimap caller remain separate.

## 2. Displacement initialization

`reconstruction/rendering/displacement_table.h` independently generates all 16
owned DWORD offsets from the original approximate binary64 pi/8 constant at
`5c7858` and negative eight at `5c7850`. It executes x87 sine, multiplication and
the truncating integer conversion represented by `59bee0`, producing
`8 - trunc(-8 * sin(i * approximatePiOver8))`. No original table, captured pixels
or original executable address is a native input.

The comparison executes the complete unchanged `57dbd0` and `59bee0`. All 3,072
cases cover precision 24/53/64, all four rounding modes, all eight empty-stack
TOP values, eight selected exception-flag seeds and four condition-code seeds.
Every case compares all 16 entries and two guards, the control word, full status
word, abridged tag and MXCSR. FIP/FDP and bytes belonging to empty registers are
excluded. Five unsupported native requests refuse without output or contracted
FP-state mutation: null/short output, occupied stack, unmasked exceptions and
reserved precision. The FNINIT baseline remains
`8,11,13,15,15,15,13,11,8,5,3,1,1,1,3,5`.

Both helper comparisons pass AddressSanitizer and UBSan in PE32-sized execution;
independent native x86-64 sanitized controls also pass without an original image.
The initializer requires x86/x87. Other architectures explicitly refuse. It is
available as an owned native provider; the running original startup initializer
has not been hooked or replaced, and the existing raster bridge still observes
original startup state. Application/live integration and full pixel acceptance
are not promoted.

## 3. Alternate-canvas ownership discovery

The new hardware-watchpoint fixture runs all 288 complete original graphics
initializer cases with the existing owned mode/CreateSurface/COM endpoints.
Four-byte write watchpoints cover both `6e1f68` and `6e1f6c` throughout each
execution. They observe all 576 authored fixture stores, exactly 288 per slot,
and **zero original pointer stores**, including transient stores that a final
snapshot could miss. The original initializer, clear and format wrapper bodies
remain unchanged. All 576 complete primary/offscreen descriptors, callback order
and alternate-storage canaries still pass their existing comparison. These are
owned successful boundaries, not physical driver allocation or a natural game
startup. Tracer exit kills its child if observation fails.

`tools/alternate_canvas_lifecycle.py` byte-checks 542,955 retained/focused decoded
rows against the pinned image. It searches direct and bounded straight-line
constant-address stores, overlapping stores and address escapes. The only
candidates in the five-slot block are the known padding stores at `4e4163` and
`4e416d` and configuration store at `4e4c4e`. No base/source publication candidate
is found in this search. **54,067 unknown computed write instructions and all
2,948 unresolved indirect edges remain retained**, as do bulk/import/external
writer possibilities. This is a candidate search, not an exhaustive alias or
unreachability proof.

| Lifecycle stage | Established boundary | Remaining original recovery |
| --- | --- | --- |
| Pointer initialization | PE virtual-tail zero-fill of `6e1f68/6c` | No pixel allocation follows from loader zeros. |
| Allocation and publication | No publication in admitted `4e40d0` execution, even transiently | Allocator, allocating caller, pointer relationship and publication elsewhere. |
| Configuration | `6e1f78` controls `96/0` padding | Configuration does not establish base/source enablement. |
| Binding and drawing | Conditional `4fd438` base → `57dc20` → active canvas `658174` → World drawing `5002a0` | Actual published generation, bounds, clear and upstream writers. |
| Copy back | Ordinary surface Lock at `4fd4b5`, rebind at `4fd4d4`, then `594508` at `4fd4f0` reads `6e1f6c`; separate `59455c` source reader | Source/base relationship; incoming caller of `59455c`; full ownership. |
| Retirement | Normal-surface Unlock at `4fd5a7` is visible | Alternate storage release/reset, aliases and last writer. Unlock of ordinary storage is not alternate deallocation. |

Chunk 3 produces this discovery boundary and reproducible watch tooling. It does
not close alternate allocation, initial pixels, all writers or retirement. Chunk
4 must remain dependent on those findings; no invented native allocator or
padding-derived ownership policy is introduced. Live observation outside the
initializer remains necessary to identify a natural publication or writer.

## Evidence and reproduction

The final SQLite-backed record is
[execution03.json](rendering-foundations-results-20261024/execution03.json).
`execution01.json` preserves the successful earlier helper/watch execution before
the final native64 and tracer-cleanup changes. `execution02.json` preserves the
subsequent successful execution before exact original helper/data links were
added to the prospective contract. Both historical fingerprints remain intact.
Raw binaries and logs are pinned under the runner's `working/tests` output.

```sh
python3 /home/hoff/swdev/magic_and_mayhem_mod/tools/working-retention.py run \
  --reserve-gib 0.12 -- python3 tools/test-rendering-foundations.py \
  --output "$PWD/working/tests/rendering-foundations-next" \
  --report research/runtime/rendering-foundations-results-next/execution.json \
  --frontier /home/hoff/swdev/magic_and_mayhem_mod/working/tests/critical-all-original-20261020/flow-4/report.json
```

Run from the owned checkout; restore the pinned frontier first if it is archived.
The frontier SHA-256 is checked against retained ACR1 provenance. Use an isolated
temporary filesystem if host compiler temporary-file quotas require it. The
runner declares all behavior/source identities prospectively, verifies complete
compiler dependency closure, retains command logs and records failures through
the SQLite writer. No installation or Wine prefix is created.

Implementation and isolated-original comparison become scoped/recorded only for
the two helpers. Integration and live replacement stay `none`; alternate
provenance stays partial. The old canvas-initializer comparison is renewed at its
unchanged successful callback boundary. Broader rendering acceptance, FPS,
physical DirectDraw and historical NT17 evidence retain their existing limits.
