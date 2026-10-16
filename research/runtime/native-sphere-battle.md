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

The Qt journey uses the real Magic Sphere button, enemy picker and save dialogs.
It saves the pending order and first flight phase, pauses, restarts the window,
then advances eight phases. The target moves from its starting cell and receives
exactly one20-point hit (24 to4 health). Fresh-window, headless CLI and an event-
counting process produce byte-identical campaign snapshots. A prior Cure-only
checkpoint still loads and refuses Sphere without modifying the input or creating
an output. The historical Cure playback checkpoint is also inspected unchanged.
The adapter saves its empty contributor-chain sentinels and native attribution
alongside the travelling effect; ordinary health remains the sole damage owner.

The source-bound runner is `tools/test-sphere-window.py`; its `--build-root` can
use separate storage from retained results. The older skirmish window report now
accepts an external build path while preserving relative binary keys for builds
inside the checkout. This affects reporting only.

[PROJECTILE04 execution](sphere-window-bound-20261016.json) binds390 source hashes,
three native behavior contracts and four scenarios. All58 normal CTests, six
ASan/UBSan/LSan contracts, installed Cure ANI checks and both retained Qt journeys
pass. Original manifests and installed inputs match before/after execution.
The native Sphere integration milestone is preview; original comparison and live
replacement remain independent and pending for this whole application adapter.
