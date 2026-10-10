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
The separate opt-in `spell-events.bin` starts with `MNMCA001`, version1 and
record size64. Rows contain16 little-endian DWORDs; the first four are sequence,
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

Live forwarding validation, failed cast/refund branches, Cure, ranged combat,
other difficulties/maps, normal native-render Quit/save, animation comparison
and scheduling equivalence remain pending. Physical hardware validation is
unavailable here: `/dev/dri` is absent. Native GDI and complete original drawing
replacement remain separate engine milestones.

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
