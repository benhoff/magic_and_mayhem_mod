# Terrain palette-count preference

The installed `CFG/prefs.cfg` is a plain configuration file. Its `[VIDEO]`
section contains `TerrainLightLevels=256`. This key selects terrain SPR palette
chain count; see the [build-specific dispatch evidence](../runtime/terrain-palette-preferences.md).
The four global curve/power controls remain in the separate packed
[`CFG/Encrypted/chaos.cfg`](global-palette-lighting.md).

The native selected loader accepts case-insensitive section/key names, decimal
signed integers with optional leading `+`, surrounding whitespace and trailing
`;` comments. Missing keys produce an absent owned value. Empty, malformed or
out-of-range signed integers produce a structured persistence error. Unrelated
preferences are ignored. Recovered admission clamps present integers to 2..256;
the native mode-0 builder admits every count in that range and owns complete
output storage, including odd counts. The selected strict parser admits at most
one sign: repeated signs such as `+-1` are malformed. This is native parsing
policy; original malformed-string conversion is not asserted equivalent. See
the [2026-10-24 review](../runtime/rendering-twentieth-three-20261024.md).
