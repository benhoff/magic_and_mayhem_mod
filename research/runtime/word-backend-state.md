# Selected direct-word backend state and ABI

This increment recovers the workspace and caller-visible state of original
No-CD backends `0x596cb8` and `0x597086`. Pixel-only clipping equivalence does
not establish these effects. The reconstruction predicts sixteen workspace words
and argument-slot mutation from encoded inputs and initial workspace; original
execution supplies an independent comparison.

The isolated i386 probe seeds registers, EFLAGS, masked x87 control/status and
three active values, XMM0..7 and MXCSR. It records original return state before
restoring the host ABI. Original bytes remain unchanged. Defined XOR-zero flags
and preserved DF are checked; undefined AF, x87 instruction/data pointers/opcode
and unused x87 registers are excluded. Unmasked exceptions, other backends,
unsafe original input behavior, concurrency and live entry-hook ABI remain open.

Fixtures cover both selected backends, contiguous direct-word frames, signed
origins, exact/partial edges, four corners, hidden sprites, zero/nonzero left/top
clip globals, oversized frames and empty dimensions. The actual native workspace
helper is checked only under the existing strict in-bounds admission.

Reproduce with a prospective declaration:

```sh
python3 tools/draft-coverage-claims.py --behavior RS.word-backend-state --output working/tests/word-state-claims-new.json
python3 tools/test-word-backend-state.py --claims working/tests/word-state-claims-new.json
```

Model input validation is an intentional safety policy. It is not recovered
malformed-input acceptance. Clip coordinates must define a nonempty viewport
within a bounded positive canvas; translated anchors lie in [-4096,4096].
Auxiliary planes and indexed input are excluded from the positive-frame model.
No clipped live admission or rendering replacement is introduced.

The host reference also links the production entry assembly into an
isolated test with a host-only stand-in for Win32 access admission and logging.
That stand-in uses the real native raster core and workspace helper, retains
strict native bounds and forwards other requests to the matching original body.
Production assembly register/flag/floating restoration can therefore be compared
without claiming that this tests Win32 access, LastError, busy/reentry or a live
process hook. The handled branch clears saved x87 C1 for admitted positive frames.

## Confirmed effects and corrections — 2026-10-10

The [final comparison](word-backend-state-final-comparison-20261010.json) covers
4,448 original/model and production-entry cases with no mismatches. All sixteen
workspace words, argument slots, integer registers, stack guards, defined return
flags, selected floating state, source bytes and complete canvas guards agree.
The production-entry test includes 768 native admissions and 3,680 original
forwards. The real native workspace helper matches every admitted case.

| Workspace index | Recovered meaning / write boundary |
| --- | --- |
| 0 / 1 | First source row / exclusive source row limit after vertical clipping. |
| 2 / 5 | `2*(stride-frame_width)` / frame width. |
| 3 / 4 | Retained incoming words. |
| 6 | First destination row offset in WORDs. |
| 7 | Frame-row pair address: frame+40+8*first_source_row. |
| 8 | Remaining rows; zero after drawing, retained when no rows execute. |
| 9 | Forward backend's zero row remainder; scalar backend's last-row trailing transparent count, including full width when that row has no opaque run. |
| 10 | Frame identity, written even for empty dimensions. |
| 11 | Scalar backend's last copied opaque run after destination alignment adjustment; otherwise retained. |
| 12 / 13 | Horizontal source start/end limits, written only when the horizontal clipping branch is selected. |
| 14 | Horizontal cursor; forward stores final skips, scalar's final transparent skip does not write it. |
| 15 | Last opaque iteration's right-clipped tail; reset by opaque processing and retained if no opaque iteration executes. |

Both backends return EAX=0, preserve the other integer registers and ESP, and
retain DF. Positive dimensions translate the caller's X/Y stack arguments by
signed origins, including hidden sprites; empty dimensions retain both arguments
and write only workspace10. Exact right edges select horizontal clipping even
when no pixel is excluded. No-visible-row cases still perform workspace setup.

Original scalar instructions at `0x5971c6..0x597243` and
`0x5973a4..0x5974ab` bypass the remainder store on a final transparent run. The
old adapter's unconditional workspace9=0 was therefore wrong for 120 admitted
requests in this corpus. `word_workspace.h` now derives the correct last-row
remainder from validated native draw metadata. Other workspace writes retain
their original scope; native admission is unchanged.

Original FILD/FIMUL/FISTP of the bounded exact row/stride product clears C1 for
positive dimensions, preserves the tested masked sticky status, control and
active values, and leaves empty frames' floating state unchanged. The prior
handled entry restored incoming C1. Production entry assembly now clears C1 in
its saved FXSAVE status before restoring admitted positive requests. Forwarded
requests retain original handling. Four masked rounding/status/EFLAGS seeds and
three finite active x87 values are tested; unmasked exceptions, full x87 stacks,
NaN/denormal/infinite inputs and floating history remain outside this proof.

Fresh [live shadow](word-route-state-fix-shadow-20261010.json) records 12,625
admitted pixel/workspace comparisons, 943 forwards and zero mismatches/errors.
Fresh [takeover](word-route-state-fix-takeover-20261010.json) records 12,608
admitted body bypasses and 705 forwards. Eight complete canvases / 1,070,400
pixels and workspace match independent execution of the unchanged original.
These are finite startup checks of the existing strict unclipped route; the
host probe's selected floating checks remain separate from live Win32 exception,
LastError/reentry, shutdown and sustained-session validation.

The [private fixture/result format](../formats/word-backend-state-fixtures-v1.md)
documents probe seeds, records and excluded fields. Earlier compile/dependency
parser failures, the sandbox's i386 syscall refusal, and two missing-installation
launcher preflights remain under `working/tests/` and `working/experiments/`.
The earlier successful model/entry reports keep their original declarations.
The final fixture-domain declaration precedes its rerun; no historical hashes
were refreshed. Tests use commit `01e160a` plus this increment.

Next implement a preflighted native CPU clipping path using these recovered
workspace effects, then compare it independently before live admission. Complete
Win32 admission/LastError/reentry and floating exception behavior need separate
contracts; this increment does not enable clipped live drawing.
