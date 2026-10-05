# Effect trajectory stepping

Build: no-CD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

`004df500` is a complete, call-free trajectory stepping routine. Its thiscall
receiver owns 14 DWORDs (0x38 bytes); four arguments point to X, Y, Z and a
change counter. It returns with `ret 0x10`. The native contract owns the state,
three coordinate words and counter separately; it assumes no aliasing among
them. This is stepping from authored state, not recovery of trajectory setup.

All additions and subtractions wrap at 32 bits. Branch comparisons interpret
the wrapped error as signed: strictly positive means `1..0x7fffffff`.

| State word | Observed stepping role |
| --- | --- |
| 0 / 1 | Primary / alternate Z delta |
| 2 | Allow XY when the primary Z branch is selected |
| 3 | Last selected Z branch, overwritten with 1 or 0 |
| 4 / 5 | Alternate X / Y delta |
| 6 / 7 | Primary X / Y delta |
| 8 / 9 / 10 | XY error addition / subtraction / accumulator |
| 11 / 12 / 13 | Z error accumulator / subtraction / addition |

First subtract word 12 from word 11. If the result is positive, add word 0
to Z and write word 3 = 1. Otherwise add word 13 to the error, add word 1
to Z and write word 3 = 0. Increase the counter by one only when the selected
Z delta is nonzero. Skip the entire XY update when word 3 is 1 and word 2
is zero; the XY error remains unchanged in that case.

For an XY update, subtract word 9 from word 10. A positive result selects
words 6/7 and increases the counter by two. Otherwise add word 8 to the
error, select words 4/5 and increase the counter by three. These counter
increments are unconditional, including zero XY deltas. They should not be
interpreted as a count of coordinates that changed.

The movement routine's entry is **004883f0**. Previously documented
`00488540` is an internal iteration block, not a function entry. The call at
`00488587` uses trajectory state at effect record `+0x128`, the first three
parameters at `+0x2c/+0x30/+0x34`, and counter `+0x1ce`. The subsequent
parent path checks vertical bounds, wraps horizontal fine units, produces
recount coordinates at `+0x1c2/+0x1c6/+0x1ca`, updates cell membership and
handles collisions and type-dependent termination. These are static findings;
they have not been compared or implemented by this chunk.

The [comparison record](effect-trajectory.json) fingerprints the model, tests,
runner and standalone CMake target. The helper privately maps the exact
executable, checks the entry bytes, and executes the whole unmodified leaf
without callbacks, stubs or patches. It compares every trajectory word,
coordinate and counter after every step, checks allocation guards, and compares
the output stream with 64-bit native and ASan/UBSan builds. Fixtures cover signed
error boundaries, zero deltas, XY suppression, counter/coordinate overflow and
32-step continuations. Unit tests additionally check explicit branch expectations.
The recorded run covers 8,192 fixtures and 262,144 steps: 156,944 primary and
105,200 alternate Z selections, 78,398 suppressed XY updates, and 109,554
primary / 74,192 alternate XY updates. All three output streams match.
Strict standalone CMake/CTest and normal/sanitized unit tests pass. The original
manifest verifies all 2,927 files before and after execution.

```sh
python3 tools/test-effect-trajectory.py
```

Confidence: high within the whole leaf's disjoint-state stepping contract.
Neither arbitrary authored words nor equivalence of this leaf demonstrates
that installed effect types initialize those words correctly. `004df3a0`,
the XY-only `004df5d0`, the larger movement parent, cell changes, recount
production, removal/recycling and live scheduling remain separate work.
Created effects still cannot be passed to lighting with pending recount
coordinates. The next bounded integration should recover parent projection and
recount production before admitting moving sources to the lighting cycle.
