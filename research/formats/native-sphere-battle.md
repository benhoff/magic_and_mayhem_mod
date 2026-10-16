# Native Sphere battle continuation

Inside MNMBAT01 runtime `0x5350484241543031`, the Cure payload magic `CBR2` retains
the exact CBR1 fields, followed by a bounded length-prefixed Sphere byte section.
The old runtime `0x4355524542415431` uses CBR1 and acquires no Sphere behavior.

Sphere section magic is DWORD `0x31504853` (SHP1), all fields little-endian DWORDs
unless embedded in existing envelopes. Storage has fixed counts:32 actor tracking
records,192 spatial cells, eight effect records. The section is smaller than64KiB;
truncation, trailing bytes, unknown version, malformed Boolean/links, invalid live
entity tags/generations, missing roster membership and incompatible ANI state are
refused. No untrusted variable record count allocates native storage.

Fields are resource counter; spatial scan;32 generation/attacker/time triples;
192 head/terrain/flag triples; then eight items. Each item contains optional full
native effect identity, caster/aim handles, optional last-hit exclusion, typed
projectile fields (type, spell, source, target, attacker, attribution, remaining,
logical audio, two contribution words and optional cache ordinal); placement
activity/type,63 parameters, position/fine/startup coordinates,12 sentinels,
next/cell/initialized;14 trajectory words, previous fine XYZ, counter, previous
cell XYZ, terrain ordinal, candidate; startup XYZ; forward ANI PC/display/activity,
delay/elapsed/repeats/break. Optional cache/display uses UINT32_MAX; optional handles
use a Boolean plus slot/generation when present. Fixed ANI data is bound by runtime
identity; no host pointers or health/mana copies are stored.

Native effect body type73/tag `SPH1` plus local byte slot binds the fixed spatial
record. Full native generations prevent an old trajectory from mutating a reused
slot. Removing caster, aim or a referenced collision actor cancels it without a
refund or an extra tick. Terminal battles cancel every remaining effect.

Changing the fixed profile values, sequence or semantic interpretation requires a
new runtime identity and explicit compatibility policy; CBR1/CBR2 identities must
not silently acquire changed rules.
