# Native terrain palette shading: bounded offline reconstruction

This milestone supplies owned baseline palette tables to native indexed sprite
uploads and consumes recovered terrain draw shades. It does not produce a spatial
light field, choose runtime effect chains, or replace rendering in a live game.
The native preview enables it explicitly with `--palette-shading --light VALUE`.

## Confirmed binary contract

Evidence is for the No-CD executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses are virtual addresses in that build. Confidence is high within the
executed fixtures and supported parameter boundary.

- `0057d310` calls `0057d690` with mode 0 at `0057d4e3..0057d4e8`, using
  the caller's requested table count. Mode 0 dispatches to `00582440`.
- The caller at `00505730` requests count 16 for the initial SPR load, then
  appends modes 1, 2, 6, 4, 5 and 7 with the same count. These appended builders
  are distinct effects and are outside this implementation.
- `00582440` allocates `count * 512 - 496` bytes, stores next/shift/neutral
  DWORDs at offsets 0/4/8, and begins 256-entry WORD tables at offset 12.
  For supported power-of-two counts 2 through 256, there are `count - 1`
  tables, neutral index `(count - 1) / 2`, and shift `log2(256 / count)`.
- `0057e1b0`, specifically `0057e1db..0057e20c`, walks the requested chain
  ordinal from the frame's pointer at offset 28 and selects a table by:
  `neutral + SAR(shade, shift)`, adding one when the shifted value is negative
  and shift is nonzero. This includes exact negative multiples: at count 16,
  shade -16 selects neutral, whereas shade -17 selects neutral minus one.
  The native selector checks the resulting index; the original does not clamp it.

## Mode 0 colour construction

Let `h = (count - 1) / 2`. At dark table `t`, for `0 <= t < h`, the integer
shade level is `floor((127*h - 127*t) / h)`. The math wrapper at `0059bcc0`
raises that **level to the configured power**. Define:

```
I = intensityLevel ^ intensityPower
S = saturationLevel ^ saturationPower
F = shadeLevel ^ intensityPower
L = 0.59*green + 0.30*red + 0.11*blue
channel = truncate((C + (L-C) * F/(F+S)) * I/(I+F))
```

The original stores `L` as a double before channel arithmetic. A zero source
channel stays zero, values clamp to 0..255, and the first dark table is forced
black. The native implementation uses extended channel arithmetic and explicit
truncation. The neutral table packs embedded colours unchanged. Bright table
`h+k` uses integer `C + floor((255-C)*k/h)` for each channel; the last is white.
Count 2 has only the neutral table.

Packing is RGB565 or RGB555 according to `006e1f88`. Native drawing remains
RGB565. Constants at `005c7868/70/78` supply blue/red/green weights. Initial
image globals at `005f18d0/d4` contain integer levels 1; doubles at
`005f18d8/e0` contain powers 2. Runtime setters `00582210/20/30` can change
these settings, including through configuration loading near `004e4fe9`,
`004e5062`, and `004e50e8`. The preview deliberately uses explicit levels 1,
powers 2, and count 16; it does not claim to recover each realm's configured
lighting settings.

## Native ownership and integration

`reconstruction/rendering/palette_shading.*` owns tables without PE pointers,
Qt widgets, or renderer state. Its configuration accepts only supported counts,
finite powers in 0..1000, levels 1..1000 with nonzero finite results, and finite
dark-table powers. Other original counts and floating point exceptional paths
are unsupported.

`renderer/sprites/UploadedSpriteFrame` accepts an optional RGB565 table for
indexed storage, uploads its values during construction, and retains no caller
buffer. Coverage remains a separate mask: opaque index zero still writes even
when its colour is black. Direct RGB565 input rejects palette overrides.

`apps/terrain-preview/scene.cpp` constructs baseline chains per embedded
palette, selects a checked table for each draw, and caches uploads by frame
**and table index**. World scenes retain the existing maximum of 16 uploads.
The preview's controlled uniform light feeds the recovered terrain producer,
so its shade is recorded in the queue; it does not stand in for spatial lighting.
Omitting `--palette-shading` retains the established unshaded preview policy.

## Reproduction and evidence

Build `apps/terrain-preview` normally and with ASan/UBSan, then run:

```bash
python3 tools/test-terrain-palette-shading.py \
  --preview PATH/TO/NORMAL/mnm-terrain-preview \
  --sanitized-preview PATH/TO/SANITIZED/mnm-terrain-preview
```

The runner verifies the executable hash and original manifest before and after,
logs inputs and their SHA-256 hashes, and records a report under
`working/tests/terrain-palette-shading/run-*`. `--source-root` supports frozen
source inputs. It compares every installed Forest palette colour at all eight
supported counts and both formats against the original builder, and compares
host-architecture tables separately with the original tables. It then compares
complete 512x256 native canvases and RGBA presentation hashes with original
indexed draws at shade boundaries and in generated scenes in all three realms.

The isolated PE32 helper replaces the checked allocator entry `00597880` with
host allocation **only in its private mapping**. The original builder, power,
conversion and drawing instructions execute unchanged. Raw frame palette
indices are replaced with private owned chain pointers, as the real loader does.
Its frame oracle consumes recorded native queue entries; producer ordering,
visibility, geometry and region generation retain their separate earlier
validation. This adds palette/drawing evidence, not a second independent
validation of the scene producer.

The completed frozen-input run matched 1,028,096 palette WORDs across eight
counts and both formats, with additional 64-bit host table comparisons. It
matched all 400 normal/sanitized native frames (200 fixtures): 256 tile frames
at 32 shade boundaries in four views, and 144 generated-world frames from
Celtic region 0, Greek region 0 and Medieval region 1 at six shades in all four
views. Six invalid controlled-light requests were refused. Both 63-test suites
passed; the final shading/ownership/cache subset passed again in both builds.
The before/after manifest guards each verified all 2,927 original files.

Synthetic checks cover negative exact-multiple correction, endpoint selection,
invalid counts and table indices, palette-buffer destruction after upload,
direct-storage refusal, and one scene reusing a frame at three distinct shades.
See [machine-readable evidence](terrain-palette-shading.json) for the completed
run and [coverage ledger](coverage-ledger.md) for remaining boundaries.

## Remaining boundaries

Spatial per-cell lighting, actual realm lighting configuration, runtime palette
ordinal/effect selection (including kind-dependent dispatch), water and entity
rendering, and live comparison/replacement remain separate work. General
nondefault intensity/saturation settings are configurable in the recovered
model but are not covered by this binary differential matrix. No original files
or installed binaries are patched; no gameplay balance changes are included.

The subsequent [global lighting configuration milestone](terrain-lighting-config.md)
adds installed controls, recovered admission and nondefault palette validation.
The original baseline evidence above remains the explicit image-default fixture.
