# Neighbor expansion: assembly review

2026-10-02. Review of `0x004ebae0`, before implementing its movement providers.
No executable changes or live capture were performed.

## Evidence and reproduction

Exact no-CD executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
All addresses below are preferred VAs; relocate against the loaded image for
live work. Evidence is the pinned exports
[assembly](decompiled/nocd/assembly/expand_neighbors.asm),
[inferred C](decompiled/nocd/raw/004ebae0.c), and the
[search caller](decompiled/nocd/assembly/route_search.asm).

Run `./tools/verify-decompilation-baseline.py --executable working/game-nocd/Chaos.exe`.
This review verified all 14 baseline artifacts and 1,686 instruction byte chunks
against that working executable. Original media was not consumed.

Confidence is **high, static** for instruction-level findings below. Names such
as direction, category, and movement scalar describe their use; gameplay
interpretations and helper semantics remain unconfirmed without helper analysis
and controlled runtime evidence. The inferred C is not buildable source.

## Function boundary

Use the following conceptual signature, not a host-callable engine ABI:

```cpp
void expand_neighbors(
    CellToken current,              // ECX at entry
    CandidateVector* output,        // entry ESP + 0x04
    const Descriptor* descriptor,   // entry ESP + 0x08
    const MovementPayload* prior,   // entry ESP + 0x0c
    int32_t* remaining_budget);     // entry ESP + 0x10
```

The callee saves ECX to EBX at `0x4ebaee`; after its local frame and four saved
registers, these arguments are at ESP `+0xa0`, `+0xa4`, `+0xa8`, `+0xac`.
The initial budget read occurs before the register saves at ESP `+0x9c`.
`ret 0x10` at `0x4ec776` removes four stack arguments. The caller at
`0x54ba45..0x54ba6b` supplies the candidate vector, a stack descriptor, the
current node record's payload (`record +8`), and the budget pointer.
Thus Ghidra's `param_1` is the implicit current-cell receiver; `param_3` is a
descriptor, not the creature object itself. Its `+4` field supplies the object
receiver on the creature path.

## Control-flow map

| Block | Confirmed behavior |
| --- | --- |
| `0x4ebaf1..0x4ebafd` | Return without expansion or decrement when budget is signed `<= 0` |
| `0x4ebb03..0x4ebb8e` | Decode a cell token into XYZ using a 12-byte cell stride, dimension/offset globals, and XY normalization helpers |
| `0x4ebb92..0x4ebb95` | Descriptor DWORD `+0x3a == 0` selects the standard branch; nonzero selects six links |
| `0x4ebb9b..0x4ebd02` | Six-link branch, ending at the shared budget decrement |
| `0x4ebd07..0x4ebf0f` | Standard-branch setup, with additional restrictions when descriptor `+0 != 0` |
| `0x4ebf1b..0x4ec19a` | Enumerate each offset and run restrictions/acceptance helper |
| `0x4ec19f..0x4ec243` | Rejected helper result skips candidate; descriptor `+0 != 0` takes the simpler cost/payload path |
| `0x4ec248..0x4ec4d0` | Descriptor `+0 == 0` computes direction, scalar, cost multipliers, and accumulated float |
| `0x4ec4d4..0x4ec74d` | Vector allocation/copy/append machinery for a 36-byte candidate |
| `0x4ec751..0x4ec75d` | Advance standard offset index; repeat exactly 26 times |
| `0x4ec763..0x4ec76a` | Decrement budget once for the entire expansion, including an expansion producing no candidates |

Do not recreate allocator mechanics in the host model: preserve append order
and candidate bytes with `std::vector<Candidate>`.

## Candidate and prior-payload fields

The temporary candidate begins at frame ESP `+0x3c`. Nine DWORDs are copied
into the output; stride is `0x24`. The prior argument points to its payload
equivalent, so prior offsets are candidate offsets minus eight.

| Candidate offset | Prior offset | Observed use |
| --- | --- | --- |
| `+0x00` | — | Edge cost |
| `+0x04` | — | Destination cell token |
| `+0x08` | `+0x00` | Category, initially 5; standard acceptance helper writes a separate category local |
| `+0x0c` | `+0x04` | Direction on creature branch; zero on other standard branch |
| `+0x10` | `+0x08` | Z delta for same-XY movement, otherwise zero on creature branch |
| `+0x14` | `+0x0c` | Movement scalar used as unsigned cost divisor on creature branch |
| `+0x18` | `+0x10` | Float accumulation |
| `+0x1c` | `+0x14` | Six-link branch adds edge cost to prior DWORD here |
| `+0x20` | `+0x18` | Initialized to zero; no subsequent direct store found in this routine |

Evidence: initialization `0x4ebb38..0x4ebb4c`, standard stores
`0x4ec21e..0x4ec23f` and `0x4ec471..0x4ec4d0`, six-link stores
`0x4ebcb8..0x4ebced`, copies `0x4ec56d..0x4ec598`.
The existing `MovementPayload` keeps these seven DWORDs as category plus opaque
bytes; this review does not change its public representation.

## Standard branch

The table at `0x5e178c` supplies 26 ordered XYZ offsets. Each destination wraps
X/Y, rejects Z outside `[0, depth)`, and retains duplicates on narrow maps.
`adjacent_cells()` already represents this enumeration. Legality is separate.

When descriptor `+0 != 0`, setup obtains a record from `0x4f4330` and calculates
axis restrictions between descriptor triples at `+0x0c/+0x10/+0x14` and
`+0x2e/+0x32/+0x36`. XY use wrapped absolute separation; Z uses absolute
separation. If the current cell lies within both endpoint distances on an
axis, the candidate-axis check is bypassed. Otherwise the candidate must lie
within the endpoint separation plus one of **both** endpoints. These checks
precede `0x4f3990` at `0x4ec103`. Interpretation as a search corridor is a
hypothesis, not an established gameplay name.

When descriptor `+0 == 0`, the record is computed from object `+0xa8` with
base `0x6a5f80`, stride `0x5c9`. The acceptance call at `0x4ec19a` has object
`descriptor[+4]` in ECX and supplies current XYZ, destination XYZ, an output
category pointer, and a final boolean. That boolean is true when descriptor
`+8 != 0`; otherwise it requires destination equal to the triple at
`+0x2e/+0x32/+0x36`, object `+0xd03 == 0`, and current X equal to destination X
**or** current Y equal to destination Y. Its purpose is unresolved.

Both helpers' EAX result is tested at `0x4ec19f`; zero rejects the candidate.
Their output category local is at frame ESP `+0x30`.

For either accepted standard candidate:

```text
m = metric(2 * wrapped_dx, 2 * wrapped_dy, dz)
```

ECX and EDX are explicitly doubled at `0x4ec208..0x4ec20f`; the inferred C's
single-argument metric call omits these inputs.

For descriptor `+0 != 0`, edge cost is `m * 8`, category is the helper output,
and payload direction, Z delta, scalar, and float are zero. The two trailing
payload DWORDs retain their initialized zeros.

For descriptor `+0 == 0`:

- Same XY retains prior direction and uses destination Z minus current Z.
  Other moves select the direction table and set payload Z delta to zero
  (`0x4ec248..0x4ec2b9`).
- Prior scalar is reused as an input only if Z delta matches and the circular
  direction difference has absolute value at most one; otherwise that input
  is zero (`0x4ec2bd..0x4ec2f8`).
- `0x4eb030` supplies an argument to object method `0x5205b0`; the latter also
  receives a pointer to the scalar local. Helper internals and guarantees that
  the resulting divisor is nonzero still need reconstruction.
- Base edge cost is the unsigned division of the wrapped DWORD product
  `m * record[+0x589] * 4` by that scalar (`0x4ec3c8..0x4ec3d4`). Category 1
  leaves cost unchanged, categories 2/3 triple it, other categories double it.
- A nonzero type-record field at `0x6a652d + type * 0x5c9` enables an additional
  triple cost when destination cell byte `+0xa` has bit `0x10`, or, when Z > 1,
  the cell at Z-1 has that bit (`0x4ec403..0x4ec46d`). The bit's meaning is
  unknown; do not name a terrain type yet.
- Float output adds `(wrapped DWORD(m * 0x900)) / signed DWORD(scalar * 4)` to
  prior float `+0x10`. Assembly zero-extends the numerator into a QWORD for
  `fild`, uses signed DWORD `fidiv`, then `fadd` and `fstp` to float
  (`0x4ec47d..0x4ec4d0`). Exact x87 rounding deserves separate validation.

## Six-link branch

Descriptor `+0x42 == -1` bypasses the initial float limit; otherwise expansion
requires prior float `+0x10` strictly less than that integer converted via x87.
Unordered floating comparisons also stop expansion (`0x4ebb9b..0x4ebbba`).

Six ordered mask entries at `0x5c7108` are tested against current cell WORD
`+8`. Matching links use X/Y/Z tables at `0x5c70c0`, `0x5c70d8`, `0x5c70f0`.
XY wrap. Unlike the standard branch, this block contains **no explicit Z bounds
check** before indexing the destination cell; do not invent a recovered guard.

Descriptor `+0x3e == -1` bypasses a radius test. Otherwise wrapped XY deltas
from the candidate to descriptor `+0x0c/+0x10` are squared with DWORD arithmetic,
summed, interpreted as signed for x87 conversion, square-rooted, and converted
through `0x59bee0`; the result must be strictly less than `+0x3e`.
The helper conversion/rounding mode remains to be analyzed.

A destination is accepted only if its first WORD is nonzero. Edge cost comes
from `0x5c7120`; category retains 5, direction/Z delta/scalar retain zero,
float becomes prior float plus the constant at `0x5c5388`, and payload `+0x14`
becomes prior DWORD `+0x14` plus edge cost. Append uses `0x5024e0`.
The table values and floating constant require a hash-checked export before
implementation; their contents were not inferred from their addresses.

## Translation checklist

1. Preserve all three paths, ordered append behavior, descriptor field offsets,
   and the shared budget boundary.
2. Recover the descriptor construction in `0x54b800` before treating the host
   `ObjectPrefix` and unknown argument as sufficient expansion inputs.
3. Inspect `0x4f4330`, `0x4f3990`, `0x514360`, `0x4eb030`, `0x5205b0`, and
   `0x59bee0`; export six-link tables and the floating constant reproducibly.
4. Separate host validity checks from recovered engine behavior. Preserve DWORD
   arithmetic and unsigned division; do not guess category/terrain meanings.
5. Compare captured ordered candidate bytes against the host output before
   claiming live equivalence. Existing wrapper traces cannot establish this.

## Implemented generator

The implementation is split into internal blocks with source-address comments:

| Engine block | Host function |
| --- | --- |
| `0x4ebae0` entry/budget guard | `expand_neighbors` |
| `0x4ebb03..0x4ebb8e` input setup | `prepare_expansion` |
| `0x4ebb92` branch selection | `generate_neighbors` |
| `0x4ebb9b..0x4ebd02` six links | `expand_six_links` |
| `0x4ebd07` standard setup | `prepare_standard_movement` |
| `0x4ebf1b` ordered loop | `expand_standard_neighbors` |
| `0x4ebf84..0x4ec19f` restrictions/acceptance | `accept_standard_neighbor` |
| `0x4ec1a7..0x4ec4d0` cost/payload | `build_standard_candidate` |
| `0x4ec4d4` candidate append | `append_candidate` using `std::vector<Candidate>` |
| `0x4ec76a` budget decrement | `expand_neighbors` |

This split preserves the existing public API and helper call order. Default
`NeighborMovement` initialization supplies the initial category-5/zero payload;
current-cell decoding remains the explicit map provider's responsibility.

`reconstruction/pathfinding/route_neighbors.hpp/.cpp` now implements both
branches, including the two standard acceptance paths, axis restrictions,
cost/payload arithmetic, six-link limits, ordered append, and the budgeted
entry. `generate_neighbors` omits budget charging for integration with the
existing search core; `expand_neighbors` models the generator's budget guard
and decrement and appends to an existing output vector.

The hash-checked export `working/decompiled/search-support-dtmcnnou/` adds
assembly for `0x4eb030`, `0x59bee0`, and `0x40e250`, and the six-link tables:

| Index | XYZ offset | Mask | Cost |
| --- | --- | --- | --- |
| 0 | -1,0,0 | 0x2000 | 2 |
| 1 | 1,0,0 | 0x0800 | 2 |
| 2 | 0,-1,0 | 0x0400 | 2 |
| 3 | 0,1,0 | 0x1000 | 2 |
| 4 | 0,0,-1 | 0x0200 | 1 |
| 5 | 0,0,1 | 0x0100 | 3 |

The float step at `0x5c5388` is exactly 1.0 (`00 00 80 3f`).
`0x4eb030` simply returns 4 and pops three arguments; its internal behavior is
fully reconstructed. `0x59bee0` temporarily selects x87 truncation toward zero,
converts to signed QWORD, restores the control word, and returns EDX:EAX.
The model uses the low DWORD of the truncated radius; negative wrapped squared
inputs model the low DWORD zero of a masked x87 invalid conversion. Unmasked
x87 exceptions and nondefault engine precision/rounding are not modeled.

The descriptor built by `0x54b800` has mode zero, object token at +4, unknown
argument at +8, a **zero** triple at +0x0c (not object XYZ), zero helper bytes
at +0x18, supplied target at +0x2e, and zero +0x3a/+0x3e/+0x42. Evidence:
the seven zero arguments to `0x40e250` at `0x54b80c..0x54b831`, the stores at
`0x54b81c..0x54b82a`, `0x54b851..0x54b89e`, and limit stores through
`0x54b8be`. At `0x54ba60`, one pending budget argument shifts ESP by four;
the descriptor address is the steady frame's +0x68, not +0x6c.
`creature_descriptor` and `neighbor_search_world` preserve this initialization.
Generic descriptors allow the other branches to be exercised separately.

`NeighborHelpers` still supplies record resolution, acceptance/category output,
object +0xd03, and `0x5205b0` scalar computation, plus map access. These are
explicit unresolved engine dependencies, not guessed walkability rules. The
complete generator control flow is buildable; a standalone reconstruction of
all collision and creature movement helper internals remains future work.

Follow-up: [creature acceptance](pathfinding-creature-acceptance.md) now implements
`0x514360` and its `0x4f5a40` wrapper, replacing mode-zero acceptance with recovered
control flow. It exposes lower `0x4f3990`/`0x4f47d0` providers; complete collision
rules and scalar computation remain unresolved.

Descriptor coordinate members have alignment one, because target starts at
unaligned +0x2e. Payload views use memcpy and compile-time size checks. Integer
operations preserve DWORD wrapping and unsigned division; host callback and
zero-divisor errors throw rather than claiming recovered exception behavior.
Float expressions use long double intermediates on this x86-64 host and store
float at the same boundary. Exact live x87 equivalence remains unverified.

Regression tests in `tests/route-neighbors-test.cpp` cover standard order,
full payload bytes, Z bounds, seam wrapping, axis restriction bypass/rejection,
destination boolean, turn/scalar reuse, vertical movement, category/flag cost
multipliers, unsigned division, six-link masks/order/costs/payloads, strict
radius and float limits (including NaN), empty expansions, budget append, and
integration with the search core without double charging. These use synthetic
map/helper inputs, not live collision traces.
