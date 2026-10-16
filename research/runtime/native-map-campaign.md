# Native Plains crop campaign

This authored campaign runs unchanged native skirmish rules on the existing
ordinary MAP/TTD projection, using the installed Plains `CPsec01.map`,
`Terrain.ttd` and `Terrain.spr`. The crop is (0,0,8,8), all23 source layers,
sealed XY perimeter, supported standing-cell set and synthetic one-cell profile.
Wizard spawns are (1,6,3)/(6,1,3). Rival summon candidates preserve the arena's
XY order at elevation3. The first victory selects an unchanged arena finale;
no extra rewards, health, damage, costs or AI cadence are introduced.

An immutable resource context reads bounded owned bytes through AssetStore,
decodes MAP/TTD/SPR, projects navigation and validates exact geometry/TTD/frozen
fingerprint agreement before creating a battle. Its SHA256 content revision
binds all three complete input files, crop-policy version, canonical campaign,
mission roster and both exact initial snapshots. Restore requires that context,
the expected mission policy, navigation fingerprint and ordinary-health identity.
A failed adoption leaves the previous campaign bytes unchanged. The embedded
mission is continuation data, never authority to select another installed map.

`mnm-map-campaign-cli ASSET_ROOT COMMAND ...` supports the existing campaign
create/start/inspect/cast/resume protocol, plus optional summon Z and semantic
`move INPUT OUTPUT X Y Z` for the first friendly Sentinel. The default arena
CLI retains its original commands and implicit summon Z=1. All output paths
remain exclusive by default. The isolated process journey exercises movement,
casting, split checkpoint continuation, victory/settled reopening and advancement,
plus missing/changed resources and repaired valid-policy/navigation/health
counterexamples. Execution evidence is registered after passing.

This milestone is native policy and headless integration. Full original maps,
original population/campaign entities and original mission behavior are excluded.
Terrain sprite decoding/binding here does not assert original rendered pixels;
interactive terrain presentation is a separate milestone.
