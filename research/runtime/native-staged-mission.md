# Staged native Plains mission

`native-plains-objectives-campaign` is an authored native campaign. It reuses the
sealed 12x12x23 Plains crop, installed type0 wizard/type17 Crocodile bodies,
ordinary health backend, Crocodile ANI attack/damage and shared spell runtime.
It does not reconstruct original mission scripting, rewards, wizard AI or balance.

The version1 definition in `apps/skirmish/plains_mission.json` declares a briefing,
three objective descriptions, initial and reinforcement locations and the Cure
reward. CMake embeds these bytes; the exact JSON participates in the campaign
content revision. Changing content requires rebuilding and produces a different
revision. Unsupported versions, malformed types, unknown fields, invalid text,
unsupported rewards and out-of-map locations refuse. The deliberately finite
objective language is: eliminate initial group, eliminate reinforcement group,
then defeat the rival wizard. There is no general-purpose scripting interpreter.

The coordinator starts one enemy Crocodile guard. Death of that full-generation
actor moves stage0 to stage1 with reinforcements pending. On the next ordinary
admitted tick the entire wave is staged against current occupancy and capacity.
A blocked arrival publishes no actors, RNG or partial group. Normal battle ticks
continue, and a later tick retries. Alternate ticks cannot create a wave. Death
of the wave advances to stage2. The rival wizard may be killed earlier; native
victory is gated until stage2, while player death can still settle defeat/draw.
The mission disables the authored rival summon/bolt loop; the scripted creatures
provide its opponents. Health, damage rolls, mana costs and regeneration retain
their existing values.

The mission's owned CRB4 driver can acquire wizards. It prefers living opposing
creatures, using the recovered selector first and an explicit native nearest
creature fallback outside that selector's admission. Automatic actors retarget
from a wizard when a creature opponent becomes available. Explicit player attack
orders retain their target. Wizard approach/reach uses each body's own rectangle
on the same standing plane; movement still uses recovered ordinary edge legality
and native occupancy/reservations. Attacker ANI hit events use the existing
Crocodile damage roll and authoritative health/death backend. Wizards receive no
creature stun or knockback controller. This composition is native policy, not
original type17-to-type0 melee/defense/stun equivalence.

Victory settles once per adopted saved timeline, marks the first region complete,
and appends Cure exactly once to the player's spell roster. The finale uses its
existing explicit arena profile; campaign commands and the Qt Cure button now
honor the earned loadout. Wrong reward rosters and queued locked spells refuse
transactionally. Reloading an older active save intentionally rewinds its timeline;
there is no global award database or cross-process save-fork deduplication.

The battle and objective state publish together. Active objective checkpoints
retain stage, reinforcement admission and both groups' slot/generation identities.
Restoration validates content identity, group roles/ownership/lifetimes, deaths,
stage order and victory gate. Defeat/retry rebuilds initial actors and objectives
under a new attempt. Existing campaign identities and their wire versions retain
previous behavior and do not migrate to this mission.

Qt exposes **New Plains mission**, an initially paused briefing/objective panel,
stage/reinforcement feedback and the earned reward. CLI creation is
`mnm-map-campaign-cli ASSET_ROOT create-mission OUTPUT.mnmc`. Window startup is
`mnm-skirmish --installation ASSET_ROOT --mission-campaign`.

MISSION01.native-window prospectively binds613 unchanged source keys,
passes117 normal CTests, and matches normal/ASan/UBSan/LSan installed mission
contracts: full-health completion in595 native ticks,
91 per-phase campaign continuations, two fixture wizard melee hits,
blocked/partial wave refusal, early wizard death, defeat/retry, locked/forged
rewards and queued spells, stale handles, codec refusals and actual earned Cure
healing in the finale. Five actual Qt journeys compare full campaign bytes with
fresh CLI processes for new controls/pending summon, attack, wave/victory and
reward/next mission. Three retained pre-change save profiles round-trip unchanged.
All installed input hashes and the2927-file original manifest bookends pass.
Compiler-emitted repository dependencies are entirely declared.
Earlier failed/interrupted development and acceptance attempts remain under
working/ and supply no positive validation. This advances only NP.staged-mission
to native preview; affected original/native shared-source evidence retains its
historical hashes and current freshness limitations.

The acceptance runner is
`tests/test-staged-mission.py`, under the repository storage guard. It separates
normal player-order/full-health completion from explicitly weakened test fixtures
used to exercise exact transition/save boundaries. Full original missions,
arbitrary branching/objectives, additional species, original scheduler/search,
physical sprite/audio integration and live replacement remain outside scope.
