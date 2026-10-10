# Selected glyph backend caller state and coverage initialization

The independent state model matches 27,974 executions of the unchanged No-CD
entry `0x581ec0`. It covers integer returns and registers, rewritten arguments,
stack cleanup, defined return flags and exact cold coverage-table bits. It does
not emulate the original x87 effects or enable a live glyph bypass.

The build is pinned to SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence is high within the bounded comparison domain. The
[state model](../../reconstruction/rendering/glyph_backend_state.c) uses supplied
PE32 address identities and encoded input bytes; it never reads original output
as a prediction input. The separate native admission policy refuses malformed
inputs atomically instead of executing their unsafe original branches.

## Recovered caller contract

The entry receives the frame in ECX, X in EDX, and Y/R/G/B on the stack. It
preserves EBX/EBP/ESI/EDI and returns with `RET 16`. In RGB565 mode, it quantizes
and overwrites all three tint arguments even when geometry rejects the draw.
The Y argument becomes a source-row end and then, if an opaque run executes,
the final run counter, zero. AX-only early returns retain the upper EAX half.
The final `ADD ESP,40` defines arithmetic flags using the actual stack address;
DF remains unchanged.

Global `0x6f5eac` is compared with zero before geometry rejection. A zero sentinel
regenerates 64 coverage floats by multiplying integers 0..63 by the exact float
constant `0x3c820821` at `0x5c7860`, approximately 1/63. The entry leaves the
sentinel at zero. The integer model rounds this exact product to binary32 without
depending on the host floating environment. An ideal native division by 63 does
not describe the recovered operation.

The entirely empty cold frame returns EAX `0x006f0000`: the initializer's table
address survives above AX. Admitted pixel rows return zero. Selected zero-height
frames that reach the no-row branch return status 4. Malformed-row status 3 and
alternate bounds status 5 remain unvalidated and outside native admission.

## Execution and evidence

The final prospectively declared run is
`working/tests/glyph-backend-state/run-3qrghf1m/`. All 27,974 integer, argument,
coverage-table, callee, RET16, defined-flag, DF, source, padding, guard, XMM and
MXCSR comparisons passed. The corpus contains:

- 20,736 synthetic configurations, including padded and transparent RLE, both
  cold and initialized tables, empty dimensions, horizontal rejection, vertical
  cropping, hidden geometry, byte tints and nonzero clip origins.
- 7,230 placements of all 1,205 glyphs across six installed SFT fonts.
- Eight separately identified diagnostics with two incoming active x87 values.

All four rounding controls and 24/53/64-bit precision controls are exercised.
In the 27,966 cases with zero or one active finite x87 value, control words,
active values, tags and TOP are retained. These are original observations, not
native FP-state emulation. Sixteen atomic state/table refusals pass ASan/UBSan;
the freestanding PE32 model has no undefined runtime helpers. Native invalid
frame, index, clip, tint and address inputs are never sent to original code.

[Comparison](glyph-backend-state-comparison-20261010.json) and
[attempt history](glyph-backend-state-experiment-history-20261010.json) retain
source, case, result and artifact hashes, commands and immutable manifest checks
before and after execution. Earlier compiler, probe and auxiliary-preflight
failures remain unsuccessful results. The final sources were declared before
rerunning.

```sh
python3 tools/draft-coverage-claims.py --behavior RS.glyph-backend-state \
  --behavior NR.glyph-backend-state-admission \
  --scenario glyph-backend-state-host-20261010 \
  --output working/tests/glyph-state-claims-next.json
python3 tools/test-glyph-backend-state.py \
  --claims working/tests/glyph-state-claims-next.json
```

## Boundaries for the next glyph bypass

Cold table regeneration occurred in 8,625 cases whose geometry skipped pixel
rows. A bypass must preserve that preparation and the rewritten arguments even
when no pixels are drawn.

Original x87 status changed in 23,324 executions. Restoring the saved entry
FXSAVE image would lose those effects. Complete condition and sticky status,
x87 instruction/data pointers and exception behavior remain unimplemented.

Upward cold rounding generates coverage[63] bits `0x3f800001`, just above 1.0,
in 3,495 cases. The existing native unit-coverage policy refuses that table.
This finding does not relax its admission or renew historical glyph comparisons.

All eight selected draws with two active incoming x87 values cause masked stack
faults and change those values. Blending reaches seven temporary stack slots.
Future admission must check the required physical slots; a count alone does not
establish that those slots are free. Other stack shapes and unmasked faults remain
outside the comparison domain.

The existing producer assembly's handled tag10 branch also lacks `RET 16`.
Future glyph bypass must implement and test its epilogue, argument changes,
EAX/ECX/EDX, return flags and FP effects together. Original-active observation
continues to forward the entry and does not use that handled branch.

The [committed-history review](coverage/glyph-backend-state-history-0191a55-577d293-20261010.json)
checks 51 file and 103 behavior transitions in `0191a55..577d293`, with zero
unresolved receipt gaps. It establishes accounting, not a past gate pass or new
execution of earlier work.

Live glyph replacement, higher text layout/reset/lifecycle, RGB555, negative
tints, arbitrary FP state, Win32 errors and reentry remain open. Next implement
a guarded entry/epilogue and FP-effect composition, compare it independently,
and then enable a bounded producer bypass.
