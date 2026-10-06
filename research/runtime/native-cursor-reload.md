# Native global cursor reload and independent DIB raster comparison

The recovered `-1` route now has an explicit global cursor callback in
`reconstruction/rendering/surface_retry.hpp`. It carries no source/destination
selector. Successful Restore still performs key setup first and ignores key
failure; an absent cursor binding refuses explicitly. Existing callback booleans
remain compatible. Conflicting boolean/explicit routes and unknown route values
refuse before any draw. Budgets retain their separate exhausted/returned states.

`reconstruction/rendering/surface_bitmap.hpp` models the bounded original file
contract: BM and fixed-size header checks, full palette reads by bit depth,
`bfSize - bfOffBits` pixel length, and sequential reads without offset seeking.
Missing/truncated files return failure. The cursor helper resolves the fixed
`bitmaps\cursors.bmp` path through a supplied reader and sends owned DIB input to
a separately bound global draw callback. It ignores callback return values and
returns zero, as recovered. Unsupported/over-budget native inputs and backend
exceptions remain explicit refusals, not new original success/failure findings.
Reader lifetime/path resolution and global surface ownership belong to the
application adapter; no build addresses enter renderer code.

`renderer/dib.cpp` converts owned uncompressed 40-byte DIB inputs to top-down
RGB pixels. The supported inputs have positive dimensions, one plane,
1/4/8-bit full RGB palettes with usage zero or 24-bit BGR with usage one,
zero declared palette counts, and dimensions at most 2048. Row alignment,
indexed nibble/bit order, palette BGR ordering and bottom-up rows are explicit.
The original installed 24-bit cursor's trailing two payload bytes are retained
by the parser and ignored outside the raster extent. Negative/oversized dimensions,
compression, alternate headers/counts/usages and short buffers refuse.

`GlBlitter::reloadDib` accepts canonical RGB24/32 surfaces with masks
`00ff0000/0000ff00/000000ff`. It writes at origin without scaling, cropping the
source to the destination dimensions. An attached clipper, indexed/16-bit or
alternate-mask destination refuses. Only the written rectangle becomes valid.
A larger invalid destination keeps its untouched border unknown; complete read
and presentation still refuse. A sentinel reload defines only the global cursor,
not either surface whose Restore triggered the call.

## Independent execution evidence

The [real GDI capture](surface-dib-gdi-capture-20261006.json) executes
StretchDIBits to a 32-bit BI_RGB DIBSection in one reserved Wine 11.16 session.
Inputs come from the retained original loader corpus, not native output:
1/4/8-bit synthetic bitmaps, 24-bit synthetic input, the sequential-offset-gap
case, and the installed 400 × 280 cursor. Each has equal, cropped and larger
DC destinations, initialized to `00556677`. A GdiFlush precedes owned pixel
capture. Probe completion and source fingerprints are checked; the immutable
manifest passes before and after. A failed sandbox initialization produced no
capture and is excluded; the successful run used the required socket access.

The [offline comparison](surface-dib-gdi-offline-20261006.json) matches all
336,732 independently captured RGB pixels in 18 cases. Six regression tests
cover pixel/border mutation, usage, failure and unused-high-byte boundaries.
GDI raw high bytes are retained but excluded from RGB equivalence. Microsoft's
[StretchDIBits contract](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-stretchdibits)
defines the color-usage and raster-operation inputs; its
[CreateDIBSection contract](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createdibsection)
requires synchronization before reading GDI-written memory. This probe supplies
the original helper's usage values rather than assuming generic BMP decoding.

The [current native result](native-cursor-reload-rgb24-32-20261006.json) covers:

- 1,456 C++ recovery dispatch traces against projected unchanged original x86
  captures, 813 budget stops preserving trace prefixes, and two invalid budgets.
- Twelve file-load results and exact header/palette/pixel hashes against the
  original submitted buffers, including failure and sequential offset behavior.
- 36 native RGB24/32 reload cases, comparing readback and presentation against
  the 18 independent GDI outputs: 1,346,928 RGB pixel comparisons.
- Twelve scripted recovered wrapper/native cursor compositions, including
  missing assets and failed Restore. Triggering surfaces stay undefined after
  successful Restore, even when the global cursor becomes readable.
- 137 explicit refusals, including unknown borders, malformed inputs, incompatible
  surfaces, DC clipping, missing cursor binding and ambiguous routes; zero final
  native surfaces.

The [initial RGB32 result](native-cursor-reload-20261006.json) is retained with
its original source hashes. It predates expansion of the test/runner to RGB24;
its stale fingerprints do not assert validation of those later test versions.
The [fresh compatibility regression](original-surface-retry-cursor-regression-20261006.json)
compares all 4,480 earlier callback schedules, preserves 1,536 bounded stops,
and passes the 30 native restoration policy checks. Other historical subsystem
fingerprints remain historical; this work does not refresh them silently.

## Reproduction and limits

Offline comparison and refusal tests require only Python's standard library:

```sh
python3 tools/check-surface-dib-gdi.py
python3 -B tests/test-surface-dib-gdi.py
```

Build the native library and run the bounded C++/GL comparison with a fresh report:

```sh
cmake -S renderer -B working/build/cursor-reload -DBUILD_TESTING=ON
cmake --build working/build/cursor-reload --target mnm-renderer
xvfb-run -a python3 tools/check-native-cursor-reload.py \
  --report working/tests/new-native-cursor-reload.json
```

To recapture the GDI endpoint, use a new corpus/report destination with
`tools/capture-surface-dib-gdi.py` under Xvfb. The runner reserves the shared Wine
lock, refuses active Wine sessions, clones a disposable prefix and cleans up
only that prefix. Native/offline checks need no Wine or original installation.

Confirmed comparison is standalone Wine GDI to an RGB32 DIBSection, not an
original DirectDraw GetDC destination. DirectDraw DC identity, RGB565/indexed
conversion, DC/clipper palettes and clipping, failure out-parameters, compressed
or malformed original allocation branches, other reload callback bodies,
real lost-draw composition, Windows hardware drivers and live/wire recovery
remain pending. Headless composition binds the global cursor through the test
adapter; no live interception or engine bypass is introduced.

## Subsequent DirectDraw DC step

The subsequent [Surface2 DC comparison](surface-dib-ddraw.md) adds real
GetDC/GDI/ReleaseDC RGB565/RGB32 outputs and bounded native RGB565 reload.
All 72 native/DC cases match 2,693,856 pixel comparisons. RGB565 native
presentation now uses observed bit replication; 24 scripted cursor compositions
pass. This supersedes the earlier RGB565 destination refusal for canonical masks.
The earlier results above retain their exact historical hashes. Indexed/DC
clipping, native DC admission, actual loss and live/wire recovery remain pending.
