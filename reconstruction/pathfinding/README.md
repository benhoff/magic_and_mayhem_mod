# Route-request reconstruction: first milestone

This directory contains readable, buildable **host-side models**, not original
source or installed engine replacements. The search routine is still an
unresolved dependency supplied through a test callback. Nothing here is a live
hook or changes gameplay.

## Three layers

1. Raw C and assembly: `research/runtime/decompiled/nocd/`, checksum-pinned
   to no-CD SHA-256
   `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
2. Analysis annotations: `tools/ghidra/AnnotateRouteMilestone.java`, applied
   only to that exact PE32 build at preferred image base `0x00400000`.
3. Readable model: `route_request.hpp` and `route_request.cpp`, with tests in
   `tests/route-request-test.cpp`.

The generated Ghidra database stays under ignored `working/`. Scripts and
the readable model stay tracked. Raw exports are never edited or formatted.

## Reproduce

```bash
./tools/verify-decompilation-baseline.py
./tests/test-reconstruction.sh
./tools/decompile-game.py --annotated
```

The first two commands verify evidence and test the host-side reconstruction.
The third creates a fresh analyzed project and annotated C export; it does
not replace the raw snapshot. Normal `decompile-game.py` remains an unannotated
export, and `--all` still controls export scope. No commands overwrite earlier
runs. The test requires a C++17 compiler (`CXX` or `g++`), with ASan and UBSan.
Leak detection is disabled because it requires ptrace unavailable in the sandbox.

## Evidence and confidence

| Model | Source VA | Evidence / confidence |
| --- | --- | --- |
| `normalize_coordinate` (X) | `0x0040e290` | Signed-negative addition loop, unsigned-positive subtraction loop; high within positive-dimension domain |
| `normalize_coordinate` (Y) | `0x0040e8a0` | Same operation using a different dimension global; high |
| `copy_z` | `0x0040eeb0` | Copies one DWORD; high for operation, medium for Z label |
| `route_request` | `0x00512800` | Four stack DWORDs, ECX object, local budget, constructor stack tracing, snapshot copy and flag stores; high static evidence |
| Search callback boundary | `0x0054b800` | Six stack DWORDs, ECX context, `ret 0x18`; labels inferred, internal behavior not reconstructed |

Coordinate labels and object identity are inferred. Unknown argument/fields
retain explicit unknown names. The helper constructors return their destination
pointer in EAX. The annotation script records that return and marks the three
constructors inline **for analysis only**, to expose caller stack-memory effects.
This is not code inlining or binary rewriting.

`0x00690354` aliases `0x00690148 + 0x20c`: the wrapper's flag store affects
the first search-state byte in the shared route context. The model preserves
that alias rather than inventing an independent global. The wrapper does not
reset it after search; search itself can change it. This also means the search
initialization branch is entered when this wrapper calls it, not necessarily
a continuation of a previous search. Other callers remain to be investigated.

## Layout and behavior constraints

- `RouteSnapshot` is exactly `0x20c` bytes; count is at `+0x10`.
- The object's snapshot starts at `+0x96b`, giving count at `+0x97b`.
- Object flags at `+0xb8b` and `+0xd03` retain their byte offsets.
- Search state at context `+0x20c` is outside the copied snapshot.
- All structures are packed, with compile-time offset/size assertions.
  Object/context types are **minimum accessed prefixes**, not complete classes.
- All opaque snapshot bytes are copied, not just known fields.
- Budget is a stack-local copy; changing it does not mutate the configured limit.
- Any nonzero count reports success; the wrapper itself does not cap count at 16.
- The host callback ABI must never be cast to the game's 32-bit thiscall ABI.
  Real hooks need a separate x86 bridge and verified live object lifetimes.

Positive signed dimensions and a non-null backend are host-model preconditions.
Invalid setup throws before changing modeled state. Those guards are NOT
discovered engine branches; this model does not claim equivalence for corrupt
dimensions or a missing backend. No search algorithm, allocation strategy,
priority ordering, or movement rule is changed.

## Validation and remaining work

Tests cover 63,519 coordinate cases, signed extremes with large dimensions,
Z copying, exact callback arguments, budget isolation, aliasing, nonzero/zero
counts, opaque-byte copying and untouched surrounding memory. Compile-time
assertions check binary offsets; ASan/UBSan check the host model.

These tests validate a reconstruction against independently asserted static
observations. They do **not** prove live-game equivalence. Remaining validation:
capture arguments and state for a simple route, blocked target, budget
exhaustion and several simultaneous orders; compare those traces to the model.
No live tracing/hook installation is included in this milestone.

Actual Ghidra validation on 2026-10-02: annotated run
`working/decompiled/nocd-96ofmeq_/` applied the script successfully, exported
all seven routines without failures, and recorded `annotations_applied: true`
and `input_unchanged: true`. The annotated wrapper now shows the correct
`search_route_candidate(context, object, unknown_argument, x, y, z, &budget)`
call and no longer contains the earlier `extraout_ECX` placeholders. The
remaining search export is still only partially typed. The full tooling suite
and dedicated reconstruction/baseline tests pass.

Next static milestone: recover queue/node/waypoint structures and the heuristic,
then reconstruct search with its original budget and partial-route behavior.
