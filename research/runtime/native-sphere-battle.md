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
