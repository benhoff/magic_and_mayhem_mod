# Native RGB565 surface clipping backend

2026-10-06. `NR.surface-copy.clipping` is a native policy modeled on the separately
[captured Wine Surface2 driver baseline](surface-driver-clipping.md). It is not a
recovered Windows-driver contract or an original game replacement.

`GlBlitter::setClipper` stores an owned, destination-specific descriptor. Detached,
attached without a list, attached with an empty list, and an ordered list of
regions remain distinct. Native policy admits at most 32 positive-size, in-bounds,
disjoint regions and preserves supplied order. Setter validation completes before
state changes. Updates retain clip state; content swaps exchange textures while
clip state stays with the surface identity; destruction retires it. Shared COM
clipper identity, HWND lists and automatic region normalization remain pending.

`surfaceCopy` supports distinct RGB565 source/destination surfaces. Its separate
Blt/BltFast admission returns the seven captured HRESULT outcomes. Source and
destination geometry is one-to-one; unsupported stretching, other formats,
self-copy and unvalidated flag combinations raise a native admission exception.
The selected source-key flags represent the captured **missing-key** cases, not
new successful keyed clipping. Busy flags are explicit caller admission
observations for fixture replay, not an implementation of native Lock/Unlock,
lost surfaces or Restore.

The geometry planner widens arithmetic before subtracting/adding signed input
coordinates. Blt clips destination pieces and maps each back to the source;
it retains valid earlier pieces when a later mapping fails. GL executes those
pieces with the existing integer copy primitive, then returns the HRESULT.
The retained partial-failure cases each write six pixels before `INVALIDRECT`.
Empty regions cause no draw. BltFast admission/clipper rejection follows the
bounded captured cases; it does not consume a clip list. Source pixels stay
unchanged. Neither normal copy planning nor drawing uploads or reads back pixels.
`RenderStats::copies` continues to count draw pieces, not API calls.

The existing `copy` primitive keeps its prior already-in-bounds semantics. This
allows a separate command operation to carry Surface2 policy without changing
historical v1/v2 COPY admission. Clipper state is not yet in those wire contracts.

## Evidence and checks

[Final backend comparison](native-surface-clipping-backend-final-20261006.json)
feeds the C++ executable only captured inputs and initial pixels, never expected
HRESULTs or post-call pixels. It compares all 408 native results, full destination
arrays and preserved source arrays afterward with the independent driver corpus.
Its policy checks cover owned clip storage, invalid setters, update/swap/destruction
lifetime, unsupported operations, extreme coordinates and cleanup. The two existing
native blit/surface suites pass; the five original opaque fixtures also pass fresh
CPU/OpenGL replay after the matrix update. All capture evidence remains immutable.

```sh
cmake -S renderer -B working/build/renderer -DBUILD_TESTING=ON
cmake --build working/build/renderer --target render-surface-copy-fixtures
xvfb-run -a python3 tools/check-native-surface-clipping.py
```

Regression requires only the retained fixture corpus, Qt/Mesa and Python; it does
not launch Wine, the game or consume source media. Reports use new per-run paths,
or an explicit new `--report`. `opengl-surface-clipping` is registered with CTest.

The command protocol/replay is the next integration step. Actual game clip regions,
original error/retry/Restore paths, other formats, keys/fills with clipping,
region permutations/normalization, overlap, primary/video surfaces and Windows-driver
comparison remain separate. Renderer integration here is headless and synthetic;
no live transport, replacement or whole surface milestone is claimed.
