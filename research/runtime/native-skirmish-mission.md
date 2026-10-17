# Native authored mission geometry

The native simulation accepts a bounded `SkirmishMission` independently of Qt,
installed assets and legacy hooks. It declares a map identity, dimensions,
stationary player/rival spawns, ordered rival summon candidates and canonical
lexicographic standing cells. Each declared standing cell must pass the supplied
navigation's spawn predicate. Dimensions are 4..16 horizontally, 2..32 layers;
standing cells exclude the sealed XY perimeter and layer zero, at most4096.
Wizard spawns differ; rival summon candidates are unique supported cells within
the unchanged two-cell XY summon distance. The complete policy is serialized
for custom missions; the arena remains implicit and byte compatible.

The owning resource adapter must compare the restored policy with its expected
mission. A navigation fingerprint alone cannot authenticate authored spawn/AI
policy. Both arena app adapters enforce this comparison. Session file restore
also requires its current mission policy, and stages failure before publication.
Combat costs, timing, stats, AI interval and reward policy are unchanged.

This is intentional native policy, not recovered original mission behavior.
Original campaign/spawn/AI equivalence and complete installed maps remain outside
this milestone. Tests cover a10x7x4 mission at elevation2, continuation through
summoning, motion and casting, rejected destinations and malformed policies,
canonical versioning and resource mismatch. Evidence is recorded separately in
the central coverage register after execution.
