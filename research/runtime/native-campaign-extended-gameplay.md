# Native gameplay smoke follow-up

The portrait smoke passes its bounded highest-difficulty route. This follow-up
investigates the remaining gameplay cases without changing original rules.
Original drawing and simulation stay active; physical input reaches the native
viewport and passive observations retain the original thread identity.

## Ground calibration and Apprentice

The unchanged [two-probe rerun](native-campaign-idle-probes-20261010.json)
reproduced an idle Apprentice wizard after right clicks at `(540,230)` and
`(550,280)`. Independent native/original images agree. The read-only scenario
`Realms/Celtic/CelticScript1.CFG` limits spell tutorial events28–38 to difficulty0;
this does not establish tutorial gating at difficulty1.

The [extended-probe run](native-campaign-movement-defeat-20261010.json) observed
wizard XY changes `(-1,+3)` and `(-4,0)` after further physical directions
`(-140,+50)` and `(-140,-60)`. Both earlier directions remained idle. Terrain,
selection and command admission are not individually proven rejection causes.
The harness now tries at most eight directions and derives its projection basis
from two independent measured vectors. It preserves all idle attempts and checks
identity, living health, exact sequence intervals and snapshot-derived deltas.
The run reached original player Zombie melee `105→103` enemy HP, but Cornelius
died; this is a failed journey, not a rendering or combat success claim.

## Cast and impact observer

The No-CD executable is SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Original manifests are checked before and after experiments. No immutable file
is changed. Existing `MNMGP001` creature/melee records retain their wire format.
The separate opt-in `spell-events.bin` now starts with `MNMCA002`, version2 and
record size64. The reader also accepts historical V1 (`MNMCA001`) cast/impact rows. Rows contain16 little-endian DWORDs; the first four are sequence,
kind, original thread ID and Windows millisecond tick. Capacity is65,536 records.

Kind1 forwards thiscall/no-stack-argument cast entry `0x57b710`, guarding whole
instructions `6a ff 68 c8 2d 5c 00`. Decoder `0x534000` confirms descriptor spell
at+0, source index+0x1c, target index+0x20 and aim XYZ+0x10/+0x14/+0x18. Remaining
row fields are spell, source index/type/owner, mana before/after, target index,
aim XYZ, opaque original EAX and successful post-call source identity check.
EAX is not treated as admission success: the original routine has no established
Boolean result. Pool pointer/capacity are reread before each identity check.

Kind2 forwards thiscall impact entry `0x48ecf0` with its one DWORD amount argument,
guarding `8b d1 53 55 56`. Its original body calls defended damage `0x514860` when
secondary+0x48 is not-1 and its borrowed target+0x198 exists. Remaining fields
record effect slot, update type, spell ID, target slot/type/owner, HP before/after,
amount, attribution field+0x212, active before/after. Identity changes preserve
prior HP rather than inventing damage. Attribution is recorded without claiming
full projectile/source lineage. Receivers, arguments, return bits and LastError
are preserved; observers do not consume RNG or issue simulation orders.

V2 additionally guards dispatcher `0x48b0f0` (seven entry bytes `6a ff 68 78 02 5c 00`, plain return at `0x48ea28`) and defended damage `0x514860` (five bytes `56 8b 74 24 08`, three DWORD amount/owner/borrowed attribution arguments). Normal same-thread dispatch copies spell+0x4c and living current-pool source+0x44/owner, restoring nested context afterward. Kind3 fields following the common four are target slot/type/owner, HP before/after, active before, amount, damage attribution owner, copied spell, original caller VA, copied source slot and owner. Borrowed attribution is forwarded unchanged and never retained. Exception-unwind context cleanup, complete source lineage and formulas remain pending.

## Refusal and ranged cases

`--spell-cases` requires distant/fogged terrain and insufficient-mana Zombie attempts, over at least1.5seconds with no new living player Zombie or caster mana loss. Same living caster identity and observed summon cost are independently checked. Pre-ingress UI refusal is separate from internal cast admission. Player Fireball71 requires original defended opposing HP reduction inside the copied effect context, with matching source and damage attribution owners. Cure41, refunds, damage formulas, other maps and full animation equivalence remain pending.

[Negative1](native-campaign-spell-cases-negative-1-20261010.json) preserves an invalid rejection point which actually summoned. Distant `(90,90)` subsequently spent no mana or created actor. The assumed green Cure HUD case failed in [negative2](native-campaign-spell-cases-negative-2-20261010.json) and [negative3](native-campaign-spell-cases-negative-3-20261010.json). The third original trace identifies the green player spell as Fireball71 and a6mana debit, not Cure41. Both raw spell traces remain beside the reports. The starting loadout is Fireball71, Zombie14 and Brownie1; healing requires a separate loadout. Fresh complete ranged/forwarding validation is still pending. Native GDI and complete drawing replacement remain separate engine milestones.

## Capture and unexpected exit failures

The [next highest-difficulty run](native-campaign-capture-race-20261010.json)
completed10 Zombie hits and a wizard finishing blow but failed the independent
original resumed image: `FrameStream::nextFrame()` rejected a concurrent seqlock
write. End-phase acquisition now retries on100msUI polls, failing after5seconds.
Diagnostic helper requests retain their10-second deadline. Retries happen before
image saving or route advancement and count toward the ordinary rate windows.
No image requirement, performance threshold or fallback rule is relaxed.

The [portrait follow-up](native-campaign-portrait-defeat-findings-20261010.json)
completed the first enemy's lethal depletion and22 portrait cycles, but later
enemies killed Cornelius. Original World exit events and fresh Main followed.
The old harness waited for gameplay until interrupted; cleanup verified all
original files. Fresh Main, defeat or battle-result admission during this journey
now terminates it as an unexpected-exit failure. Wizard death during combat also
fails promptly. End-to-end validation of these changes is still pending.

The [final-cadence profile](native-portrait-final-profile-20261010.json) retains
6,864 samples at99Hz with zero lost samples from20seconds of the owned native,
Wine game and private desktop processes. Software renderer JIT and original
clock/busy-wait attribution remain substantial. This is an unpaired profile;
it establishes neither a hardware speedup nor scheduling equivalence.

## Fresh bounded spell-case pass

[The V2 source-stable run](native-campaign-spell-cases-passed-20261010.json)
passed both complete native phases at difficulty3. Native/paint FPS averaged
36.04/27.34 during combat and56.97/37.05 resumed; the worst paint windows were
16.80 and20.93FPS. No fallback/recovery occurred and manifests passed before/after.
Two Zombies were summoned with original mana debit. Fireball71 lowered opposing
Redcap HP110→90 at original damage caller `0x48ed24`; its player source and damage
attribution owners agree.10 Zombie melee hits preceded Cornelius's7→0 finish on
that same enemy. Two independent ground vectors were observed. Invalid-target
and insufficient-mana windows lasted2.883 and2.943seconds with zero internal cast
returns, no newly living Zombie or mana loss: these are observed UI refusals,
not recovered internal rejection branches. Raw V2 spell and GP traces remain
beside the report. This run uses ten-second resumed stress; the longer combined
portrait/ranged journey remains a separate validation target. Cure/formulas,
exception cleanup, full lineage and physical GPU equivalence remain pending.

## Combined long portrait/ranged pass

[Fresh combined evidence](native-campaign-fireball-portrait-passed-20261010.json)
passes difficulty3 with60seconds required in each gameplay phase and60seconds
additional portrait stress.22 recenter cycles complete in61.48seconds; ordinary
retreat leaves Cornelius alive at126HP near `(9,68)` after the first enemy fight.
Portrait native/paint averages45.81/33.21FPS, worst16.39FPS; full combat plus
portrait averages41.96/31.00 and resumed58.15/37.97FPS, worst18.38FPS.
10,139native frames and7,118paint frames have zero fallback/recovery.
The independent stable face crop matches within one channel value; both
[retained native](native-campaign-fireball-portrait-native-20261010.png) and
[original](native-campaign-fireball-portrait-original-20261010.png) images remain. Fireball
lowers Redcap104→84;8 Zombie hits precede that Zombie's1→0 finish. Both blocked
cast windows contain no internal cast returns, new Zombie or mana loss. Raw GP
and V2 traces retain their source hashes. This renews only the declared bounded
presentation/cadence and gameplay scopes; original simulation/drawing still run.

## Introductory dialogue and normal Quit

[The Adept failure](native-campaign-adept-dialogue-negative-20261010.json) shows
Hermes's Place of Power dialogue consuming attempted HUD/scene clicks. A
[single early blank heading](native-campaign-late-dialogue-negative-20261010.json)
also fails: original script EVENT3 starts at TimeElapsed100 at difficulties0–2,
EVENT4 follows, and EVENT8 has another delayed Place of Power prompt. This
corrects the earlier broad early-movement hypothesis; an idle click alone does
not distinguish dialogue, terrain or selection. The harness now retains up to
12 native/original heading captures, ordinary dismiss clicks and two quiet
samples after a12second introductory observation interval. Other speakers,
layouts and exact tutorial admission remain separate pending contracts.

[The subsequent Adept attempt](native-campaign-adept-ranged-negative-20261010.json)
clears two Hermes prompts, summons successfully and observes independent movement,
but fixed center Fireball targets produce no player cast/damage. The enemy is
off the guessed center. Selection/targeting are not individually recovered causes.
The helper removes speculative scene reselection after wizard HUD centering,
retains an aiming image and uses bounded orientation-zero tile projection plus
center alternatives. At most two observed mana-debited Fireballs are allowed,
leaving the later Zombie cost available. Exact projection/order equivalence is
excluded. Fireball validation now requires a same-thread identity-valid player
cast/mana debit within10seconds and a reviewed original caller (`0x48ed24` or
`0x48b4e2`); arbitrary/stale-context health loss cannot pass.

[Fresh highest-difficulty normal Quit](native-campaign-normal-quit-passed-20261010.json)
passes the strengthened spell/combat journey and the existing rate floors:
37.54/28.15 native/paint FPS, worst16.30FPS in gameplay;57.996/38.51 resumed,
worst19.86FPS. Original No argument1 on receiver `0x6a5088` resumes World;
repeated Yes argument0 reaches the native campaign defeat report. Its original
Continue callback is local argument21 at `0x4747a0`, followed by native Main
Quit4 at `0x4a75c0`. Original launcher and native shell exit0. The Quit confirmation
is original-rendered through native command presentation; it is not a native Qt
confirmation widget. Both native/original confirmation images and exact menu,
spell and gameplay traces are retained. Modal timer pause equivalence, saves,
other exit causes and newly changed-source portrait validation remain pending.

The existing Ghidra inventory omits the first bytes of Main/confirmation entries;
new links explicitly reuse the recorded eight-byte recovered entry ranges.
[The additional normal-Quit pass](native-campaign-normal-quit-entry-bound-20261010.json)
is prospectively bound to those reviewed range links, while the earlier report
keeps its original declaration. This closes the new Quit accounting gap without
asserting whole-function understanding or refreshing old evidence hashes.

[The revised Adept combat run](native-campaign-adept-party-defeat-20261010.json)
now reaches player Fireball110→90 and Zombie melee after dialogue clearance, but
an original enemy wave kills Cornelius before the required player finishing blow.
This remains a failed gameplay journey; no rendering bug is inferred from defeat.
Cure is available from Brimstone Item1 in a Law talisman according to read-only
`CFG/mitems.cfg`; the starting Neutral assignment supplies Fireball71 instead.
The current campaign adapter does not expose Region Entry loadout editing.
A separate native Quick Battle loadout case is needed for successful healing.

## Native Quick Battle healing route and V3 observation

The separate `--healing-case` route uses native Main → Quick Battle → setup →
second map → Portmanteau. The ordinary magic-item slider supplies the offered
inventory; native spell assignment places Brimstone in Law (Cure41) and Holly in
Chaos (Zombie14). Campaign Region Entry currently cannot edit this loadout.
Physical native-viewport orders seek an injured human wizard and self-targeted
Cure. No helper writes creature health, mana, AI or random state.

V3 (`MNMCA003`, version3, 64-byte rows) adds kind4 at original signed-health
entry `0x5076f0`, guarding six whole bytes `51 a0 58 98 6e 00`. Four DWORD
arguments are delta, attribution owner, borrowed attribution pointer and feedback
flag; the original epilogue at `0x507c1d` returns with `ret 0x10`. All are forwarded
unchanged. Kind4 uses the kind3 target/context fields, recording actual positive
or negative HP change; its delta is signed. Cure's reviewed call returns to
`0x48b5d8` after pushing feedback1, effect attribution+0x216, owner+0x212 and
configured magnitude. The proof requires a living injured human wizard, positive
HP increase, positive delta, exact Cure caller and spell/source/owner/target,
plus a same-thread identity-valid original Cure cast/mana debit within10seconds.
The health row can precede the enclosing post-cast row. V1/V2 readers remain
supported, but cannot admit kind4. No EAX acceptance, cleansing, clamping,
refund/damage formulas, complete lineage or exception-unwind cleanup is claimed.
The same strict20FPS average,10FPS windows and two-second stalls apply to healing;
independent before/after native framebuffer and original publication are required.
The battle itself ends with bounded private cleanup, not a completed Quick Battle
or validated natural defeat/result route.

[The first healing attempt](native-healing-hud-negative-20261010.json) reaches
native gameplay, genuinely summons Zombie14 and injures the human wizard, but
selects the wrong Cure HUD coordinate. The independent image shows the two-item
Quick Battle HUD centered at Zombie523575/Cure573575. The failed source-bound
report and raw V3 spell, gameplay and menu traces remain retained; it cannot
support successful healing. Empty talisman assignments are retained in the native
loadout report; validation requires exactly the two nonempty spell assignments.

[The second healing attempt](native-healing-interval-negative-20261010.json)
actually observes Cure380→400, but still fails: the one-second probe retries
before the delayed Cure effect, then attaches the previous heal to a later,
already-full-health observation interval. The strict interval check rejects it.
The helper now retains the original injured actor's sequence as the transaction
start and waits three seconds for each ordinary target attempt. The proof still
requires a positive original health return and same-thread matching caster debit,
with the health return inside the retained injury-to-recovery interval; a later
full-health cast cannot validate an earlier heal.

[The fresh complete healing run](native-healing-passed-20261010.json) passes
native Quick Battle map2 and the original-offered two-spell loadout. Cure41 raises
the injured human wizard277→377 at original caller `0x48b5d8`, with positive
signed delta100 and matching same-thread player cast/mana debit. Its observed
injury-to-healing interval, raw V3/GP/menu traces and independent before/after
images are retained. Native/paint averages40.81/30.15FPS, worst16.38FPS, with
1,290native publications/918paints and zero fallback/recovery. Original manifests
and stable declared sources pass. This closes successful bounded healing, while
retaining both failed attempts; original rules/drawing remain active and full
battle completion, cleansing/clamp/refund formulas and other exclusions remain.
