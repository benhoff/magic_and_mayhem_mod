# Creature footprint initializer

Build: No-CD SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence: high for the complete valid-receiver leaf; no live replacement.

`004e1260` takes a 36-byte receiver in ECX, raw height DWORD and raw selector
DWORD on the stack, and pops eight bytes. It overwrites all sixteen WORD rows
and height at +20. The function has no semantic return value. Height is stored
unchanged, including zero and overflowing values; occupancy query `004e1200`
applies the wrapped height shift later.

Selector zero writes rows `1800 3c00 7e00 ff00 ff00 7e00 3c00 1800`, followed
by eight zero rows. In the recovered MSB-first convention this is a tapered
shape within bitmap columns 0..7 and rows 0..7, with widths 2,4,6,8,8,6,4,2.
It is not centered in the full sixteen-column bitmap. Every nonzero selector,
including signed-negative bit patterns, writes zero rows 0,1,14,15 and `3ffc`
in rows 2..13: a twelve-by-twelve rectangle at bitmap columns/rows 2..13.
Each bit covers four fine coordinate units on each axis.

Static caller `005063f7` supplies receiver creature+b97, height from a descriptor
at +c, and selector formed by comparing descriptor +8 with 2. That interpretation
comes from disassembly only; descriptor provenance and the complete creature
construction parent have not been executed or integrated here.

Native `initializeCreatureFootprint` returns owned `CreatureOccupancy` values.
It accepts raw uint32 height/selector rather than narrowing the original selector
to bool. Its C++ object layout is not a shared wire contract. Existing query
sources and TL14 evidence remain unchanged.

`creature-footprint-initializer.json` records 273 original initializer executions:
thirteen raw heights, seven selectors and three previous-buffer fills. Full
36-byte outputs and sixteen-byte guards on either side match native output.
Each result is then queried by original `004e1200` and native occupancy at all
XY coordinates -1..64 and six Z boundaries, totaling 7,135,128 matching queries.
Original32, native64 and ASan/UBSan streams match. No original instructions,
callbacks or stubs are changed; executable hash and initializer entry bytes are
checked. Manifest checks pass before and after. Strict normal/sanitized tests
use an independent geometric footprint oracle; standalone CMake/CTest passes.

TL15 proves the initializer and this bounded initializer/query composition.
Creature candidate gathering, status filtering, coordinate production, collision
hit ordering/type behavior, installed descriptor production, construction
lifecycle and live replacement remain separate milestones.
