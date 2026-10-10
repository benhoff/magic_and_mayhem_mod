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
