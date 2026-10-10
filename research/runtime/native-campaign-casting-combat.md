# Native campaign casting and combat journey

This extends the public native-menu/native-command smoke with actual gameplay
outcomes. Original simulation and drawing remain active. No health, mana, AI,
creature position, scenario or balance writes are used. The input helper issues
physical XTest input only to the explicitly supplied native viewport rectangle.

## Original observation boundary

The pinned No-CD build is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The opt-in observer guards all six entry bytes at melee resolver `0x0050c0d0`
(`81 ec 1c 01 00 00`) before installing an ordinary forwarding trampoline.
It preserves the original receiver, return value and LastError; only a bounded
trace is written. The hook is absent unless the private combined campaign test
sets its observation path. See [combat research](creature-ai-combat-spells.md)
and [entity lifetimes](entity-lifetimes.md) for the original static evidence.

The creature pool pointer and capacity at `0x006def58/+4` are reread for every
resolution. The observer accepts at most 256 slots, verifies readable storage,
stride `0xe4b` and the slot's own index. It does not presume cross-launch pool
pointer stability or add lifetime generations to original slots. Snapshots are
sampled after original World tick returns at most every 250 ms. Active slots and
their observed transitions to inactive are retained. A 65,536-record bound fails
the reader if exhausted; incomplete final rows and invalid sequences also fail.

`gameplay-events.bin` has a 16-byte `MNMGP001`, version 1, record-size 64 header.
All record fields are little-endian DWORDs. The first four fields are sequence,
kind, original thread ID and Windows millisecond tick. Kind 1 then records slot,
type, owner, active, signed health, tile X/Y/Z, behavior, action, target slot and
fixed-point mana. Kind 2 records source slot/type/owner/health, target
slot/type/owner, health before/after the original melee call, target active
before/after and the original return value. This is passive diagnostic wire data,
not a new native simulation contract or a complete combat/formula reconstruction.

## Outcome requirements and early failures

Successful casting requires a new living player-owned type-14 Zombie after the
native spell HUD selection and right-click, plus independent native framebuffer
and original-owned primary images showing controlled-creature count `0/15` then
`1/15`. This is a creature control count, not mana. The images are unsynchronized
diagnostic samples; agreement of these count readings is not full pixel equality.

Combat requires an original melee resolver return with a player Zombie source,
an opposing active target with positive prior health, and lower health afterward.
The completed journey additionally requires player wizard or combat Zombie melee
to deplete that engaged enemy's positive HP to zero or below. Corpse cleanup
remains outside scope.
An attack click, health change in periodic snapshots, or scripted death alone
cannot pass. In particular, the first Apprentice Redcap is killed by a scenario
Fireball; that event cannot establish player combat.

The retained first [negative run](native-campaign-combat-negative-1-20261010.json)
failed because four-times OCR returned `/15` from both images despite visible
`0/15`. A larger crop resolved it. The second
[negative run](native-campaign-combat-negative-2-20261010.json) created a living
player Zombie and showed `1/15` in both views, but two movement probes after
portrait selection left the original wizard idle. Combat remains pending at this
checkpoint; explicit actor selection and walkable ground probes are being tested.
Historical negative evidence and source hashes remain unchanged.

The existing 20 FPS phase-average and 10 FPS one-second native publication and
visible Qt paint floors remain mandatory, including capture overhead. Stalls over
two seconds, original-window fallback, missing observations or incomplete native
Mini Cancel/resume remain failures. Other maps, ranged combat, cast
rejection/refund branches, damage equivalence and complete drawing replacement
are outside this initial journey.

## Subsequent negative findings and corrections

The [third negative run](native-campaign-combat-negative-3-20261010.json) retained
the same idle Apprentice wizard after explicit scene selection. The normal
highest-difficulty route did subsequently move the wizard, so this does not
establish a general native input failure. Early campaign dialogue/timing gating
is a hypothesis; its precise branch remains unresolved.

The [fourth negative run](native-campaign-combat-negative-4-20261010.json) completed
native Mini Cancel/resume with zero fallback/recovery, but the old validator only
accepted the first Zombie as attacker. The combat summon had reused an original
Redcap slot. The corrected validator observes each cast interval independently
and permits a new living player Zombie after an inactive or type/owner-changed
slot, without inventing original lifetime generations. A still-active player
Zombie cannot count as a new summon. The fourth run also failed unchanged
10 FPS window floors: phase averages were 42.43/25.89 and 32.14/24.46 native/paint
FPS, with worst windows 8.57 and 9.96 FPS. Input and combat observations did not
turn it into a pass.

The [fifth negative run](native-campaign-combat-negative-5-20261010.json) selected
the spell but did not create a Zombie when targeting the wizard's screen
position; mana stayed unchanged. The precise rejection/placement branch is
unresolved. The next route targets clear ground at `(440,310)` instead.

Slow windows around diagnostic screenshots motivated moving PNG compression to
one bounded worker. GL readback and the independent original-owned copy stay on
the UI thread; detached owned QImages are compressed afterward. Metadata is
published atomically only after both saves. Timing includes the remaining
readback and worker CPU overhead. The outcome validator checks image hashes,
original owner identity, observation thread, newly living creatures, and actual
positive-to-lower target HP inside a living player Zombie's original melee call.
An unreadable or identity-changed target after the call retains its prior HP in
the trace and cannot create a fabricated damage event. Twelve synthetic report
tests include reused slots and reject script-only damage, wrong owners, dead
actors, altered images, fallback and truncated logs.

The [sixth negative run](native-campaign-combat-negative-6-20261010.json) verified
both casts and nine original Zombie damage events, including a later `5 → 0` HP
event, then native Mini Cancel/resume with zero fallback/recovery. Strict window
timing still failed. The [four-worker rerun](native-campaign-combat-negative-7-20261010.json)
passed resumed stress at 55.71 native/36.38 paint FPS, worst 19.04 FPS, but repeated
post-hit wizard recentering still had a 9.94 FPS window. Neither is a passing
overall smoke. Those post-hit selections are replaced by normal terrain hover
while observing the fight to enemy HP depletion; recentering stress remains a
known negative branch, not a fixed or excluded historical result.

[Profiles](native-campaign-combat-profile-20261010.json) retain two 10-second,
99 Hz CPU-cycle samples of the private native Qt and Wine game processes with
zero lost samples. The default Mesa worker pool showed many worker threads in
both processes. Explicit `LP_NUM_THREADS=4` bounds the software pools and is
recorded by the runner; it changes no engine setting. Both profiles contain
substantial original clock/busy-wait and Mesa JIT attribution. They cover
different gameplay windows and do not prove a deterministic hardware speedup.
Physical GPU performance and the precise recentering/pacing branch remain open.

## Passing bounded combat journey

The [passing record](native-campaign-combat-passed-20261010.json) preserves the
final executed claims and unchanged source fingerprints. Original manifests
verify all 2,927 files before and after. Native Region Entry selected highest
difficulty through the normal radio, with unchanged rules and private preferences.

The first native spell/right-click creates player Zombie slot 9, with matching
independent native/original `0/15 → 1/15` count readings. A second physical summon
creates combat Zombie slot 10. Fourteen original melee returns reduce opposing
Redcap slot 4 HP, beginning `101 → 100`. Cornelius legitimately finishes the
same enemy `2 → 0` in original melee event 2605, on the same World/menu thread.
The private [raw trace](native-campaign-combat-passed-20261010.bin) is preserved
with SHA-256 `f1540a781c9682dafe496609f58169c7ef9f864551fb05ffe04c0c79dcd4620a`,
matching the executed report. This establishes bounded health depletion after
observed player attacks, not complete damage-formula or corpse-cleanup recovery.

The prior [eighth negative run](native-campaign-combat-negative-8-20261010.json)
also defeated an enemy, but the then-stricter finish criterion incorrectly
demanded the Zombie killing blow and timed out after Cornelius finished it.
The current test still requires Zombie damage and a player killing blow on the
same engaged enemy; a script-only or unrelated death cannot pass. Thirteen
synthetic report tests cover that distinction, including a wizard finishing hit.

| Phase | Seconds | Native FPS | Visible Qt FPS | Worst window FPS |
| --- | ---: | ---: | ---: | ---: |
| Cast and combat | 78.177 | 27.21 | 20.57 | 10.95 |
| Native Mini Cancel/resumed stress | 64.099 | 55.06 | 35.40 | 15.89 |

All unchanged 20 FPS averages, 10 FPS window floors and two-second stall limits
pass. The session completes 5,798 native frames/3,958 visible paints, with zero
recovery or original-window fallback and independent same-thread menu/World
resume evidence. [Native combat image](native-campaign-combat-native-20261010.png)
is diagnostic only; moving creatures/effects are not pixel-equivalence claims.
The private software pools explicitly use four Mesa workers. Repeated post-hit
portrait recentering remains a documented negative performance branch. Apprentice
early movement gating, other maps/difficulties, hardware performance, rejected
casts/refunds, ranged resolution and original drawing/simulation replacement
remain open. The passing result does not refresh older historical evidence.

## Portrait follow-up

The later [portrait rendering fix](native-portrait-rendering.md) restores
repeated recentering during combat and validates sixty seconds afterward.
Bounded presentation damage plus a hash-checked native no-draw-skip policy
passes the unchanged FPS floors and independently compares stable face pixels.
The earlier failures and their source fingerprints remain historical.
