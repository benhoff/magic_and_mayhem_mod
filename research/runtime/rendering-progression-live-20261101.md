# Campaign/progression live execution — 2026-11-01

This continuation tests the prepared EP05 drivers against the original No-CD
build `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Each run owns its public Qt launch, display and private Wine prefix. Original
manifest checks bracket acquisitions. Failures and raw observations are retained
in SQLite; no simulation, mission script, health, mana or progression values are
written by these drivers. The preparation commit is `4e4a3d416f61844bf5ba688a6dc5009a36791507`.

## Initial negative results

| Acquisition | Confirmed result | Boundary |
| --- | --- | --- |
| [default-high-01](rendering-progression-results-20261101/progression-default-high-01.json) | Qt stayed idle because the driver omitted public Launch | Manually terminated idle task-owned Qt process; no original game or World execution |
| [default-high-02](rendering-progression-results-20261101/progression-default-high-02.json) | Real launcher refused the copied installation's missing `working/manifests/game-nocd.tsv` | Original executable never started; staging success is insufficient launch evidence |
| [default-high-03](rendering-progression-results-20261101/progression-default-high-03.json) | Full enabled intros, Preferences 800×600, fresh Celtic region 1 World, then natural player death | Failed progression candidate; no victory, original report OK, later region, normal Quit or independent pixel comparison |
| [progression combat diagnostic 04 recovery](rendering-progression-results-20261101/progression-combat-diagnostic-04-recovery-01.json) | Skip intros; two accepted Zombies and two accepted Fireballs, followed by another natural original defeat | Camulos remained at health 100; both Zombies were left behind. Collector lacked a final receipt; recovery is separately labelled |
| [defeat default high 01](rendering-progression-results-20261101/defeat-default-high-01.json) | Independent default-intro defeat attempt stopped at `Menu adapter timed out; using original menus. Requests will not be retried.` | Stage 1, no World; fallback is a refusal, not a natural defeat |
| [defeat combat diagnostic 02](rendering-progression-results-20261101/defeat-combat-diagnostic-02.json) | Separate skip-intro case entered World, then stopped at `Resident World producer cancelled` | Original exit 124, about 21 seconds of gameplay; no natural report or original OK/return |

The [startup diagnosis](rendering-progression-results-20261101/startup-diagnosis-01.json)
records the missing Launch action and its correction. The actual Qt event-loop
fixture now traverses Launch, Preferences, New Game, difficulty and Enter for all
three modes, and refuses disabled Launch/wrong realm. This is synthetic control
validation; it does not simulate a real mission end.

The [manifest diagnosis](rendering-progression-results-20261101/launch-manifest-diagnosis-01.json)
retains the real refusal and successful corrected preflight. Preparation now
materializes and pins the physical TSV required by `tools/run-game.sh`; an
existing malformed manifest or changed pinned executable is refused. Only the
owned worktree's private installation/manifest is affected.

The [combat diagnosis](rendering-progression-results-20261101/combat-diagnosis-01.json)
retains all 21 original report strings. The title was
`Defeat!  ( The Forest of Pain )`, discriminator 0, context/depth 5/5, with
`You were killed.` at text 19. Hero slot 0/type 0/owner 0 ended inactive with
health 0 at `(44,29,4)`; Camulos slot 3/type 24/owner 1 remained alive with
health 100 at `(54,22,4)`. There were 2,579 gameplay rows and zero original player
spell-cast returns; hero mana remained 12,800. The navigation-only strategy
walked the unsupported hero into hostile melee. These are passive original
observations with high confidence for this run, not recovered combat formulas
or a renderer defect. No native fallback/error preceded the rejected outcome.

The failed progression acquisition cannot be relabelled as the separate defeat
case: it rejected the result before original OK/return and normal retirement.
Its raw channel retains the report even though that earlier driver did not put
unexpected report strings in its flow file. Byte-exact intermediate helper,
driver and collector source versions are retained under `phase03-source-*.bin`
and linked from the diagnosis; historical source hashes were not refreshed.

## Corrections and next execution boundary

Unexpected reports are now copied into the flow before outcome rejection, and
the collector reads failed flows before reporting a nonzero public launch.
Input snapshots are saved atomically from helper startup and after decisions,
so an owner's early termination preserves the attempted inputs. A process-level
regression checks this termination case.

The revised ordinary-input victory policy uses the starting Zombie and Fireball
HUD controls, reads fresh positions after centering, and bounds cast attempts.
It counts a summon only after an original successful cast with mana debit and a
new living owned Zombie. Fireball admission requires an original caster debit;
defended damage is a separate observation. The fixed 17/6 mana thresholds and
projection are test policies, not recovered admission or targeting formulas.
The defeat policy never casts. No input attempt alone proves consumption.

[Acquisition inputs 08](rendering-progression-results-20261101/inputs-08.json) pin this source
phase and the real launcher manifest. [Guards 06](rendering-progression-results-20261101/guards-06.json)
record 9 progression checks, 10 pilot checks, 13 normal checks, five actual Qt
control cases, and successful Release player/fixture links. These 37 checks
validate tooling only. Earlier input/guard phases remain historical.

The first combat diagnostic retained six cast attempts: two accepted Zombie
casts (17 mana each), two accepted Fireballs (6 mana each), and two
refused/unobserved attempts. Two defended Fireball hits dealt 20 damage to
Redcaps; neither hit Camulos. The hero ended at health 0/mana 1,024 at
`(47,22,4)`, while Zombies remained alive near `(11,71,4)` and `(32,52,4)`.
Its natural report correctly failed the progression discriminator. This supplies
cast/effect observations and a failed strategy, not successful mission end.
The viewport's terminal close explicitly reports `complete: false` and
`Acceptance capture stored budget exceeded` (1,073,442,088 stored bytes,
5,852,154,000 raw bytes). It cannot supply full pixel history, independently of
the wrong outcome. The interrupted collector's
raw imports are preserved; a distinct recovery receipt records the custody gap
without inventing its lost in-memory assertions. Recovery verifies 291 artifacts
(4,101,798,516 raw bytes), the unchanged prepared sources at recovery time, and
the current player's identity against its retained as-built binary. Six exact
source bodies are retained under `phase04-source-*.bin`. This is no substitute
for the missing original collector completion assertion.

The collector now writes an immutable execution checkpoint before large raw
imports and preserves ordinary artifact-import exceptions in its final failure
report. The checkpoint always marks artifact retention incomplete and success
false; it cannot supply successful acquisition by itself. This final source-only
correction still needs prospective live execution. [Inputs 10](rendering-progression-results-20261101/inputs-10.json)
pin that phase. Inputs 09 retain a mistaken local save-path refusal before any
game execution; the successful retry uses the same original unchanged save.

These executions use preparation commit `4e4a3d416` plus their pinned uncommitted
source phases. Main advanced independently through `4fa166010` and capture-worker
repair `d0bf7a551` / merge `a4d3a6ee3`. Those changes were inspected but are outside
this owned batch and these execution fingerprints. The producer/adapter failures
are not current-master validation, and no cancellation mechanism or renderer
root cause is inferred from their terminal messages alone. Remaining case/mode/
provider executions and independent comparison wait for a successful bounded
route with complete captures; failed acquisitions cannot be admitted by the oracle.
The [actual comparison refusal](rendering-progression-results-20261101/defeat-combat-diagnostic-02-compare-01.json)
stopped at `Incomplete actual public acquisition` before running an oracle.
Its captured engine-source hashes remain historical after the collector edit;
it asserts no current renderer or pixel equivalence.

`--skip-intros` acquisitions and comparisons have explicit diagnostic kinds and
remain ineligible for nominal default acceptance. Default acquisitions keep the
enabled-intro route. Natural defeat/report handling, victory, actual later-map
entry, both frame modes/providers, precise first-frame boundaries, assembled
map/RNG identity and populated object/vptr/generation handoffs remain separate
execution obligations. No artifact5 acceptance or engine status promotion follows
from the negative results above.

The [storage review](rendering-progression-results-20261101/storage-review-01.json)
retains the owned working inventory (about 5.88 GiB allocated and 195.79 GiB main
free at that checkpoint). It identified no cleanup candidates; recent captures,
the admitted prefix fixture and all SQLite payloads remain retained. The later
bounded runs are additional active consumers.
