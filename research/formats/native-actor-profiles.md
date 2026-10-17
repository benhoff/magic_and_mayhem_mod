# Native per-actor terrain profiles

These are native checkpoint extensions, not original game save structures.
All integers in the combat/effect codecs remain explicit little-endian DWORDs;
no host pointers or C++ layouts are serialized. Entity.type already belongs to
the versioned native world snapshot and selects an immutable admitted profile.
Unknown types and type/role disagreement refuse restoration.

## MNMTER02

Eight ASCII magic bytes precede canonical compact JSON, bounded to 2 MiB total.
It retains MNMTER01's source MAP/TTD/CFG/ANI SHA-256 values, source path/crop,
standing plane, wizard origins and base64 frozen Crocodile terrain. JSON version
is 2 and wizard_frozen adds a second base64 frozen profile. Its terrain bytes
must exactly equal the Crocodile snapshot except the type0 object type and
selected normalized wizard CFG width, height, acceleration and swimming fields.
The owning driver checks those fields against its embedded raw creature.cfg.

Both profiles bind the same sealed12x12x23 standing-plane world. Type0 admits a
1x1x4 body; type17 admits a2x2x2 body. Wizard movement ANI is not supplied: the
wizard remains under the authored stationary skirmish policy. The second frozen
profile participates in navigation identity. MNMTER01 bytes and identities keep
their previous single-profile meaning.

## CRB3

The existing creature-battle section identity now admits ASCII CRB3 in addition
to CRB1 and CRB2. Its fields follow CRB2: owned melee profile blob, terrain blob,
RNG blob, then bounded sorted actor continuation. CRB3 requires MNMTER02; CRB2
requires the prior terrain profile. The managed melee actors remain type17;
stationary wizard role handles must bind type0. Dimensions are resolved from
owned resources rather than duplicated in mutable actor continuation.

## SHP4

ASCII SHP4 retains SHP3's default creature width/height DWORDs, environment
extent, cell/terrain palette and all projectile continuation. The tag additionally
requires per-actor navigation profiles. Sphere builds aim offsets and collision
bodies from each admitted actor's width/height; default width2/height2 still binds
the Crocodile environment. Downgrading the tag to SHP3 cannot silently change
geometry: the owning environment and navigation policy must match. SHP1..SHP3
keep their earlier global-profile interpretation.

The campaign identity native-plains-species-campaign fingerprints the new initial
world/resources. Legacy campaign identities select their old loaders, including
idle saves. Its arena finale explicitly retains the previous three-layer native
arena profile. No old save is migrated or rewritten automatically.
