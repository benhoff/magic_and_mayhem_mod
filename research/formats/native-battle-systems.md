# Native battle systems v1

An optional `MNMBAT01` envelope occupies native World's existing `systems` bytes.
Its fields are eight magic bytes, a nonzero little-endian uint64 runtime identity,
a counted combat byte section, a counted list of World slot/generation pairs,
and a counted opaque effect byte section. Counts and lengths are uint32. Combat
is limited to8192 bytes, effects to4MiB and the unique live effect roster to32.
Nested envelopes, zero/duplicate generations, truncation and trailing data refuse.
The outer native snapshot supplies the checksum and atomic file publication.

Unextended combat retains its exact MNMCOM01/02/03 bytes. `decodeCombat` accepts
both forms; only combat owns fighter health, attack and movement controls.
Wizard mana remains in the skirmish `campaign` component. The effect runtime owns
its selected definition/cursors/statuses and effect identities, never copied actor
health or mana. Noncreature entities must appear in the effect roster and have
missile/effect family with no motion driver. They do not reserve creature cells.

The composed skirmish binds an application-provided runtime identity. A missing
or mismatched runtime refuses restore; standalone combat refuses to advance an
owned effect clock. Runtime calls operate on the outer transaction candidate.
Ordinary admission regenerates mana/processes commands, advances movement/melee,
resolves bolts, then advances effects and checks the battle outcome. Alternate
admission advances only World/effect clocks; unadmitted ticks freeze all state.
Terminal battles cancel effects before publication. Runtime failure rolls back the
entire tick, including movement, health, mana, commands and effect allocation.
This scheduling is native policy, not recovered original update cadence.

World retirement reuses its existing release implementation without a second tick:
peer targets/goals clear, generation increments, exhausted slots retire at zero.
Opaque effect sections are preserved through ordinary combat mutations; the bound
runtime validates their semantics at the composed battle boundary. Spell parameters,
original cadence, special targets, complete AI and live campaign replacement remain
separate validation work.

## Optional creature-rule section v2

`MNMBAT02` retains the v1 field order and appends a nonzero little-endian uint64
rule-driver identity and a counted byte payload. The effect identity may be zero
in v2, with empty effect data/roster; the driver is bound independently. Combined
rule/effect data is bounded to4MiB. Raw combat and v1 envelopes remain byte-exact
when no driver is bound. A missing or mismatched driver refuses restore.

The native driver controls decision and resolution phases around exactly one
MovementSession admission. Its continuation is staged before final native and
application validation. Failure preserves the previous combat, rules, effects,
health, mana and queued orders. The legacy authored melee path remains the exact
fallback for unbound sessions. Alternate admission freezes creature-rule clocks.

The installed Crocodile application driver uses identity `0x4352424154543031`.
Its payload begins with DWORD `0x31425243` (`CRB1`), counted installed `MNMMPR01`
profile bytes (max2MiB), counted1008-byte RNG state, and a count of at most30
ordered actors. Each record stores slot/generation, home XYZ, facing, cooldown,
recent age, signed stun timer, phase0..7, optional full-generation target and
recent-attacker handles, and a counted ANI cursor (max28 bytes). Optional handles
are a0/1 flag followed by slot/generation, zero when absent. All words/counts are
little-endian32-bit. Health/mana and native movement are never copied here.
Installed decoding validates the profile and cursor reachability; shared battle
validation checks health/death, actor/target/control/generation, RNG layout and
living2x2 footprint admission. Newly summoned actors enroll on their first
ordinary rule tick; loading does not invent continuation or rewrite bytes.

For a bound creature driver, CombatState retains authoritative health and semantic
movement/control compatibility. The rules payload owns timed attack/stun state;
CombatAction mirrors a live target as approaching with no native phase timer.
Stored damage1/windup2/recovery3 are unused compatibility placeholders: installed
HTH plus recovered RNG and ANI events determine attacks. Missing-driver refusal
prevents this checkpoint from silently using authored melee with those values.
