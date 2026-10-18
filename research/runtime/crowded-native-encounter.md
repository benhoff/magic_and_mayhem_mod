# Crowded native encounters (NS70)

This milestone extends the independent NS69 original/native encounter to four and
eight retained actors, actual shared route search and cached-opponent attacks.
It uses the same selected installed type17 CFG/HTH/ANI and Celtic Plains MAP/TTD
inputs. The native runtime remains headless and independent of Wine/Qt. The full
original-backed campaign still requires its remaining original systems.

## Fixture and comparison boundary

Scenario0 retains the two-actor fixture. Scenarios1/2 use four/eight width2,
height2 actors with distinct hostile owners0..3/7. They add seal flag0x4000 to
columnx5, y1..6, all layers, and give actor0 an initial move request into actor1's
occupied position. Legal nonoverlapping stands are selected deterministically
from the installed12x12x23 crop. These obstacles, start selection and initial
request are common authored inputs, not recovered campaign construction or
balance changes. Crowded input envelopes use MNMENC02 with a scenario word;
MNMENC01 decoding remains available for scenario0.
The common fixture explicitly seeds generator field+589 to1 and field+5ad to0,
matching the native type defaults. Their full installed CFG importer mapping is
not established by this comparison; the original setup is an authored selected
profile rather than the original creature constructor.

Each side owns its actor/ANI/grid/search/scheduler/R250 state after common
initialization. Original509e50,52a780,52a9f0 execute with the declared empty
World/owner/inventory/presentation/effect endpoints. Native32 uses the ordinary
transactional session. Every tick compares the actor whitelist, forward/reverse
ANI state, active motion sample/initial ordinals, valid route points/metadata, health/damage words, all terrain cells,
shared request control words, marker69c and complete250-word random state.
Original search container internals and unused actor-route tails are not part of
this persistent projection. The immediate-route helper separately checks all
sixteen points and the projected131-word transfer, including preserved tails.

Native64 and ASan/UBSan/LSan64 replay the initial portable snapshots and compare
final native bytes and every per-tick digest. Twenty-four fresh checkpoint
processes restore native continuations directly while independently rebuilding
original prefixes; early four-actor checkpoints retain an unfinished shared
search. This is an oracle replay checkpoint, not an original heap/save loader.
An injected native health fault must reproduce at the same tick/field with
identical after artifacts. Corrupt oracle input must refuse before publication.

## Recovered findings and bindings

Confidence is high within the stated domains and pending outside them.

- **516d60/51b130:** validate the cached opponent, query the wrapped destination
  footprint in Y/X/Z order, pause for incoming engagement/moving opponents,
  publish alternate-target action20, or retain a30-tick recent-response timer.
  The helper compares65,536 attack/obstacle cases with declared reach/action/face
  endpoints. Coupled encounters bind real reach, occupancy and action providers.
- **508810:** four footprint-pair25-word tables at5e4f10,5e4f74,5e4fd8,5e503c
  determine facing. Generic angle approximation chooses different diagonal
  opponents' facings. The32,768-case direct comparison includes camera turns,
  reverse-facing publication, all four footprint pairs and out-of-table returns.
  AnimationKind2/3 attachment and block855 repair remain excluded.
- **50bd60/4f4ba0:** the diagonal movement probe runs from the supplied source
  toward the target/its motion endpoint. Reversing these on sloped terrain is
  observable. Its22-byte parameter block is1,1,1,3,0,0,0, independently of the
  attacker's width/height. Initial ordered actor-pair reach and immediate target
  selection compare directly against unchanged original bodies and RNG.
- **5128a0/54b800:** the scheduler now uses the recovered actor wrapper with a
  single shared search, signed53-per-frame and70/210-per-request budgets,
  destination/mode/origin continuation and complete131-word route scratch.
  Neighbor descriptors come from the canonical recovered constructor. Region
  requests are admitted only with the regional graph disabled. Budget yields
  preserve ordered open entries, node records and packed search context.
  The composed scheduler passes request flag1 and exercises the210 allowance;
  the model's70-budget flag0/mode0 branch has no new native integration claim.
- **512800:** immediate autonomous/knockback requests use an independent search
  and the pinned build's250-expansion budget at5e174c. They transfer all131 words,
  publish the route-count marker and clear regionPending without consuming the
  scheduler budget. The336-query helper corpus covers obstructed target cells,
  both request modes and all2/4/8-actor fixtures. It replaces the previous bounded
  one-hop knockback route, whose diagonal vertical metadata differed.
- **Motion samples:** reverse ground motion can cross from the diagonal sample
  bank into the preceding bank. Native cursors index the complete48-word type
  table; retaining only the suffix incorrectly underflowed a host array. Rebase
  these cursors only after a fresh movement animation selects samples. Cached
  attacks, blocked admission and reused segments retain their existing ordinals.
  Active current/initial ordinals are compared against original pointer offsets
  at every tick, independently of position/interpolation outcomes.
- **5093e0 cleanup:** an empty released anchor restores link0x80 when its kind,
  occupancy and auxiliary references are empty, flag0x2000 is clear and the
  object-column query permits it. The admitted native geometry has empty
  auxiliary references/object columns; the full object-column lookup is excluded.

No gameplay balance coefficients change. Friendly displacement/fallback and
region ownership are distinct pending work, even though cached attacks and
hostile occupied-destination replanning now execute together.

## Continuation and reproducibility

[CMD1 version5](../formats/native-command-continuation.md) stores shared request
scratch/metadata and absolute sample ordinals. Mid-search roundtrip, resumed
execution and failed-frame rollback are tested. Quiescent version4 state migrates;
active old searches/motion, lossy legacy writes and malformed request state refuse.
No original save compatibility is asserted.

Run `tools/test-paired-encounter.py` with a new output directory, installed input
root and a prospective declaration from `tools/draft-coverage-claims.py` for the
`crowded-encounter-ns70-20261015` scenario. Pass absolute worktree/tool/input/output
paths when launching through the main checkout's retention guard: the guard runs
its child from the main checkout. Reserve6GiB for three architecture builds.
The runner refuses changed declared/compiler/installed sources, verifies original
manifests before/after, checks the pinned executable hash and guards complete
executable sections against undeclared private-image changes. Original files are
never patched. Reusable build directories are optional; raw results use new paths.

The final source-bound report and coverage ledger record the actual executed
counts. NS69 and earlier report bytes/fingerprints remain historical and unchanged.
Broader shared-source contracts are pending unless explicitly claimed by fresh
execution; a new corpus does not refresh unrelated species/World/trace contracts.
Full World/constructor/campaign, generic region graph, friendly displacement,
dynamic actors, water/flying, object columns and live replacement remain separate.

## Recorded execution

The [source-bound NS70 report](crowded-native-encounter-bound-ns70-20261015.json)
passes9,216 ticks,24 fresh checkpoint processes and164,260 combined helper cases
with221 source fingerprints unchanged. The corpus reaches blocked movement,
replanning, cached attacks, target changes, stun recovery, knockback and retained
deaths/removals. Raw final logs, snapshots and traces are retained under
`working/tests/crowded-encounter-bound-ns70-20261015-v4/`. Development failures,
directional-reach/facing probes and superseded runs remain retained; no passing
result is asserted for those failed or stopped versions. Original executable and
installed inputs pass their before/after/hash guards; native64 and sanitizer
snapshots and per-tick digests agree with native32.
