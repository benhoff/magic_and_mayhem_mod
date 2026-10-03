# Creature movement scalar: 0x005205b0

Build: no-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Evidence and confidence

Assembly: `working/decompiled/search-support-ywr0z18h/creature_scalar.asm`,
`base_scalar.asm`, `adjust_scalar.asm`, `scaled_metric.asm`, with hash manifest.
Read-only exporter `tools/export-search-support.py` now reproduces all four ranges.
Implementation: `reconstruction/pathfinding/route_scalar.*`.
Confidence is high for operations and ABI: 149,070 isolated original x86 comparisons
match both output DWORDs with the entire scalar chain unchanged. Gameplay names
such as speed, haste, friction or gravity remain hypotheses; the API retains
source-offset names. Nothing is patched in an installed executable or running game.

## Wrapper and inputs

`0x5205b0..0x52061f` is thiscall on the creature, with seven stack arguments:
argument, category, delta X, delta Y, delta Z, prior/scalar pointer, base pointer.
It returns with `ret 0x1c`. The generator supplies argument 4 via `0x4eb030`.
The prior/scalar pointer is genuinely in/out: direction and vertical-turn rules
in the generator determine whether the previous neighbor payload scalar is reused.
The second output is the unmodified result of `0x505840`; the generator discards it.

The wrapper first calls `0x505840` on creature DWORD `+0xac` with category and
three deltas. It writes the base output, doubles the DWORD if creature `+0x778`
is signed-positive, then halves it with signed truncation toward zero if creature
`+0x77c` is signed-positive. Order matters, including DWORD overflow. It rereads
creature `+0xac` before calling `0x505920` with all inputs and the adjusted base.
Frozen snapshot replay preserves the two pointer sources independently but does
not emulate live mutations between these reads.

## Base table calculation: 0x505840

- Categories 1, 2, 3 with type DWORD `+0x3c == 0` immediately return the unaligned
  global DWORD `0x6c007d`. No metric or table lookup occurs on that branch.
- For nonzero XY motion, `0x4eade0(dx,dy) & 1` selects a bank. XY must fit its
  recovered 3x3 direction table. Pure vertical movement substitutes X = delta Z,
  Y = 0, Z = 0 for this stage's metric, and uses bank parity zero.
- Categories 0, 3, 4 sum twelve DWORDs starting at `+0xd8 + parity*0x30`.
  Categories 1, 2 sum twelve starting at `+0x138 + parity*0x30`.
  Other categories leave the sum zero (but still perform the metric).
- `0x4eaca0` doubles X/Y and calls the existing weighted metric `0x4eac10`.
  Multiply metric by sum with DWORD wrap; logically shift right two; multiply
  by twelve with DWORD wrap; return unsigned division by twelve via the
  original multiply-high constant `0xaaaaaaab` and shift three.

Do not cancel the multiply/divide by twelve: overflow makes that incorrect.

## Prior scalar adjustment: 0x505920

Thiscall receiver is the type record, eight stack arguments, `ret 0x20`.

First set target to unsigned `(adjusted_base * 4) / argument`, wrapping the
numerator. Except category 4, delta Z = +1 scales target by signed
`wrap(target*2)/3`, delta Z = -1 by signed `wrap(target*3)/2`.
Other Z values do not apply these adjustments.

Start with type signed DWORD `+0x10`. Negate with DWORD wrap when target is
signed-less than prior. Except category 4, delta Z = +1 subtracts and delta Z = -1
adds `float[0x5e15f0] * float[0x5c7260]` using x87 arithmetic, then truncates.
The second float is file-backed `76.8000030517578125`; the first is mutable and
must be captured. Do not assume its file value `0.968999981880188` persists.

Let distance = logical-right-shift-two of the wrapped DWORD product
`metric(2*dx,2*dy,dz) * 9 * 256`. Let discriminant be signed interpretation of
`wrap(prior*prior + distance*acceleration*4)`.

For nonnegative acceleration, the next value is
`(sqrt(discriminant) + signed(prior))*0.5`, capped above by signed target.
Negative discriminants create NaN and follow the x87 unordered comparison path.
For negative acceleration, clamp discriminant to zero before the square root,
then floor the next value at signed target. Convert with original `__ftol`
semantics (signed QWORD truncation, low DWORD result, low DWORD zero on masked
invalid conversion). The target local is not the wrapper's base output.

## Reproduce and limits

```bash
python3 tests/test-native-movement.py --component scalar
```

119,070 branch fixtures plus 30,000 deterministic randomized DWORD/table/overflow
fixtures. Covers all adjacent deltas, seven categories including invalid ones,
positive/nonpositive modifier states, varied prior values, argument divisors
1..4 and positive/negative/zero slope globals. All object/type buffers remain
unchanged. Linux i386 execution may require running outside the syscall sandbox.
The private PE loader receives a hash-checked temporary snapshot and redirects
no helper instructions in this component; metric, direction and conversion
routines also run as original code. Host tests and ASan/UBSan checks pass.

Division by zero and out-of-table XY deltas are explicit host errors rather
than executing native faults or invalid memory reads. This is an algorithm
reconstruction, not a change to creature balance. End-to-end installed search
agreement still needs a fresh real world capture; see the world replay protocol.
