# Seeded terrain section and wildcard-location selection

Selected No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This milestone reconstructs the RNG, single-block descriptor fields and edge
admission, compact candidate/location tables and seeded selection helpers.
It supplies owned native selection primitives for the next generator stage.
The terrain preview still loads only complete authored regions; this milestone
does not enable random recipes or add a generated scene.

## Recovered helpers

| Original address | Selected operation |
|---|---|
| 0054e170 | Initialize the 250-word RNG from a 32-bit seed |
| 0054e200 | XOR two state words, store the result, advance cyclic indices |
| 0052edf0 | Build a single-block section descriptor from MAP header/caller fields |
| 005300a0 | Admit a section at one grid slot, including rotated edge constraints |
| 0052d4e0 | Clear one 50-entry candidate table |
| 0052f790 | Compact ordinary or special random descriptors with valid rotations |
| 0052ff10 | Choose a compact candidate entry and one of its valid rotations |
| 005310f0 | Compact valid locations for a requested descriptor/rotation |
| 00531710 | Choose a compact location and one of its valid rotations |
| 00530c60 | Choose a valid relative rotation at the current anchor |

`terrain_selection.hpp/.cpp` remains under reconstruction, independent of Qt,
widgets, injected hooks and application orchestration. It consumes native MAP
header storage. Its grids, descriptors, tables and return values own ordinary
C++ data; no original address or packed host layout is exposed to callers.
Selection functions do not mutate caller inputs. An empty valid table or
rotation set returns `std::nullopt`.

## RNG and selection

The original initializes 250 words in descending index order. Each word uses
a 64-bit multiply by `41c64e6d`, adds `3039` with carry, combines the upper
half-word fields, and then applies the 32-bit diagonal masks at word indices
3, 10, ..., 220. Initial indices are 0 and 103. Each draw XORs the two selected
words, replaces the first, and advances both indices modulo 250. Arithmetic
uses explicit unsigned 32/64-bit storage, including seed wraparound.
The selected index lookup table at `005eb940` is confirmed to be this cyclic
increment table by the executable fixture.

The selection helpers construct a local RNG, initially with the constant
`499602d2`, then **reseed it from the region receiver's seed for every choice**.
The initial constant has no effect on successful choice results. Native helpers
start directly at the supplied seed and have no shared mutable RNG state.
The region seed is not advanced by these helpers. The ten-attempt wrapper's
separate `seed += 500` policy remains outside this milestone.

Candidate and location choices consume two draws. The first positive 31-bit
word modulo the number of compact entries selects an **entry index**; the
second selects an ordinal among that entry's enabled rotations, in order
0, 1, 2, 3. Selection is uniform over compact entries, not all entry/rotation
pairs. The candidate index must subsequently be translated through the table
to its descriptor index. Rotation-only selection consumes one draw.
Random occurrence limits affect admission; they are not choice weights.

## Single-block descriptor and admission

MAP +18/+1c must both be one for this native helper. The selected north/east/
south/west labels come from decoded MAP header +20/+28/+30/+38, preserving
32-bit values and the -1 sentinel. A Specific descriptor has occurrence maximum
one; a random descriptor receives its maximum from caller data. The original
special flag is set if any nonzero edge label occurs only once among the four
edges. The reader retains these fields; the recovered interpretation resides
in reconstruction, separate from native format parsing.

Admission rejects an occupied destination or a descriptor whose placed count
**equals** its maximum. The comparison is equality in the selected binary,
not greater-than-or-equal; fixtures include counts above the maximum. Rotation
moves edge labels clockwise. Neighbor coordinates wrap toroidally. Opposite
labels must agree when a grid dimension is one.

North, east and south comparisons accept an unspecified (-1) facing neighbor
edge. The original single-block west check has an asymmetric sentinel gate:
it compares candidate W to neighbor E but checks **neighbor W** for -1.
The native model preserves this behavior. Its explicit unit fixture and
original comparisons distinguish it from a symmetric edge matcher. The later
multi-block walk contains additional checks and different side effects and is
not modeled by this helper.

Candidate scanning visits up to 50 descriptors in input order, excludes
Specific entries, separates ordinary versus special descriptors, and retains
only descriptors with at least one admitted rotation. Valid entries occupy a
compact prefix. Location scanning visits rows first, then columns, and likewise
compacts only valid anchors. A requested rotation -1 tests all four; 0..3 tests
only that rotation. Either-coordinate wildcard normalization and Specific
placement/fallback orchestration are still separate.

## Fixture boundaries and reproducibility

The reference executable privately maps the pinned PE at its preferred image
base and calls the unchanged helpers. RNG initialization/each draw compares
all 250 words and both indices. Admission compares boolean results, candidate
and location fixtures compare complete initialized packed tables (including
unused slots), and seeded choice fixtures compare entry index, rotation,
no-choice output preservation and unchanged receiver seed. Native API state
is semantic storage; original scratch/carry side effects are not claimed.

The three seeded selection helpers use Windows `FS:0` exception-list setup.
The Linux PE32 fixture installs a private four-byte FS segment, checks that the
exception head returns to its sentinel after each call, and restores the prior
FS selector. No executable bytes are patched, no Wine process is launched and
no exception handler or Win32 import is executed on these selected paths.
This fixture setup is confined to the test executable.

Installed single-block MAP headers are independently unpacked from the guarded
installation. Each receives a deterministic fixture section ID (the MAP header
does not contain a section ID) and is passed to the unchanged descriptor builder
in both Specific and Random forms. This confirms selected header fields and
special classification for all 616 installed single-block MAPs. It does not
execute the original whole-CFG loader or map-generation caller.

```sh
python3 tools/test-terrain-selection.py
```

The wrapper pins the executable hash, guards the original manifest before and
after, records source/input/helper hashes, compares normal original/native
fixtures, and runs the same native corpus plus ownership/refusal units under
ASan/UBSan. Optional `--source-root` selects a frozen source snapshot.
Validation passed: 256 seeds/262,144 draws with full RNG state; 207,840
single-block admission checks; 4,096 candidate and 10,240 location tables;
78,584 original seeded choices; and 1,232 descriptor comparisons for the
616 installed single-block headers. The native sanitizer corpus also checks
831,360 rotation-mask choices. All eight reconstruction CTests passed in each
normal and ASan/UBSan build.

Evidence counts and provenance are in
[the companion report](terrain-region-selection.json).

Native capacities are a 1..5 block grid, at most 50 descriptors/candidates and
25 locations, rotations 0..3 or -1 where allowed, IDs 0..99 and occurrence
maxima 0..999. Descriptor construction additionally requires square nonempty
MAP extents at most 128 and layers 1..32. Malformed grids, masks, coordinates,
capacities and multiblock MAP inputs throw before selection. These refusals are
native bounds policy, not original error-dialog equivalence.

Confidence is high for the selected helper domain and full RNG state. Remaining
work is multi-block descriptor/connector admission, candidate pruning, Specific
placement and rotation fallback, solver/backtracking, failure handling/retries,
whole-recipe generation, conversion to the owned assembly plan, and generated
scene comparisons. General generation, live integration, entities, lighting
and water remain unverified.
