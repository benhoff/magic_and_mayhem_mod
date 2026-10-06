# Effect trajectory initialization

Build: no-CD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Recovered contract

Whole thiscall `004df3a0` (`ret 0x20`) initializes the owned 14-DWORD trajectory
receiver used by [004df500 stepping](effect-trajectory.md). Arguments are source
X/Y/Z, destination X/Y/Z, a DWORD change-counter pointer and a DWORD multiplier.
It reads period globals `006c5494` (X) and `006c5498` (Y), multiplies each by the
multiplier with 32-bit wrapping, and calls the original XY initializer
`004df260` at `004df46a`. The native interface accepts those two globals as
owned raw words. State, coordinate inputs and counter must be disjoint.

All arithmetic wraps at 32 bits. Comparisons interpret the words as signed;
absolute value of `80000000` remains `80000000`, including its signed ordering.
For each horizontal axis, let D be the wrapped absolute source-minus-original-
destination difference, and P the wrapped period product. If D is signed-greater
than abs(source + P - destination), subtract P from the selected destination.
Independently, if D is signed-greater than abs(source - P - destination), add P.
Both comparisons use the original destination and D. Strict comparisons preserve
the destination on ties; this is not a modulo-normalization policy.

`004df260` clears words 4..7. Its X/Y direction is +1 for destination >= source,
including equality, otherwise -1; span subtraction wraps after signed endpoint
comparison. If X span >= Y span, word6 receives X direction and word7 stays zero;
otherwise word7 receives Y direction and word6 stays zero. Word8 receives twice
the signed-larger span, word9 twice the other, and word10 the larger undoubled
span. Counter is reset to zero.

The parent clears words 0..2, derives the horizontal span using wrapped absolute
differences to the selected destination, and selects Z direction by signed
endpoint comparison. Word1 receives that direction even for equal Z endpoints.
When Z span >= horizontal span, word0 also receives it and word2 stays zero;
otherwise word0 stays zero and word2 becomes one. Word13 is twice the larger span,
word12 twice the smaller, and word11 the larger undoubled span. It resets the
counter again. **Word3 is preserved**; only stepping subsequently overwrites it.
The native contract therefore initializes state in place rather than asserting
that this routine constructs all 14 words.

## Evidence and boundaries

[TL19 comparison](effect-trajectory-initializer.json) records 18,112 authored
initializations, every complete receiver/counter and allocation guards, plus
32 steps per fixture (579,584 steps). The helper privately maps the exact PE32,
checks initializer/helper entry bytes and calls whole unmodified `004df3a0`,
its original `004df260`, and original `004df500`. No callbacks, patches or helper
stubs are substituted. Initial contents are randomized, including retained word3.
Fixtures cover coordinate equality, signed endpoints, wrap choices, half-period
ties, zero/asymmetric/wrapped period products and arithmetic overflow. Full raw
random words supplement the explicit endpoint matrix. Original, native64 and
ASan/UBSan output streams match. Strict normal/sanitized units and standalone
CMake/CTest pass; immutable manifests verify all 2,927 files before and after.

```sh
python3 tools/test-effect-trajectory-initializer.py
```

Confidence: high within this disjoint owned initializer and chained-step contract.
Period/coordinate provenance remains authored. Static ballistic-update caller
`004891e0`, within `00489080`, uses
receiver effect+128, source parameters +2c/+30/+34, destination +38/+3c/+40,
counter +1ce, multiplier 32. It first calls `004df230` to zero the whole state
at `004891b8`, then invokes movement `004883f0` at `004891e9`. This caller mapping
is static evidence, not execution of the ballistic parent or an installed-input
capture. The whole motion creation helper `0048a950` is now recovered separately in
[effect motion initialization](effect-motion-initializer.md). Other callers,
creation policy/ownership, live scheduling and replacement
remain separate. Historical stepping evidence remains unchanged; the initializer
is an additional source module and does not rewrite the stepping implementation.
