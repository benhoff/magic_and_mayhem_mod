# Owned terrain light fields and controlled source stamps

Confirmed for the working no-CD Chaos.exe build, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This milestone adds offline per-cell light storage and selected source stamping,
with native Qt/OpenGL preview integration. Entity source discovery and live
replacement are separate milestones.

## Recovered storage and initialization

The map object at `006c5490` owns five byte buffers at offsets `+7cc`, `+7d0`,
`+7d4`, `+7d8`, and `+7dc`. Their byte count is `+7e0`. The selected initializer
`004ecdd0` allocates them when `+7cc` is null, using
`((mapLayers+1)/2) * width * height`; this size calculation is confirmed by
static instructions at `004ecde0..004ecdee`. Allocation/error behavior is not
reconstructed. The comparison fixture supplies preallocated guarded buffers.

Initialization fills `+7dc` with the low byte of `005e18f0` (image value -127),
then copies through `+7d8`, `+7d4`, `+7d0`, and `+7cc`. The selected fill/copies
execute unchanged in the fixture, including DWORD and tail-byte cases.
It calls the kernel builder `004f1890`, and sets update flags `+6426=1`,
`+642a=1`, `+642e=0`. Native storage owns all five buffers; it does not reproduce
those update flags or scheduler behavior.

The published buffer is global `006c5c5c`. Row offsets are `006cb942`, matching
map `+64b2`; paired layer offsets are `006cb8c2`, matching map `+6432`.
A map cell samples byte `((layer/2)*height + row)*width + column`. Adjacent
map layers therefore share a light plane; XY is toroidal while light Z is clipped.
These offsets are static build-specific references, not portable runtime pointers.

## Global control admission

`004e46f9..004e47ef` reads `GLOBAL_OPTIONS` `AmbientLight` and `LightRamp`.
Present integers are clamped to -127..127. Ambient is negated after the clamp;
ramp is stored directly. Missing values preserve current controls. Image defaults
are ambient -108 and ramp -16 at `005e18e8` and `005e18ec`. The installed packed
`CFG/Encrypted/chaos.cfg` supplies 50 and 7, yielding ambient -50 and ramp 7.
This admission is confirmed by isolated original instruction execution and
boundary/missing-value comparisons. Native parsing remains strict; malformed
numeric strings are refused rather than emulating permissive original `atoi`.

## Distance kernels and stamps

`004f1890` builds size-2..17 signed byte cubes. For size `n` and nonnegative
cube coordinates `x,y,z`, using admitted ramp `r`, the recovered value is:

```
p = trunc(sqrt((x*r)^2 + (y*r)^2))
d = trunc(sqrt(p^2 + (z*r)^2))
value = -clamp(d + 127 - r*n, 0, 127)
```

The intermediate truncation matters. The formula describes the distance, but
byte-exact evaluation also preserves the original CRT's scaled calculation at
`0059d646`: take absolute inputs, divide each by their maximum and store the
ratios to double, sum their squared products with x87 intermediate precision and
store that sum to double, then take the square root and rescale with the original
double rounding boundaries. Native evaluation uses explicit double stores and
extended intermediates for this bounded finite-input path. A direct square root
of squared integers was disproved: size 12, ramp 9, coordinates (2,1,11) produce
-119 in the original, while direct square-root evaluation produces -120.
Original math and integer conversions
execute unchanged in the comparison fixture. The native kernels contain 23,408
bytes in total. The selected source index `2*n-1` points to that cube at map
`+637d + 4*index`, with extent byte at `+6404 + index`. Size one has a separate
reserved storage path and is outside this implementation's validated scope.

`004f2140` receives column, row, map layer, and source index. It reflects the
cube into eight octants around the source's paired Z plane, wraps XY and clips Z.
At each reached cell it takes a signed maximum into the working `+7d8` buffer;
for the base `+7dc` buffer it first limits the kernel value to ambient. Overlaps
combine by maximum. The routine contains no geometry/blocker lookup. Source
admission elsewhere in the engine has not been reconstructed.

Native source coordinates must be in bounds, size must be 2..17, and both XY
dimensions must be at least the size. This bounds the selected original routine's
single-wrap coordinate corrections. Unsupported inputs are refused before
changing the field. Native storage samples only after explicit `publish()`,
which owns a copy of the work buffer. The original copy window at
`004f2a42..004f2a62` is checked and executed in the fixture. The preview chooses
this immediate publication explicitly; it does not reproduce the complete
`004f27a0` lifecycle, smoothing, update flags, or player/source selection.

## Preview integration and validation

Use `--terrain-lighting` with `--world --palette-shading` and a packed
`--lighting-config` request. Add repeated `--light-source column,row,layer,size`
options for controlled fixtures. With no sources the field remains -127.
Uniform `--light` and this option are mutually exclusive. JSON records source
requests, effective ambient/ramp, field extent/hash, publication policy, and each
submitted tile's sampled byte. Existing palette-count preferences still apply.

Both complete 65-test suites passed (normal Debug and ASan/UBSan). The new unit
checks cover strict config parsing, negative/clamped admission, independent owned
copies, publication, odd/even layer aliasing, wrapped symmetry, ambient limits,
idempotent weak-source overlap, and invalid inputs preserving state.

Reproduce original comparisons and scene production with:

```bash
./tools/test-terrain-light-fields.py \
  --source-root working/tests/terrain-spatial-source \
  --preview working/tests/terrain-spatial/build/mnm-terrain-preview \
  --sanitized-preview working/tests/terrain-spatial/sanitized/mnm-terrain-preview
```

The 564-fixture field matrix covers every admitted ramp (-127..127), missing
controls, both signed integer extremes, clamping, every supported source size,
minimum dimensions, odd layer counts, wrapped corners, clipped Z, overlapping
sources and 256 reproducibly seeded random cases. It compares native 32-bit
models with original buffers/kernels after every stamp, then compares complete
serialized kernels and five buffers with an independent 64-bit native build.

The successful matrix includes 13,202,112 kernel bytes, 3,594,685 final
five-buffer bytes and 1,695 original source stamps, checked against both native
builds; buffers are also checked after every stamp.

The scene matrix covers installed Celtic, Greek and Medieval MAPs, four
orientations, and empty, centered and corner/overlapping source fixtures in both
builds: 72 frames. Original generated fields are passed directly into the
original world traversal/ordinary producer to compare complete queues, including
per-cell shades. Original indexed drawing then compares the whole output RGB565
frame. Native field hashes and every submitted light byte are checked separately.
Two unshaded compatibility frames and 18 invalid-request refusals also passed.
Results and input/source/helper hashes are in
[terrain-light-fields.json](terrain-light-fields.json).

The runner verifies the original manifest before and after. The profile callback
and return boundaries are private fixture substitutions with expected bytes
checked; original initialization, math, stamp, world producer and draw routines
remain unchanged. Neither original artifacts nor installed executables are patched.
All findings above have high confidence within these bounded paths.

Remaining work includes entity/source discovery and admission, source movement,
geometry-dependent lighting policies elsewhere, player-specific work/base
selection, smoothing/ticking, underwater effects, camera light-bias lifecycle,
size-one sources, smaller-grid wrapping behavior, allocation/reload failures and
live integration. Controlled fixture sources are not installed game entity lights.
