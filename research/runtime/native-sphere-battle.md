# Magic Sphere in the native battle

The projectile-enabled authored arena adds spell73, Magic Sphere. The application
adapter composes recovered cast admission, type3 update and actor impact, trajectory
initialization/stepping, cell/terrain bookkeeping and creature footprint queries.
Game services still own health/mana and accept semantic orders without depending
on reconstruction or Qt. Cure and Sphere share one outer battle transaction.

The declared fixture uses cost8, damage20, speed20 and lifetime100. These values
match the inspected decoded record but are not installed loader/profile bindings.
The owned forward ANI sequence is authored; logical sound start/follow records do
not play physical sound. Coarse creature cells feed collision candidates; a live
fine position feeds footprint offsets when available, otherwise logical grid fine
coordinates are used. A hit updates the last-hit exclusion and can retain flight.
This is a straight trajectory: moving out of its path can evade it.

The arena projectile world is an open8x8x3 torus with empty terrain, distinct from
creature navigation's boundary walls. A terrain-hit service would stop the effect;
it does not implement original48ee40 ricochet geometry. Generation-bound native
allocation, empty contributor chains and manual spatial retirement are adapter
policies, not whole493f20/4943f0 comparison claims. Full type setup, original
collision catalog/profile provenance, actor/status/attribution side effects beyond
the ordinary health adapter, physical audio, SPR rendering and live original
campaign replacement remain pending. No authored Arc Bolt save is reinterpreted.

The new runtime identity is `0x5350484241543031`. Its version2 Cure envelope keeps
all prior Cure state and appends the Sphere continuation. New campaigns bind this
profile. Existing Cure-only and effects-free campaign profiles remain exact load
fallbacks. A checkpoint contains no borrowed pointers or asset dependencies.

## Terrain-bound Plains profile

The optional map Sphere profile owns an 8×8×23 environment projected from the
admitted ordinary MAP/TTD bytes. Each cell retains its catalog occupancy shape
and complete flags DWORD (MAP flags8 plus flags10 shifted16). The existing
native catalog variant of the bounded motion helper receives every distinct
8×8×4 occupancy bitmap (at most4096). The compared four-entry helper remains
a separate wrapper; expanded catalog admission has native validation only. This is a native resource adapter, not
a new original catalog-construction comparison. Blocked-cell flags include the
sealed crop perimeter. A blocked exit leaves the recovered effect detached;
native retirement accepts that exact case and releases its generation-bound
entity. Terrain hits stop the native projectile; original ricochet is excluded.

The immutable environment and dimensions are carried by the new SHP2 section.
On restore, internally consistent but changed geometry is rejected against the
admitted resource context before adoption. Arena continuation still uses SHP1
and its original open torus. Cure presentation dimensions match each mission.
Existing effects-free and Cure-only map identities remain distinct load profiles;
new map creation in the window selects both spells. CLI `create` and `create-cure`
retain their prior profiles; `create-sphere` selects the new one. The player spell
roster and both exact initial snapshots participate in the content revision.

`tools/test-native-sphere-map.py ASSET_ROOT --output NEW_DIRECTORY` declares its
source/contract scope before running normal/sanitized contracts, actual Qt
controls and dialogs, moving-target hits/misses, simultaneous Cure, mid-flight
save/restart, valid forged-environment/profile refusals, win/reopen/advance and
legacy-profile loading. Run under the main storage guard and use checkout-local
TMPDIR; retained reports are separate execution evidence. Optional historical
`--legacy-map`/`--legacy-arena` inputs check exact load/zero-tick byte preservation.

The combined fixture uses canonical creature stats and automatic target
acquisition/path movement with live fine positions. Production creature stats,
spell magnitudes/costs and AI policy remain unchanged.

In the window journey, the moved friendly creature resumes automatic combat
and finishes the hostile creature after its20-point Sphere hit. Event attribution
checks exactly one Sphere hit independently of that later melee damage; the
stationary-friendly core fixture retains the hostile creature at4 health.
