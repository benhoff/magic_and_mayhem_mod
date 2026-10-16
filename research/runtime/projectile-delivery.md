# Type 3 projectile delivery

Pinned No-CD SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Instruction review identifies spell73, **Magic Sphere**, as the unique compressed
cast-admission arm8, creating update type3. The decoded configuration name is not
Fireball. Other projectile types remain separate contracts. Confidence is high
for the explicit branches below, within the selected input domain.

The complete `4970e0` entry selects `4973f7` for type3. Motion `4883f0(1)` runs
first, then `+1d2` decrements with DWORD wrapping. Result1 resets creator `+44` to
-1 and calls `48ee40(1)` (ricochet). Result0 resolves `+48` to the actor cache,
uses signed configured `6b13c5 + spell*721` magnitude through `48ecf0`, records
`+64 + 1` (not contributor attribution `+212`) and100 on the cached actor, and
sets creator to the hit target. Visible local damage calls `51fba0(1)`. The
hit arm does not run timeout, ANI or audio work and does not release immediately.
Result2 releases; other nonzero results release on signed nonpositive lifetime.
Surviving nonhits restart the animation while tick returns1. Spell73 starts
logical sound710 (`2c6`) through `571310` or follows its existing output reference
through `571460`. Auxiliary `+222` work is excluded.

The selected whole `48ecf0` actor branch calls defended damage `514860` with
signed magnitude, attribution and the two contribution words. A non-sentinel
target with null cache does nothing. Sentinel targets enter terrain/area damage
and are outside this contract. Native invalid graphs are refused explicitly.

`projectile.hpp/.cpp` owns typed actor/effect projections with supplied motion,
ricochet, damage, release, animation and audio services. Callback order and
mutation are part of the contract; callers own transactional staging. This is
not a claim that supplied lower services have been recovered by this comparison.
Existing independently recovered motion creation, trajectory, cell transition,
terrain sampling and creature footprints are available for native composition.
Original ricochet geometry, installed spell/actor parameters, complete type setup,
physical audio, SPR drawing and live original campaign replacement remain separate.

The scope-bound PROJECTILE01 run passes32,768 authored cases on native64,
Linux/i386 original and ASan/UBSan/LSan, plus standalone CMake/CTest. Full guarded
objects, immutable original body/dispatch and ordered calls agree. The reference
uses private FS and hash-guarded lower redirects in an isolated mapped image;
no original file or live process is patched. See
[execution record](original-projectile-bound-20261016.json). Motion, ricochet and
physical backend ownership remain explicit supplied boundaries in this result.

The additional PROJECTILE02 domain composes the real4883f0 movement parent with
trajectory and footprint helpers for type3/spell73. It compares128 authored worlds,
2048 repeated calls and601 creature-hit results across original32/native64 and
sanitizers. Guarded records, cells, actors, terrain, columns and result bytes agree.
Native admission now permits up to64 iterations and an8192 counter so declared
speed20/lifetime100 can continue; this comparison specifically exercises1..20
iterations and16 repeated calls, not every admitted raw state. Actor catalogs stay
fixed within these reference fixtures. Live moving actors are a separate native
integration result, not a composed original scheduling comparison.
