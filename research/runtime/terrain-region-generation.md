# Terrain generation attempt resets and seed progression

Selected No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This milestone reconstructs the attempt loop in `0052d240` **after** configuration
loading. It composes the recovered [Specific driver](terrain-region-specific.md)
and [one-attempt solver](terrain-region-solver.md) over an already described
catalog. CFG/catalog initialization, generated terrain assembly and live
replacement remain separate milestones.

## Caller behavior

The complete original entry stores the supplied seed, calls descriptor
initialization `0052d320`, then CFG/catalog loading `0052d530`. The bounded native
entry instead receives owned placement state, one request per descriptor and a
32-bit seed. It does not perform those configuration operations.

The selected loop starts at `0052d269` and executes at most ten attempts. Before
each attempt it:

1. Sets all descriptor placement counts to zero, including suppressed Specific
   counts from an earlier attempt.
2. Resets grid occupancy, section/descriptor/rotation/source-X fields and edges
   through `0052d440`. **Stored source Y is not reset.**
3. Clears all 25 retained location entries and all candidate tables (`0052d390`).
4. Clears scratch section/descriptor/edges to ffffffff and rotation to zero.
5. Runs Specific placement `0052f3f0`, then the solver `00532030`.

After each attempt it adds **500** to the receiver's seed, including a successful
attempt. The addition wraps modulo 2^32. A successful result stops before another
reset. Ten failures return the tenth attempt's partial placements, counts,
request mutations, tables and carry; they do not clear the final failure state.
The original full caller returns a normalized zero/one result, unlike the
solver's low-byte-only result contract.

Descriptor identity, maxima, edges/connectors and Specific request fields are
retained across resets. In particular, normalization or failed fixed-anchor
recovery can change request coordinates to wildcard values; later attempts use
those mutated requests. They do not start again from the caller's original
request vector. Stored source-row values are also retained across attempts and
can contain a previous placement's row or ffffffff from removal.

## Owned native interface

`terrain_generation.hpp/.cpp` supplies `resetRegionGenerationAttempt` and
`generateRegionPlacement`. Both consume state by value. The result owns the
final state, mutated request vector, last location table, next seed, completion
status and at most ten attempt records. Records contain the seed used, status,
backtrack counters from the native solver and ordered Specific diagnostics.
There are no shared PE pointers, original object layouts, widgets or Wine
services in these interfaces.

A shared `validateRegionPlacementState` now exposes the foundation's ownership,
extent, count and assignment checks for both Specific placement and generation
reset. This avoids duplicating that validator in each consumer. Existing native
request bounds, connector/rollback refusals and solver operation limits remain
unchanged. Refusal throws without changing the caller's owned input; successful
or ten-attempt failed generation returns an inspectable owned result.

## Unchanged original loop comparison

```sh
python3 tools/test-terrain-generation.py
```

The wrapper guards original inputs before and after, pins the executable hash,
independently decodes the 683 installed MAP headers, records sources/exports and
supports a frozen tree via `--source-root`.

The 32-bit oracle privately maps the selected PE. A compiled assembly adapter
creates the original saved-register/local/argument stack, supplies the described
receiver in ESI and zero in EBX, and enters the **unchanged instruction stream at
0052d269**, immediately after CFG loading. It checks the entry bytes and return
sequence, with the whole-file hash providing the stronger build pin. The
original loop's `ret 12` returns through the adapter's synthetic arguments to
its C caller. No executable instruction or executable file is patched.

The previously validated private FS exception head supports the selected
chooser paths. The same checked private MessageBoxA import binding records
configuration-warning categories and returns IDOK, without dialogs. These are
reference adapter policies, not a claim of original CFG loading, Win32
initialization or a complete original launch.

The oracle checks the loop's final status and seed, the complete packed
50-descriptor bank including requests/counts, all 25 grid slots and candidate
tables, all 25 retained location slots and final scratch carry. Reset fixtures
also compare the original reset helpers separately on incoming placed cells,
nonempty candidate tables and arbitrary controlled source-row values in empty
cells. This checks retention as well as fields which are cleared.

Independent original helper calls replay each attempt boundary, comparing each
native trace entry's seed, completion status and ordered warning categories;
the final replay receiver must byte-match the receiver produced by the unchanged
loop. Native trace backtrack counters retain the previously recovered solver's
meaning; they are not new independently captured counters from the caller.

## Evidence and limits

The corpus contains 4,096 general synthetic cases and 256 targeted recovery/
suppression cases, plus two single-source catalog fixtures per installed header
(1,366 installed cases). Fixtures vary catalog capacity, asymmetric edges,
Specific requests, source shapes, prior placements/tables/source rows and seeds.
Targeted cases force a fixed-anchor conflict on the first failed attempt and
check that later attempts keep the resulting wildcard request; full-grid cases
exercise location failure/count suppression.

All **5,718 reset cases matched**. **5,701 complete-loop cases matched across
19,883 attempts**: 4,149 succeeded, 1,552 exhausted all ten attempts and 1,636
retried, including 84 successes after a retry. There were 782 multi-block cases
and 241 seed-wrap cases. Warning counts were 128 requested-rotation rejections,
128 fixed-location rejections, 64 requested-location failures and 128 all-location
failures. All 1,366 installed-header loop fixtures were accepted.

Seventeen synthetic cases reached the existing native `Region fixed rollback
before grid` refusal after Specific placement. Their reset state is compared,
but their unchecked original solver/loop paths are not executed. Their full-loop
equivalence is not claimed. These refusals are explicit native bounds policy,
separate from the 5,701 compared cases and from original error handling.

The complete deterministic corpus and refusal counts run again under ASan/UBSan.
Ownership/reset units cover retained source rows, cleared counts/candidates,
successful seed wrap, ten failures with partial final state, persistent request
normalization and invalid request ownership. The rendering reconstruction CTests
passed all twelve tests in each normal and sanitizer build. Leak detection is disabled under the
execution sandbox; address and undefined-behavior checks remain enabled.
Counts and provenance are recorded in [the companion report](terrain-region-generation.json).

Confidence is high for the compared post-CFG domain. These installed-header
fixtures are not shipped realm recipes or generated scene images. Remaining
work is complete CFG/catalog construction and connector threshold policy,
conversion of successful assignments to owned terrain assembly plans, and
generated terrain scene comparisons. Entities, water, lighting and live
integration remain separate.

The subsequent [catalog milestone](terrain-region-catalog.md) now reconstructs
selected recipe/header catalog construction and the original connector-threshold
scan. Original CFG/file-error recovery and generated scene production remain
separate.
