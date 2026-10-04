# Selected terrain camera initialization and updates

Pinned No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This is an offline recovered setter contract; allocation, resource acquisition,
input timing and full camera lifecycle remain separate.

## Evidence and confidence

Static assembly and isolated execution of unchanged selected PE32 functions
support the following high-confidence field transformations:

| Entry | Selected behavior |
|---|---|
| `004f7930` | Rebind map: +21/+25 map pointers, +61 layer cut, +31/+23d view zero, +35 zero, +59 minimum width/height, +5d logical half of span. Position, height, fractions, viewport and terrain mode (+6d) remain untouched. |
| `004f7970` | Copy rectangle to +11/+15/+19/+1d; +51/+55 become right-left and bottom-top. |
| `004f7a20` | Split unsigned world coordinates into whole X/Y cells with logical shifts by 5 and fractions with mask 31; height uses shift 4 and mask 15. Wrap cells by map extent and clamp whole height to 0..layers-1. |
| `004f7b50` | Add map-axis offsets to cell*32+fraction, split signed totals using floor division and nonnegative remainder, then wrap whole cells. |
| `004f7c60` | Convert screen-direction offsets into map-axis offsets for views 0..3, then call `004f7b50`. Zero input performs no update; nonzero input also writes +99=3 (an opaque original update marker, outside the native terrain camera contract). |

The resource initialization function `004f7860` first calls `004a4e20`.
Only after success does it bind map/resources, set cut/span/diagonal and copy the
viewport. Its initial +41 is zero for maps with more than one layer (also zero
for a one-layer map); it does **not** initialize +39/+3d or +45/+49/+4d in this
selected range. Do not infer an initial camera position from this function.
This full allocation/resource function has static evidence only and is not
executed or replaced by the native implementation.

Screen-direction transformations, for horizontal `x` and vertical `y`:

| View | Map X delta | Map Y delta |
|---|---|---|
| 0 | x+y | y-x |
| 1 | x-y | x+y |
| 2 | -x-y | x-y |
| 3 | y-x | -x-y |

For each axis, `total=cell*32+fraction+delta`, `whole=floor(total/32)` and
`fraction=total-32*whole`. Wrap whole cells modulo the positive map extent.
Negative exact multiples retain a zero fraction. The native implementation uses
checked wide intermediates and rejects arithmetic overflow instead of modeling
32-bit overflow. Dimensions are restricted to 1..128 in X/Y and 1..32 layers.
Viewport extents must be positive and fit signed 32-bit arithmetic. These are
native safety policies, not claims about malformed-input behavior in the binary.

## Native integration

`reconstruction/rendering/terrain_camera.hpp/.cpp` provides map binding,
viewport, position and screen-direction scrolling without Qt or runtime hooks.
The terrain preview application uses these through `--recovered-camera` and
ordered, repeatable `--scroll x,y` steps. The widget still only presents pixels.

The preview selects a 512x256 viewport, map center at world height zero and the
requested view. This starting position, viewport choice and explicit view
assignment are application policies. Span/minimum dimension, fractional
coordinates and scrolling are recovered transformations. Existing `--camera`
and `--pan` retain their earlier explicit configuration mode; mixing them with
`--recovered-camera` is rejected. The camera JSON includes all origin correction
fields and scroll steps for reproducibility. Safe traversal limits still apply.

```sh
working/build/terrain-camera-frozen/mnm-terrain-preview \
  --root working/game-clean --map Realms/Celtic/Forest/CFsec01.map \
  --world --recovered-camera --view 1 --scroll 19,-7 --scroll -91,41 \
  --visibility --output working/tests/camera-example
```

## Validation

`tests/terrain-camera-reference.cpp` privately maps the hash-verified executable,
then executes the original map-binding, viewport, position and screen-scroll
setters. It compares every represented position/origin/span/cut field after
initialization and each update. There are 8,192 deterministic fixtures with
1..128 dimensions, 1..32 layers, all four orientations, unsigned position/height
splits, negative viewports, negative/positive scrolling, exact negative multiples,
zero steps and map boundary wrapping: **139,264 matching state comparisons**.
Resource allocation and original imports are never invoked.

Installed validation covers 16 traces/32 normal/sanitized images on CFsec01,
including zero steps, negative multiples, multi-step panning and large wrapping.

The independent native boundary test also checks preservation on rebind,
height clamping, dimension rejection and atomic scroll overflow rejection.
Normal and ASan/UBSan camera, traversal, queue and visibility tests pass.

```sh
xvfb-run -a env QT_QPA_PLATFORM=xcb QT_OPENGL=desktop \
  LIBGL_ALWAYS_SOFTWARE=1 ASAN_OPTIONS=detect_leaks=0 \
  python3 tools/test-terrain-camera.py --source-root SOURCE_SNAPSHOT \
  --preview NORMAL --sanitized SANITIZED
```

The wrapper guards original files before/after, pins executable/source/installed
asset/build hashes and exports assembly and a report. Installed scene checks
compare complete normal/sanitized image bytes, queue/owner reports and released
surfaces. These are native integration smoke checks, **not** comparisons against
original whole-scene pixels. Prior independently checked terrain traversal and
ordinary producer contracts remain the rendering foundation.

Remaining: original full camera lifecycle, rotation lifecycle/interpolation,
nonwrapped/clamped movement (`004f79b0`/`004f7b30`), follow-target setup,
map initialization and normalization, surface admission, lights/water/entities,
input clock and live integration. These results do not establish a complete
native game scene or any gameplay change.
