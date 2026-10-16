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

SHP2 (`0x32504853`) binds a mission environment. After its magic it stores width,
height, layers and cell count (DWORDs); count must equal the product, at most4096,
with dimensions4..16 XY and2..32 Z. Next come count terrain-index/flags DWORD
pairs, a DWORD palette count1..4096 and that many32-byte occupancy bitmaps
(each byte encoded as a DWORD0..255). Terrain indices must be below that count. The SHP1 resource counter, scan, actor tracking and item
fields follow, using the declared number of spatial cells instead of192. The
entire Sphere section remains below1MiB. Static cell terrain and all flag bits
except dynamic0x80 must agree with the environment. Active body positions and
trajectory bounds use the declared mission dimensions. The resource adapter
checks complete environment equality during restoration; a structurally valid
but changed environment is not accepted as resource authority.

SHP1 layout and default arena bytes remain unchanged. A legacy effects-free or
Cure-only checkpoint acquires no Sphere state. SHP2 and CBR2 are native owned wire
policies, not claims about the original game's save format or allocator.
