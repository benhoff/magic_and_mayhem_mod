# Native skirmish subsystem v1

Intentional native checkpoint format, not an original file structure. The
outer native world snapshot is unchanged. `campaign` contains this byte blob;
`systems` contains the combat blob. Integers are little-endian DWORDs, signed
coordinates use two's complement, identities are `(slot, generation)` pairs.

| Field | Size |
| --- | --- |
| `MNMARM01` | 8 bytes |
| FNV-1a-64 fixture/rules identity | 8 bytes |
| outcome, AI wait, next projectile ID | 3 DWORDs |
| two wizard records | 48 bytes each |
| unit count, unit records | DWORD + 12 bytes each |
| projectile count, projectile records | DWORD + 52 bytes each |
| pending order count, order records | DWORD + 36 bytes each |

Wizard: identity, mana, regeneration counter, spell, remaining windup,
three aim coordinates, optional-target flag and identity. Unit: identity and
role. Projectile: ID, owner, source identity, target identity, origin XYZ,
destination XYZ, remaining flight ticks. Order: kind, subject identity,
optional-target flag/identity, destination XYZ. Absent fields must be zero.

Enums: outcome running/victory/defeat/draw = 0..3; role wizard/Sentinel = 0..1;
spell none/summon/bolt = 0..2; order summon/bolt/move/stop/attack = 0..4.
At most 32 retained actors, 32 projectiles and 64 pending orders are admitted;
blob size is capped at 8192 bytes. IDs cannot wrap. Rules identity hashes DWORDs
`1,20,20,3,8,4,2,4,10,48,24,6,8,6,8,8,3` in that order.

Decode rejects wrong identity/version, truncation, trailing bytes, bad flags and
counts. Semantic restore additionally checks arena/resource binding, ordered
full-generation actor identities, roles, owners, profiles, stationary wizard
positions, mana/phase ranges, target hostility, projectile ordering and terminal
cleanup. Restore builds a complete candidate before replacing active state.
A pending target may have died after admission, and an airborne bolt retains its
source identity after source death; tick rules decide refusal, miss or impact.
