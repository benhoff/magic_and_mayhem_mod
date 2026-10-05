# Global palette lighting controls

The installed `CFG/Encrypted/chaos.cfg` contains the baseline palette settings
in `[GLOBAL_OPTIONS]`. It is a packed CFG container, decoded through the native
[persistence loader](encrypted-cfg.md). The realm recipe CFGs do not
supply these four settings in the installed corpus.

| Key | Installed value | Meaning in the recovered mode-0 builder |
| --- | --- | --- |
| `LightCurve` | 50 | Intensity level, raised to `LightPower` |
| `ColourFactor` | 50 | Saturation level, raised to `ColourPower` |
| `LightPower` | 2.2 | Power applied to the intensity and dark-table shade levels |
| `ColourPower` | 2.2 | Power applied to the saturation level |

The original configuration loader admits integer levels 1..1000 and powers
0..1000. A missing key preserves existing settings, whose image defaults are
levels 1 and powers 2. Native extraction owns optional numeric fields; original
admission is a separate recovered contract. Native numeric input is strict:
finite complete decimal values only, with optional plus signs and trailing
semicolon comments. Invalid values,
integer overflow, nonfinite numbers, and trailing junk are refused rather than
imitating every legacy `atoi`/`atof` lexical case.

Unrelated keys, including `AmbientLight`, `LightRamp`, and underwater controls
(`UWLightCurve`, `UWColourFactor`, `UWLightPower`, `UWColourPower`), remain
outside this selected palette configuration loader. They are not used to
invent spatial lighting or underwater behavior.

`CFG/prefs.cfg` also contains `TerrainLightLevels=256`. The selected preference
read at `0054c269..0054c2de` stores a clamped 2..256 value at receiver offset
0x1d. Its complete dispatch into terrain SPR loading is not reconstructed in
this milestone; configured preview scenes retain their explicit count 16.

Evidence and confidence: confirmed installed CFG contents, hash-pinned binary
string references and configuration instruction windows, executed admission
comparisons, and original palette/scene comparisons are documented in
[terrain lighting configuration](../runtime/terrain-lighting-config.md).
