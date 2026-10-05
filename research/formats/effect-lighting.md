# Effects configuration lighting fields

The installed packed `CFG/Encrypted/effects.cfg` decodes through the existing
CFG container reader. The original loader requests `cfg\effects.cfg` after
joining the current directory; the native reader explicitly opens the packed
installed path. Those paths identify different stages of installation.

The lighting loader reads exactly `FXA_0` through `FXA_88`, with no zero padding.
Each section supplies these selected fields:

| Key | Original interpretation |
| --- | --- |
| `LightSourceDiameter` | Nonempty profile output: decimal prefix, original 32-bit wrapping, signed clamp to 0..33. Empty/missing: retain prior value. |
| `LightSourceAffected` | ASCII case-insensitive exact `TRUE` sets the stored DWORD to 1. Every other value retains its prior value. |
| `ClippedToHeight` | Same sticky-flag rule. |

Each profile read has capacity 256, including its NUL terminator. The native
asset service owns the selected strings and uses `ProfileSnapshot` for
case-insensitive names, quote removal and truncation. Its existing bounded
ASCII input and duplicate-key/section rejection are native policies; arbitrary
Windows INI parsing is not newly recovered here.

The shipped file has 89 entries. Six have a nonzero diameter: types 3 (4),
13 (6), 21 (10), 22 (10), 24 (5), and 36 (4). Type identity comes from this
effects table, rather than the separate gameplay `objects.cfg` file.

Confidence: high within the selected fields. Whole-loader comparison and the
installed input hashes are recorded in [effect table recovery](../runtime/effect-lighting.md).
Other effect configuration fields and the meaning of the two flags downstream
remain outside this document's recovered scope.
