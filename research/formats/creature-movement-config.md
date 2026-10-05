# Selected creature movement CFG fields

The installed encrypted `CFG/Encrypted/creature.cfg` uses the existing packed CFG
container. Decoded text has `CREATURE_0` through `CREATURE_27` sections, with title
annotations outside ordinary INI syntax. `CREATURE_10` is labelled REDCAP.

The selected movement fields are TileHeight, TileSizeXY, Acceleration, CanFly,
SwimmingAbility, GroundSpeed and FlyingSpeed. The bounded native reader retains
these seven values only; this is not a complete creature format implementation.
The owning CFG/container parser and its existing validation remain separate from
build-specific runtime conversions and native movement admission.

[NS09](../runtime/native-creature-profile.md) documents the pinned original
loader offsets, conversion evidence, ANI-derived samples and remaining bounds.
