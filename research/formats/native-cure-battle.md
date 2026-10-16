# Native Cure battle continuation v1

Cure's `CBR1` payload occupies the MNMBAT01 effect section, with runtime identity
4355524542415431. CombatState owns current/max health and Wizard owns mana;
neither value is serialized in this effect payload. Each rule call derives a
short-lived CureSession actor projection from those authoritative components.
Positive health consequences pass back through CombatSession::heal and must
match the recovered helper result. Mana is translated exactly between whole
arena units and the recovered Q8 resource projection, with one admission debit.

Fields are explicit little-endian DWORDs: magic, low/high playback tick, resource
notification counter,124 learning flags, counted notices,8×8 observations,
counted actor slot/generation/status/ongoing-damage records, selected recipe entry
and counted44-byte ANI records, eight fixed Cure effect records and optional World
handles, then the complete presentation dimensions/scan/cells/records/controllers.
Relative ANI cursor fields match the Cure World continuation; no asset paths,
borrowed pointers, sprite pixels, or wall-clock deadlines are stored.

Limits are4MiB payload,32 actor identities,20 notices,4096 selected ANI records,
192 spatial cells and eight effect slots. Counts are checked before allocation;
fixed presentation counts must match before constructing copied ANI players.
The selected sequence must terminate with STOP; validated controller state is
restored against these owned records. Pool/actor/native identities, effect tags,
spatial links and active/type/controller states are cross-checked. Active effects
must appear exactly once in the shared native roster. Snapshot checksums detect
corruption; this is owned content, not original ANI-file authenticity validation.

Actor records store full generation handles. New summons enroll automatically;
retired records may be reused only when no active effect refers to their index.
Creature removal cancels associated Cure without healing, refund or clock advance.
Native World release handles reference clearing and generation retirement. Death
before effect resolution releases without resurrection or refund. Terminal battle
settlement cancels remaining effects before storing its result.

The asset-free arena profile declares cost3/magnitude8 and a short authored ANI
fixture, separate from original balance/cadence. Tests also supply installed Cure
recipe74 records to the same adapter. Both variants embed the selected definition
and require no asset reopening to resume. Older campaigns retain the exact legacy
content identity and effect-free profile; new campaigns bind the Cure-enabled
initial manifest/snapshot. Whole original update/allocator/follow/auxiliary effects,
negative/special/wizard target Cure, physical audio, SPR drawing and original live
campaign replacement remain outside this format's scope.
