# Owner task event inputs: NS81

NS81 connects ordinary native task admission and replacement progress to regional
membership, special-cell occupancy, published danger/support and health/movement
consequences. It does not add gameplay balance changes or complete the World AI.
All preferred-image addresses identify No-CD Chaos.exe SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168` only.

## Recovered producers and meaning

The old `successes`/`attempts` API names are retained for wire/source compatibility.
Task DWORDs `+2f/+33` are **accounted reward/cost**, not counts of successful or
attempted routes. In particular:

- Whole `514990` calls `416d80` for damage valuation. It resolves each of two
  supplied attacker task IDs through `416c30` and adds the result to task `+2f`,
  doubling only the attacker reward when the victim's configuration kind is
  0/24/25/26. Independently, each victim source at actor `+678/+67c` follows
  source-parent `+39` to a terminal source and adds the undoubled result to its
  task at source `+5`, field `+33`. Duplicate roots produce duplicate credits.
  No health or zero damage returns before any credit. Native arithmetic refuses
  overflow atomically; original DWORD wrap equivalence is outside this scope.
- Positive notifying `5076f0` healing values the actual clamped health gain and
  credits its own terminal task roots at `+2f`. Notify=false skips that credit.
- `512460` values a completion only when actor `+fc` is pending, clears it once,
  and adds its `416d80(..., extra=5e1908/5e1904)` result to terminal task `+33`.
  Ordinary movement is not automatically a task cost. The existing recovered
  segment-completion model owns this branch and original ancestry traversal.
- `509e50` resource contribution, `4181d0` new-sighting notification consequences,
  and other interaction/cast/death producers also add reward/cost. Their call
  sites are retained in the displacement inventory. They are not replaced by
  synthetic route counters or credit for every newly created memory.

Owner actor `+922/+926` are separate published danger/support totals. `41a060`
accumulates manager `+12084` and publishes `+922`; `41d270` accumulates `+120a9`
and publishes `+926`. They match the existing recovered regional producer's
`dangerTotals`/`deductionTotals`, including publication lag. Replacement reads
`danger/(support+1)` and captures its binary32 baseline at task installation.
Incoming task cost lowers progress, earned reward raises it; task expiry alone
still does not set completion. NS80's replacement formula remains unchanged.

Actor `+de7` is regional membership, initialized to -1 in `5065e0` and updated by
regional lookup (`52a780` includes the `52a941` store). It is not a commander
order-assignment flag. `40aa40` requires all source actors to have membership.
Whole `522690` enumerates observation points through `4f3840`, reads the occupant
WORD at tile `+4` and compares it with actor slot `+0`; it does not merely compare
actor position with a point. The native token mapping is slot+1, with ffff empty.

Whole `52a3a0` distress requires wizard kind0/24/25/26, no block7c, health at most
half the supplied maximum, and **strict** danger/(support+1)>0.5. Constant
`5c5a88` is binary64 `000000000000e03f`. Ordinary type17 never passes this wizard
predicate. The native lab's designated commander is an ordinary actor, an existing
explicit native policy rather than a recovered wizard/campaign factory.

`5065e0` initializes task6 percent `+936` to100. Subsequent UI/configuration/save
writers remain distinct from this initialization. Native planner percent supplies
both scoring and nonzero enable admission. Negative original nonzero enable values
remain outside the native percent admission, as before.

## Native composition and continuation

Fresh activation enables event inputs. Each planner slice reads the current
logical commander mapping/enabled owner, published totals, source region and
special-cell occupant tokens. Brain0/root and the ordinary singleton source are
structural boundaries of this lab; no wizard alternative is invented. Preparation
latches eligibility for the sweep in the existing original order.

The encounter loads type17 capacity from Health, `+1a0` from HTH_Matrix and `+280`
from **LRC_Matrix**. Loader `502b20` writes LRC into DWORD indexa0 (byte280),
whereas FearHate is a later matrix. The physical source configuration means the
profile accessors' target-only physical scaling cannot apply. Actual native
health feedback uses the existing recovered `contributionAccount`, not HP lost
as a substitute for its returned score. Melee passes the retained attacker slot
at impact; owner attribution alone supplies no attacker task. Healing credits
only its own task when notifying. Lethal ordinary cleanup's zero-quantity feedback
is preserved; there is no added kill bonus. Flagged completion calls the supplied
accounting service once and charges the current single-root task.

The sole active task remains attached until replacement, including when expired
or completed. Replacing it resets reward/cost and captures a new owner baseline.
Explicit segment contribution roots for that planner actor are refused to avoid
charging the same logical task through two representations. Multiple sources,
multiple brains, delayed projectile recipients and general ancestry remain
outside this adapter's domain.

CMD1v15 appends one bool `eventInputs` to v14 replanning state. v14 reads default
it false and preserve supplied eligibility/progress; older v12/v13 policies stay
separate. Writing v14 with event inputs refuses information loss. Task counters,
owner publication state and in-flight preparation/probes already persist in the
continuation. Resume requires neither installation access nor original addresses.

## Evidence and remaining boundaries

`owner-task-inputs-original-ns81-20261016.json` records4096 prospectively declared
original32/native64/sanitized cases,20480 comparisons. Whole52a3a0 uses a supplied
maximum; whole522690 uses supplied point enumeration; whole514990 executes its
nonlethal/dead/zero credit paths and victim ancestry with supplied account and
source-ID services. It covers wizard doubling, absent recipients, parent and
duplicate victim roots. No text in the target bodies changes. Maximum production,
score numerics and source ID lookup are independently bounded services; this is
not a whole original health-to-planner journey.

`owner-task-inputs-native-ns81-20261016.json` records the separate native journey.
It runs the real grouped features/inference and lifetime draws through a bounded
authored model/region fixture. Actual damage makes an expired task replaceable;
an undamaged control at the same clocks retains its identical proposal. Neither
selected scores, counters nor active lifetimes are assigned to force replacement.
Admission failures, melee/healing/notify credit, one-shot flagged completion,
actual nonzero regional support publication, per-slice restores, old-v14 active
resume and malformed/lossy writes pass405 checks/248 restores in both native and
sanitizer builds. Actual damage contributes34 cost units and the old task spends
102 slices expired before replacement; the control remains attached. Two512-frame
installed-model seeds pass1068 checks/128 restores/34 refusals. Both five-test core
suites, perception/danger regressions, six cold-process resumes and CLI whole/split
equality pass. All274 declared sources and installed inputs remain unchanged;
the original manifest verifies before/after. These are native integration results,
separate from the original component comparison above.

Retained read-only exports under `/var/tmp/magic-and-mayhem-ns81-lab/export-v1`,
`export-v2` and `export-v3` include C, disassembly, references and manifest-bracketed
hash reports; `owner-task-profile-static-ns81-20261016.json` preserves the v3
loader artifact hashes and reviewed LRC mapping. Static displacement leads and the
pinned threshold are in `owner-task-inputs-static-ns81-20261016.json`. Confidence
is high within the executed predicate/credit projection; wider field-writer
classification and producer composition remain partial. Development fixtures and
the superseded first native run are not final validation.

Still pending: sighting observer/notification credit, resource/interaction/cast
credit in the native lab, native production of FC flags and its global score
ratio, observation-point list/campaign construction, general wizard maximum and
configuration writers, source split/merge and dual brains, training/feedback,
whole generation-to-original-driver composition, World cadence and live
replacement. Static links and storage relocation establish none of these.

## NS82 continuation after composed-driver comparison

The bounded [NS82 original driver journey](owner-task6-composed-loop.md) sends
nonlethal514990 credit with supplied scoring34 into an actually generated and
installed task, then compares retention/expiry-relative replacement. It also
found and corrected task6 lifetime to include produced travel+600. NS81's
original predicate/credit results and its native event results remain historical;
NS82 reruns native real health/credit/resume with corrected generated lifetimes.
Notification/resource producers and full original health-to-World scheduling
remain outside these comparisons.

## New-sighting task/service composition — OSC1 (2026-10-23)

The [bounded new-sighting composition](observation-task-services-20261023.md)
now joins new4181d0 observation insertion to nonempty observer source traversal,
terminal-parent task resolution522770 and real416d80 valuation. Prior statements
that sighting task credit is absent remain historical for their earlier scopes.
The new one-owner/one-actor scope uses both families0/1,full mode3 drain and
restoration,health as valuation level,ordinary1/wizard10 quantity caps and
modulo32 task2f accumulation. Refresh/expiry do not credit;reacquisition does.
Preinstalled task/source storage,visibility/activation and profiles are authored
inputs. General task installation/scheduling,resource/interaction/cast credit,
multiple owners/records,World providers and live acceptance remain open.

## Broader concrete task credit services — BTC1 (2026-10-23)

The [new composition](broader-task-services-20261023.md) replaces supplied
416c30 task-ID lookup and416d80 valuation in a separate bounded provider/caller
comparison. Both brain task indexes and522770 terminal source parents resolve
actual owned task records. Activation, defined item healing/resource/flag paths,
nonlethal damage, positive healing, periodic resource contribution and normal
pending completion share those records and modulo32 reward2f/cost33 arithmetic.
The old NS81 logical overflow refusal and historical supplied-endpoint proofs
remain unchanged. UI/registration/item application/world services are explicit
boundaries; this is not complete native lab or original World integration.
General task installation/scheduling,source split/merge,training,death/scenario
cleanup and exceptional/live acceptance remain open.
