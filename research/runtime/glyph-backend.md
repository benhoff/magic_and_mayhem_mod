# Guarded native glyph kernel and handled entry

The next bounded step after [caller-state recovery](glyph-backend-state.md)
implements native RLE traversal and x87 blending, an explicit owned-context
route, and a handled entry with RET16. This adapter is not installed in the game.
Process-memory discovery, read/write admission and context lifetime remain open.

## Contract and admission

The No-CD build is pinned to SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The original glyph entry is `0x581ec0` (first seven bytes
`83 ec 28 8b 44 24 30`); the truncation helper is `0x59bee0`
(`55 8b ec 83 c4 f4`). Native code calls neither original function on its
handled path. Original geometry and caller results use the prior independent
integer model; the new assembly handles floating-point effects.

Native admission requires owned distinct frame, canvas, table and aligned
FXSAVE buffers, bounded complete RLE including hidden rows, byte RGB tint,
RGB565, a valid half-open canvas clip and supported masked x87 precision.
All seven physical push slots below TOP must be empty; a remaining active
value must be finite and canonical. An occupied-register count alone is
insufficient. Reserved MXCSR bits and pending unmasked status are refused.
Warm coverage must have binary32 bits in `0..0x3f800000` (positive zero through
one). Cold coverage uses
the recovered x87 multiplication/store sequence under the caller's controls,
including the upward endpoint `0x3f800001`. This cold endpoint is recovered
behavior, distinct from the stricter intentional warm-table policy.

Preflight refusal preserves destination pixels, table, output FX image and
integer result. The entry forwards a refused request exactly once with its
incoming registers, arguments, flags and FP state. No unsafe malformed request
is submitted to original code. The route defaults to forwarding unless an
embedding adapter supplies a single-threaded owned context matching the frame.

## Floating-point implementation

The kernel wrapper saves the host environment, restores the admitted caller
image, executes integer traversal and explicit x87 helpers, captures the result
and restores the host environment. Each opaque run loads three quantized tint
channels. Per pixel, coverage and its inverse feed the recovered channel
multiply/add ordering and a native truncation helper temporarily selecting
round-toward-zero for FISTP. Run grouping and final pops preserve the original
status evolution. The route writes the selected caller results into its saved
entry frame and restores the resulting FX image before RET16.

The comparison includes x87 control/status/TOP/abridged tag and occupied logical
register values, plus XMM/MXCSR and masks. Instruction/data pointer history,
opcode history, empty register contents, arbitrary FP values, unmasked exceptions,
other pixel formats and complete Win32 ABI equivalence remain excluded.

## Validation

The [current comparison](glyph-backend-comparison-20261010.json), declared before
execution, records **28,142 complete matches / 22,656,678 canvas WORDs**:

- 27,966 native handled calls; the original glyph body is never called for them.
- Eight two-active-value original-once fallbacks, retaining the original masked
  stack-fault effects rather than attempting native arithmetic.
- 168 targeted original-once fallbacks: 48 absent contexts, 48 mismatched owned
  frames, 48 finite IE-unmasked draws and 24 finite warm-aboveone tables.
- 24 atomic admission refusals under ASan/UBSan. PE32 relocatable linkage has
  only the embedding original-trampoline symbol unresolved, with no CRT helper.

The 128 context forward-counter increments exclude the 48 absent-context calls;
the instrumented original body counts all 176. All twelve precision/rounding
control combinations, selected sticky/C1 seeds, empty and one-active finite
caller stacks, complete horizontal rejection, vertical crops, source/padding
guards and table preparation before geometric skip pass. Confidence is high
within this explicitly admitted scope.

The corpus is the retained `MNMGLS01` input and case index from the independently
generated [caller-state comparison](glyph-backend-state-comparison-20261010.json):
20,736 synthetic configurations, 7,230 installed-font cases covering all 1,205
glyphs in six SFTs, and eight two-active diagnostics. New reports fingerprint
both inputs, the generator and complete compiler/source closure. Immutable input
manifests pass before and after each experiment. The
[experiment history](glyph-backend-experiment-history-20261010.json) retains all
failed and intermediate successful versions. These include removal of a PE32
memcpy import, correction of C call-site alignment and a forced-fallback harness
destination-reset fix. Earlier results retain their original source hashes.

Reproduce with fresh prospective claims, passing the corpus/index paths from a
caller-state run (generate them with `tools/test-glyph-backend-state.py` if needed):

```sh
python3 tools/draft-coverage-claims.py --behavior RS.glyph-backend \
  --behavior NR.glyph-backend-admission --scenario glyph-backend-host-20261010 \
  --output working/tests/glyph-backend-next-claims.json
python3 tools/test-glyph-backend.py --claims working/tests/glyph-backend-next-claims.json \
  --corpus working/tests/glyph-backend-state/<run>/cases.bin \
  --index working/tests/glyph-backend-state/<run>/cases.json
```

Committed-history review `577d293..eb3cd18` accounts for 18 exact file and 28
behavior transitions with zero unresolved receipts. This is separate from new
execution validation. No live replacement milestone follows from this headless
comparison. Next work is a process-memory and lifetime guard, then an explicit
bounded live glyph route with diversified captures and independent original
replay. Higher text layout and font reset/lifecycle remain separate work.
