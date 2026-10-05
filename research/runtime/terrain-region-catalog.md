# Owned region catalog construction and connector thresholds

Selected No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This milestone connects the existing owned CFG recipe fields to the recovered
terrain descriptor catalog, request vectors and connector thresholds. It is
bounded offline evidence. Original CFG token/file/error handling, generated
scene assembly and live replacement remain separate.

## Selected original behavior

`0052d530` loads section headers in **Specific-list order followed by Random-list
order**. Repeated section IDs remain distinct entries. The first header supplies
source-block width/height through tile dimensions divided by block dimensions;
the caller checks agreement across later sources. Descriptor expansion then
visits each source's rows first and columns second through `0052edf0`.

The connector scan in `0052e3ca..0052e44d` starts with a signed running value zero
and visits eight header edges in order: north 0/1, east 0/1, south 0/1, west 0/1.
It compares the first four and final two as usual maxima. The **south-0**
comparison, if greater, assigns **north-0**, and the **south-1** comparison, if
greater, assigns **north-1**. These are confirmed instruction/decompilation
assignments, not a guessed maximum calculation. The running value can decrease
and the final threshold need not exceed every serialized outer label.

For example, `[3,7,0,0,90,80,0,0]` finishes at seven, giving threshold eight,
rather than 91. The reconstruction deliberately preserves that selected behavior;
it does not repair it or assert that every outer label is below the threshold.
The threshold is running-value +1 at receiver +0c. Four independent connector
counters start at running-value +1/+2/+3/+4 at +10/+14/+18/+1c. Each consumed
internal connector pair advances its corresponding counter by four.

`0052edf0` writes one descriptor per source block, preserving section identity,
Specific status, requested column/row/rotation, source block coordinates,
dimensions/layers, count zero and selected edges/special flag. Specific maxima
are one. Random maxima are caller occurrences and all their request fields are
-1. The receiver's maximum source layer count updates while descriptors are
expanded. Header expansion and special-edge classification use the already
validated [constraint foundation](terrain-region-constraints.md).

## Owned catalog interface

`buildTerrainRegionCatalog` in `terrain_catalog.hpp/.cpp` accepts an owned recipe
and an ordered vector of MAP pointers for that call. It consumes header fields
only and retains no source pointer or cell buffer. Returned storage owns the
bank, request vector, final connector counters, block-grid dimensions, square
section side and maximum layers. `firstDescriptors` records each source's bank
start, and `sourceForDescriptor` preserves source-entry provenance, including
repeated section IDs. These vectors prepare successful generated assignments
for later owned terrain assembly.

The existing native CFG reader supplies the selected recipe keys; this service
does not duplicate parsing or path resolution. File orchestration still belongs
outside reconstruction. The catalog can initialize
`makeRegionPlacementState` and pass its requests into the
[generation-attempt interface](terrain-region-generation.md); generated scene
production has not yet been wired into the preview.

Native policy requires a nonempty exact source list, 1..5 block dimensions,
supported MAP header extents, square source blocks with equal side lengths,
Specific fields within existing native bounds, Random maxima 1..999 and at most
**50 expanded descriptors**. Capacity is checked after accounting for every
multi-block source, rather than counting only recipe entries. Missing/malformed
headers, all-rotation size impossibility and connector arithmetic overflow are
refused. The original corresponding file/dialog recovery paths are not claimed.
Incoming recipe/MAP storage remains unchanged on success or refusal.

## Bounded binary reference

```sh
python3 tools/test-terrain-catalog.py
```

The wrapper verifies immutable originals before and after, pins the executable
hash, independently parses the three installed realm CFGs and decodes the 564
available referenced MAP headers. It records input, source, export and helper
hashes and supports a frozen source tree with `--source-root`.

The 32-bit harness maps the PE privately. It copies the **unchanged 131-byte
threshold instruction window** `0052e3ca..0052e44d` into an isolated executable
buffer and appends an adapter return. All relative branches remain within that
window. A compiled stack adapter supplies the original header-array offsets and
receiver/count registers. Entry/exit bytes are checked, and the pinned whole-file
hash identifies the selected code. No original-image instructions or executable
file are patched. The descriptor builder runs unchanged at mapped `0052edf0`.
Valid selected paths make no Win32 calls; original file loading and warning
branches are not executed.

Comparisons cover the threshold, all four final connector counters, source-bank
starts, maximum layer count and the complete 50-descriptor packed bank including
requests, counts, source coordinates, dimensions, edge labels and special flags.
The controlled grid/candidate/carry state is also checked for unintended changes.
Native provenance indices are owned interface metadata, not a new shared
original field. The complete corpus is repeated under ASan/UBSan. The sanitizer
helper links real Qt asset and packed-container dependencies for the native CFG
reader; its selected reads consume independently decoded CFG byte buffers.

The synthetic corpus contains 2,048 multi-source catalogs, varied block shapes,
Specific/wildcard requests, occurrence maxima, asymmetric and signed edge labels,
mixed layers and evolving counters. All 2,048 synthetic cases matched. With
39 available installed catalogs, **2,087 catalogs / 26,888 expanded descriptors
matched**. In 291 cases, the recovered threshold differed from a simple maximum.
Selected CFG IDs, sizes, Specific tuples and Random lists matched an independent
parser for **all 40 shipped recipes**. Earlier authored-region work separately
checks the other selected recipe strings/paths.

Medieval **REGION0** refers to the absent `Realms/Medieval/Test` sections
0 and 20..29. Its eleven missing MAP inputs are recorded individually in the
report. The fixture uses explicit unavailable-header markers and the native
builder refuses that catalog; its original missing-file recovery is not executed
or claimed equivalent. The other 39 shipped recipe catalogs were accepted.
The missing Test recipe does not justify borrowing similarly named Village
assets or changing installed input.

Unit checks cover ownership, repeated IDs, multi-block source mapping, threshold
quirks, expanded capacity, missing headers, mismatched sides and arithmetic
bounds. All thirteen rendering reconstruction tests passed in both normal and
ASan/UBSan builds. Leak detection is disabled under the execution sandbox;
address and undefined-behavior checks remain enabled. Counts and provenance are
recorded in [the companion report](terrain-region-catalog.json).

Confidence is high for selected available-header catalog construction and the
threshold instruction window. Original CFG parsing, missing/invalid-file dialogs
and continuation policies remain separate from native strict refusal. Next work
is generation from complete available recipe catalogs, successful-assignment
conversion to owned assembly plans, and generated terrain scene comparisons.
Entities, water, lighting and live integration remain separate.

The next milestone now connects complete available catalogs to the generation
loop and converts successful assignments into owned concrete section-copy plans;
see [generated terrain plans](terrain-generated-plans.md). Installed generated
scene production and frame comparisons remain subsequent work.
