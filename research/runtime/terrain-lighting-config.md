# Installed global palette lighting configuration

This milestone connects the native terrain preview to the four baseline palette
lighting controls in installed `CFG/Encrypted/chaos.cfg`. It uses the native
read-only asset/CFG interface and the recovered mode-0 palette builder. Spatial
per-cell lighting and live replacement are separate milestones.

## Confirmed source and recovered admission

Evidence is for No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence is high within the selected executed instruction window and fixtures.

The configuration window `004e4f75..004e5173` queries `[GLOBAL_OPTIONS]` for
`LightCurve`, `ColourFactor`, `LightPower`, and `ColourPower`. Its string
references are `005e3b9c`, `005e3b8c`, `005e3b80`, `005e3b74`; the section is
at `005e3d80`. `ESI=1` is established at `004e468e` and supplies the lower
integer bound.

| Field | Original admission | Setter | Storage |
| --- | --- | --- | --- |
| LightCurve | clamp integer to 1..1000 | 00582210 | 005f18d0 |
| ColourFactor | clamp integer to 1..1000 | 00582220 | 005f18d4 |
| LightPower | clamp double to 0..1000 | 00582230 | 005f18d8 |
| ColourPower | clamp double to 0..1000 | 00582250 | 005f18e0 |

A missing or empty profile read returns zero and skips that setter, preserving
existing state. Image defaults are levels 1 and powers 2. Present finite powers
at or below zero become positive zero; the native model preserves this detail.
The installed file specifies levels 50 and powers 2.2, with inline comments.
These settings come from global configuration rather than installed realm
recipes. See [on-disk fields](../formats/global-palette-lighting.md).

The selected settings do not determine the complete lighting system. In
particular, `AmbientLight`, `LightRamp`, light-source propagation and underwater
settings have distinct consumers. `TerrainLightLevels` in `CFG/prefs.cfg` is
also separate: `0054c269..0054c2de` admits a 2..256 value into preference offset
0x1d (`EDI=2` at `0054bf3e`). The installed value is 256, but the complete
preference-to-terrain-loader dispatch remains outside this milestone.

## Native implementation

`assets/palette_lighting.*` extracts four optional owned numeric fields through
the existing packed CFG decoder. It trims semicolon comments for these numeric
fields without changing generic CFG storage, which preserves raw value strings.
It does not depend on reconstruction, rendering, or application widgets.

`applyPaletteLighting` in `reconstruction/rendering/palette_shading.*` applies
the recovered admission to supplied state, retaining the table count and missing
fields. Native input deliberately requires complete finite decimal values, with
optional plus signs; malformed values, overflow and trailing junk are refused.
Native present-but-empty numeric values are refused; only absent keys preserve
state. This does not claim all legacy `atoi`/`atof` lexical behavior. Palette construction
still refuses nonfinite powers/results; admission alone is not evidence that
all extreme admitted configurations produce usable tables.

The preview takes an explicit packed installed CFG request:

```bash
mnm-terrain-preview --root working/game-clean --palette-shading \
  --lighting-config 'cfg\encrypted\CHAOS.CFG' --light -17 \
  --output working/configured-terrain
```

The asset resolver handles the Windows separators and case differences. Output
JSON records effective curve/factor/power values and the requested path. The
existing explicit image-default fixture remains available by omitting
`--lighting-config`. Generated/world scenes consume the same optional settings.
Both paths retain count 16 and controlled uniform light; neither reconstructs
spatial illumination or chooses an effect chain.

## Offline validation and reproduction

Build normal and ASan/UBSan previews and run:

```bash
python3 tools/test-terrain-lighting-config.py \
  --preview PATH/TO/NORMAL/mnm-terrain-preview \
  --sanitized-preview PATH/TO/SANITIZED/mnm-terrain-preview
```

`--source-root` accepts frozen source inputs. The runner verifies the executable
hash and the immutable manifest before and after, hashes input/source/helper
artifacts, and records evidence under `working/tests/terrain-lighting-config`.
See [machine-readable evidence](terrain-lighting-config.json).

The configuration oracle maps a private PE copy and runs the original profile
query/conversion/clamp/setter window. A native callback supplies bounded profile
strings; the checked first instruction at `004e5173` becomes a private return
boundary. Original conversion, admission and setter instructions remain
unchanged. The oracle checks the four resulting globals, including exact double
bits. This validates the selected original window, not Windows profile-file
parsing or the complete initialization function.

The palette oracle executes the original builder and indexed drawing routine,
using the same checked private allocator substitution as the previous
[baseline palette milestone](terrain-palette-shading.md). Host-architecture
palette tables are compared independently with original tables as well as the
32-bit reconstruction. Complete scene comparisons consume native recorded
queues; previous producer, generation, geometry, sorting and visibility
validation remain separate evidence.

The fixture matrix covers missing/partial controls, integer limits, powers at
zero and at clamp limits, signed zero, and deterministic mixed settings. Palette
profiles are image defaults (1/1, 2/2), installed controls (50/50, 2.2/2.2),
zero powers with levels 1000, and asymmetric controls (35/70, 1.5/2.5), each at
all eight supported table counts and both RGB565/RGB555 formats. Scene checks
use installed controls in Celtic region 0, Greek region 0 and Medieval region 1,
all four views and eight controlled shades under normal and sanitizer builds.
The completed frozen-input matrix matched 1,068 original configuration
admissions and 4,112,384 palette WORDs across four profiles, eight counts and
both formats, with separate 64-bit host comparisons. All 192 configured
normal/sanitized generated-world frames matched the original indexed draws and
RGBA presentation hashes. Four invalid configuration requests were refused.
Both complete 64-test suites passed. Before and after manifest checks each
verified all 2,927 original files.

Synthetic parser checks cover comments, case-insensitive keys, plus signs,
missing fields, strict numeric errors, nonfinite input and construction refusal.

## Remaining boundaries

Per-cell lighting production, runtime light sources, ambient/ramp consumers,
preference-driven terrain palette counts, underwater/effect chains and their
ordinal dispatch, and live rendering comparison/replacement remain outstanding.
General floating point exceptional paths and legacy numeric-prefix parsing are
unsupported. No original files or installed executable files are changed, and
no gameplay balance values are modified.
