# Selected native tinted glyph pixels

Reviewed 2026-10-10 for the No-CD executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
`RS.font-glyph-raster` now has a dedicated independent original comparison.
Confidence is high within the tested RGB565 pixel scope. Original live text
drawing remains active; this is an isolated pixel contract, not text takeover.

## Recovered pixel contract

The unmodified fastcall entry `0x581ec0` takes an indexed RLE glyph in ECX,
X in EDX, and Y/R/G/B on the stack. Signed origins adjust the destination anchor.
It rejects a glyph extending beyond either horizontal clip edge, including a
partially visible glyph; vertically it selects the surviving complete source
rows. Exact right/bottom contact uses half-open clip bounds. Transparent runs
leave destination words unchanged; opaque index zero still reads the destination.

For selected RGB565 mode zero, tint channels are divided by 8/4/8 before
interpolation. Each opaque source byte selects one of 64 initialized float
coverage values. Original x87 arithmetic blends each destination channel with
its quantized tint; `0x59bee0` converts by truncating toward zero. Native
`CanvasSequence::glyph` consumes the owned decoded plane/mask and supplied
coverage table. The private original fixture supplies those same source inputs
and a deterministic independently generated background. Original output never
initializes native rendering. The fixture does not modify any original file or
original raster body; only mapped input globals/storage are supplied.

## Native admission policy

`NR.font-glyph-admission` is separate from recovered failure behavior. It
requires indexed storage, matching plane/mask extents, byte tint, finite coverage
in exactly `[0,1]`, and opaque indices below 64 even in cropped-away rows.
Every visible destination sample must already be defined. All such checks
precede mutation: a late invalid index or undefined destination cannot leave a
partially painted glyph. Transparent samples do not read undefined storage.
This deliberate rejection policy is not an original exception/error contract.

The former implementation discovered indices/undefined pixels while drawing,
which allowed partial mutation; it also accepted coverage slightly greater than
one. Both are corrected without changing admitted pixel interpolation.

## Execution and reproduction

```sh
python3 tools/test-font-glyph-raster.py
```

The runner verifies the immutable original manifest before/after, pins the
executable hash and installed source inputs, captures actual repository compiler
dependencies and binary hashes, and verifies their source/input stability.
It writes new logs, corpus records, complete original/native canvases and a
machine-readable report beneath `working/tests/font-glyph-raster/`.
Host execution of its private original fixture requires i386 syscalls; a sandbox
which blocks those syscalls cannot execute the original comparison.

[The final result](font-glyph-raster-comparison-20261010.json) passes:

- 720 synthetic RLE fixtures: all 64 coverage indices across four run layouts,
  six tint choices, three unit-coverage tables and ten signed/edge/cropped/hidden
  placements, with ordinary and padded original rows.
- 3,615 placements of all 1,205 glyph records across all six installed SFTv3
  fonts: in-bounds, top-cropped and horizontally rejected.
- 4,335 complete original/native canvases, comprising 3,420,849 RGB565 WORDs,
  with zero pixel differences and unchanged original source/guard/padding bytes.
- Eight assertion-enabled atomic admission refusals, transparent undefined
  no-op and horizontal rejection, repeated under ASan/UBSan without findings.

The final run is `working/tests/font-glyph-raster/run-214rs5z5/`. Earlier runs
`run-u6p1t15a` and `run-zh3qpsf5` retain dependency-parser harness failures;
`run-4us7pmhj` retains the sandbox SIGSYS failure. `run-yqqgmcax` is the earlier
successful comparison before the final dependency/scenario binding and sanitizer
runner change; it remains exploratory, with its original source hashes intact.
None of the failed or earlier versions establish final-source validation.

## Remaining boundaries

This does not validate original SFT loading/palette conversion, text contour
advance/cursor state, code pages or higher-level layout, RGB555, coverage-table
initialization/lifecycle, arbitrary floating-point control state, caller-visible
returns/registers/flags, Win32 errors/reentry, general driver access, or live
glyph suppression. Next recover and independently compare the active text caller
state, then add strict original-active shadow admission before bypass.

Wider canvas producer, minimap, World and presentation evidence that fingerprints
the changed shared canvas source retains historical hashes and requires separate
current-source execution. This result renews only the two named glyph contracts.
The [committed history review](coverage/font-glyph-history-ae448ba-d412789-20261010.json)
checks 27 exact file transitions in `ae448ba..d412789`, with zero unresolved
receipt gaps; it asserts accounting, not a past gate pass or new execution.
