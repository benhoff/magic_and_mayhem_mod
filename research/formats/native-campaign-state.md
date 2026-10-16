# Native campaign persistence

This is an intentional native data policy, separate from original version-20
`.sav` files and the recovered [campaign rules](../runtime/persistence-progression.md).
It introduces no battle resolution, automatic rewards, stat prices, spell
unlocking or balance changes. The six signed attribute slots and caller-supplied
experience are storage fields; their gameplay meaning belongs to future rules.

`game/persistence/campaign.hpp` owns a typed `CampaignState` and a
`CampaignCheckpoint` pairing it with a complete world. `setCampaign` validates
both before replacing the checkpoint's progress. Invalid input leaves the
supplied checkpoint unchanged. The world's existing opaque `State.campaign`
and `State.systems` bytes retain their original owner; combat/profile consumers
continue to receive those exact bytes. Wizard IDs, realm IDs and region IDs within
each realm must be unique. Region owners, player and battle participants refer
to existing wizard IDs. Spells are unique per wizard; inventory preserves order
and duplicate item IDs. Selection must name an existing realm/region pair.
An active battle must match that selection, include the player, and cannot
coexist with campaign completion. A checkpoint additionally requires its active
battle map ID to match `State.map` exactly.

Campaign identity and content revision are required strings. The revision must
identify the actual authored content/rules used by the caller, ideally by an
external content fingerprint; the codec does not calculate that fingerprint or
load a campaign manifest. Reads require an expected identity/revision and reject
mismatches. Merely accepting the saved revision as the expected value would
defeat this check. IDs are case-sensitive owned byte strings, 1..255 bytes with
no NUL, rather than filesystem paths or host pointers. No text normalization or
UTF-8 validity is asserted.

All integers are explicitly little-endian. Layout:

| Sequence | Encoding |
| --- | --- |
| Signature, version | Eight bytes `MNMCAMP\0`, u32 version 1 |
| Campaign identity | String ID, string content revision |
| Progress | u32 difficulty (0..3), u64 caller transition counter |
| Selection | String player wizard, string realm, string region, u8 completed |
| Wizards | u32 count; each: string ID, u64 experience, six i32 attributes, spell string list, inventory string list |
| Realms | u32 count; each: string ID, u32 region count, region records |
| Region | String ID, u8 owner present, optional string wizard ID, u8 completed |
| Active battle | u8 present; if present: string realm, string region, string map, participant string list |

A string is u32 byte length followed by bytes. A string list is u32 count
followed by strings. Flags admit only 0/1; unknown versions, truncated fields,
excess counts, bad references and trailing bytes refuse. Default limits are
4 MiB of encoded campaign bytes, 80 wizards, 32 realms, 4,096 total regions,
256 entries in each spell/inventory/participant list, and 255 bytes per string.
Battle participants also cannot exceed the wizard limit. Counts and remaining
minimum wire storage are checked before record construction. Limits are explicit
native admission policy, not recovered original storage or enum semantics.

A campaign checkpoint uses an ordinary native world snapshot as a container:
its map tag is exactly `mnm-native-campaign-checkpoint-v1`, its campaign bytes
contain the version-1 campaign block, and its systems bytes contain the complete
encoded battle/world snapshot. All outer counters are zero; outer slots, pending
commands, navigation and animation are empty. Other envelope state refuses.
This composition avoids changing existing world formats or overwriting battle
profiles. Suggested checkpoint extension: `.mnc`. Both layers use existing world
size/storage limits; the complete outer container must fit the configured world
byte budget as well as the inner snapshot. The nested block and world bytes are
covered by the outer corruption checksum. The standalone block has no checksum/authentication;
semantic validation alone does not detect every possible bit change. Existing
snapshot versions/layouts and opaque campaign consumers remain supported:
generic `decodeSnapshot` does not reinterpret existing campaign bytes.

`encodeCampaign`/`decodeCampaign` handle the standalone block.
`encodeCampaignSnapshot`, `decodeCampaignSnapshot` and `readCampaignSnapshot`
validate the full checkpoint and content/map binding. Reads return staged
`CampaignCheckpoint` data only. Callers must still use their
resource-owning session's restore validation/rebinding before committing
movement, animation or combat state. No host resources or original-save records
are reconstructed here. The lifecycle-only test uses `World::restore` because
its world has no navigation/animation backend.

`writeCampaignSnapshot` validates campaign identity and map before delegating to
the existing POSIX snapshot writer. Default overwrite refusal, private temporary
file, file fsync, atomic publication and directory fsync retain that writer's
policy. `durable=false` means publication happened but final durability/cleanup
was not confirmed. It must not be treated as an uncommitted transaction.

Validation uses synthetic two-realm fixtures and explicit authored test events.
The tests cover every truncated prefix, selected malformed inputs, cross-reference
and content/map refusals, transactional updates, exact signed/u64 fields,
independent Python wire decoding, active-battle restart and fresh-process
continuation through region/realm/completion events. File checks cover overwrite
refusal/acceptance, missing parent and corrupt snapshot refusal. Crash/power-loss,
injected fsync/write failures, actual campaign content, UI integration, original
equivalence and resource-backed battle resumption remain pending.

For an existing native session, construct `CampaignCheckpoint{session.world().state(), progress}`
and call `writeCampaignSnapshot(path, checkpoint, expectedIdentity)`. After
`readCampaignSnapshot`, validate/restore `checkpoint.world` through that session's
normal restore path, then adopt `checkpoint.campaign` together with the restored
session. Applications must stage both before replacing either live owner.

Build target `mnm-campaign` links `mnm-world`, with no Qt, original assets,
reconstruction or hook dependencies. Reproduce through
`tools/test-native-campaign-persistence.py` under the storage guard. Coverage ID:
`NP.campaign-persistence`.

## Version-2 attempt and result continuation

The version-1 layout and byte encoding remain unchanged when there is no
session attempt/result data. Version 2 uses the same fields, then adds a u64
attempt immediately after an active battle's participant list. After the battle
branch it adds a u8 last-result-present flag; a present result is u64 attempt,
string realm, string region, u32 outcome (1 victory, 2 defeat, 3 draw).

Attempts are nonzero, no greater than the transition counter. A result must
refer to an existing realm/region. A simultaneous active attempt must be newer
than the last result. The session imposes its additional start/settlement order;
the general codec does not derive rewards or battle outcomes. A decoded
version-1 battle has attempt zero. Version 2 without attempt/result data refuses
as noncanonical. Standalone blocks still rely on the outer snapshot checksum.
See [session ownership and limitations](../runtime/native-campaign-session.md).
