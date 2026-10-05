# Terrain palette-count preference

Confirmed for the working no-CD Chaos.exe build, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This is offline reconstruction and native preview integration, not live replacement.

## Recovered path

The preference receiver is `006de6c8` (passed to `0054be80` at `004e8fbf`).
The selected read window `0054c269..0054c2de` requests `TerrainLightLevels`
from `VIDEO`, uses the original integer conversion at `0059c5a4`, and stores
an admitted DWORD at receiver `+1d`, global `006de6e5`. `EDI=2` is established
at `0054bf3e`. Present integer values are clamped to 2..256; a missing value
preserves the receiver's existing value. Admission does **not** round to a
power of two. These instructions and strings are confirmed by disassembly
and bounded original execution; confidence is high.

At `0046e6af` the original terrain resource path loads that global and pushes
it as the count argument to the SPR loader `0057d310` at `0046e6c6`, with
receiver `006de558`. Path construction immediately before this appends the
string `terrain` at `005da070`. The same collection receives mode 1, 8 and 9
chains at `0046e6f4`, `0046e71c` and `0046e743`, each using the preference.
Those effect chains are recorded evidence, not implemented native behavior.
The selected mode-0 count argument dispatch is confirmed by original execution.

## Native boundary

`assets/palette_lighting.*` now exposes an owned optional terrain preference,
read from a **plain** CFG. It shares strict numeric parsing with the packed
global palette controls while keeping parsing separate from recovered admission.
`applyTerrainPalettePreference` implements the original clamp and missing-key
behavior; the existing builder continues to support the eight powers of two
from 2 through 256. A present admitted non-power-of-two value is explicitly
refused by the preview. Native malformed/empty/overflowing numeric strings are
refused rather than emulating permissive original `atoi` behavior.

The preview's `--preferences 'CFG\prefs.cfg'` requires `--palette-shading`.
Without this option the previous controlled count-16 fixture remains available.
With the installed prefs file the count is 256. JSON records the effective count
and requested preference path. Global `--lighting-config` remains independently
selectable, and `--light` still supplies a controlled uniform light field.

## Validation

Run the normal and ASan/UBSan builds' CTest suites, then:

```bash
./tools/test-terrain-palette-preferences.py \
  --source-root working/tests/terrain-preference-source \
  --preview working/tests/terrain-preference/build/mnm-terrain-preview \
  --sanitized-preview working/tests/terrain-preference/sanitized/mnm-terrain-preview
```

The runner verifies the executable hash and original manifest before/after.
The fixture executes the selected preference conversion/clamp/store instructions
unchanged. It supplies a private profile callback, checks the bytes before
inserting return boundaries, and substitutes a checked private loader entry
with an argument observer. File loading itself is not exercised in that window.
Separate scene comparisons execute the actual original palette builder and
indexed drawing routines through the existing bounded palette fixture.
No original or installed executable is modified.

Both complete 64-test suites passed (normal Debug and ASan/UBSan).

The admission matrix covers missing values, all integers -2..258 and both signed
32-bit extremes, against all eight supported prior counts: 2,112 admission and
loader-dispatch comparisons, including preserved non-power-of-two admissions.
The scene matrix covers all eight supported counts, four orientations and three
controlled shades, distributed across Celtic/Greek/Medieval generated scenes,
plus the installed preference in all three realms/four views/two shades.
That yields 240 normal/sanitized scene frames compared byte-for-byte with original
RGB565 drawing. Additional cases check missing-key fallback, lower/upper clamps,
and refusal of missing files, unsupported counts, malformed/empty fields and
requests without shading. Results and input/source hashes are in
[terrain-palette-preferences.json](terrain-palette-preferences.json).

Spatial lighting remains outstanding: tile light is currently a controlled
fixture value. The original ordinary producer reads a separate signed byte grid
at `006c5c5c`, indexed using `006cb942` and `006cb8c2`, then subtracts scene
`+b1` and clamps the low end to -127. Reconstructing that grid's initialization
and updates is the next boundary. Palette effect/ordinal selection and live
terrain replacement also remain outstanding.
