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
Mini Cancel/resume remain failures. Other difficulties/maps, ranged combat, cast
rejection/refund branches, damage equivalence and complete drawing replacement
are outside this initial journey.
