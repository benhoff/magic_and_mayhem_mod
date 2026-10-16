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
