# Creature admission and batched terrain-light refresh

## Evidence and confidence

Confirmed for the no-CD Chaos.exe with SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The whole unmodified creature updater at **004f27d0** is executed in a private
PE32 mapping with preallocated guarded light buffers, controlled creature
records, and player relation bytes. Its original stamps call **004f2140** on
map object **006c5490**. No installed executable or original input is patched.
Disassembly is supplemented by per-tick native/original comparisons; see the
[reproducible report](terrain-creature-lighting.json).

`tools/test-terrain-creature-lights.py` verifies the original manifest before and
after the experiment and pins the executable hash before execution. It builds
one deterministic fixture as original/native 32-bit, native 64-bit, and native
64-bit with ASan/UBSan. All builds must agree on every buffer byte and cycle
state value. The 96 deterministic fixtures cover 2,304 ticks, 18,398,280 buffer bytes,
and 11,520 cycle state values, all matching across the three builds. The full
terrain/native renderer suite passes all 66 tests in normal and ASan/UBSan
builds. Both manifest checks verify all 2,927 original files.

Native unit checks also cover admission combinations, rejected
inputs without mutation, delayed publication, and independent copies.

## Owned recovered contract

`reconstruction/rendering/terrain_creature_lighting.*` consumes owned creature
records and two 8-by-8 byte tables, advancing the existing owned
`TerrainLightField`. It has no Qt, application-widget, runtime-pointer, Wine,
or game-simulation dependency. This is a build-specific recovered contract,
not a new native gameplay policy. It is not yet driven by captured game objects
or connected to a live updater.

The creature collection starts at **006def58**, has capacity **006def5c**, and
uses stride **0xe4b**. The distinct signed scan bound is **006df180**. Only slots
below capacity can be admitted, even when the scan bound is larger. A record
requires nonzero DWORDs at **+4** and **+e4**; their broader semantic meanings
remain unconfirmed. Position DWORDs are **+8,+c,+10** and owner is **+174**.

Current player is **00644520**. An eligible creature with the same owner is
admitted directly. A different owner requires *both* bytes at
`00643a80 + owner*0x154 + 0x18 + player` and
`00643a80 + owner*0x154 + 0x20 + player` to be nonzero. The tables are directed
from owner to current player; symmetry is not required. Their tests are
confirmed, but labels such as alliance or shared vision are hypotheses and
are deliberately absent from the API. Every admitted creature uses stamp
index **33**, selecting the previously recovered **size-17** kernel.

## Cycle and publication

The map's **+808** enables this updater. When disabled, a valid-player call
leaves cycle state and all buffers unchanged. The original cycle globals are:

| Global | Owned state |
| --- | --- |
| 006e8d90 | phase, 0..7 |
| 006e8d94 | remaining eligible count |
| 006e8d98 | current stamp quota |
| 006e8d9c | next slot cursor |
| map +642e | published flag |

At phase zero, or whenever **005e41a8** is nonzero, the updater copies base
**+7dc** into work **+7d8**, recounts admitted creatures, and resets the cursor.
When remaining is nonzero, the normal quota is unsigned
`(remaining + 7) / (8 - phase)`. Ineligible slots advance the cursor without
consuming quota. An admitted slot stamps its current position and decrements
remaining. The original tests admission again during visitation: moving,
deactivating, or changing owners between ticks can leave the recount stale.
The reconstruction preserves this, including unsigned count wrap if admission
grows mid-cycle; it does not silently normalize the snapshot between ticks.

When changed is **exactly 1**, and remaining is nonzero, quota becomes the entire
remaining count and phase is set to 7 before stamping. Other nonzero values
restart enumeration without forcing completion. Empty recounts do not take
the forced-completion branch. Phase then increments modulo eight.

At wrap to phase zero, work is copied to published **+7cc** if both map flags
**+642a** and **+6426** are zero, setting **+642e=1**. If either is nonzero,
work goes to target **+7d4**, setting **+642e=0**; published remains unchanged.
Prior **+7d0** is untouched by this routine. The semantic names
`transitionActive`/`transitionRequested` describe these publication controls,
not a reconstruction of the separate interpolation routine.

Stamping also raises the base buffer to the ambient-capped source value.
Consequently, restarting work from base does not erase every trace of previous
creature positions. This behavior is retained and compared byte for byte.
The outer updater **004f27a0** calls **004f27d0**, then **004f2ad0**, and only
then clears the shared changed flag. The native caller similarly owns that
flag; the creature cycle does not clear it prematurely.

## Bounded admission and remaining work

The native contract accepts players and active light owners **0..7**, scan
limits/capacities up to 65,536, and the existing size-17 field stamp bounds.
It checks the admitted snapshot before mutating field or cycle state. Invalid
values raise `std::invalid_argument`. Inactive or disabled-light records are
ignored before owner admission, as in the binary.

Spectator player **-1** follows an additional camera-anchor stamp branch at
**004f2aa6**; it is excluded from this milestone. An active other-owner **-1**
can lead the original to a null-based relation lookup and is also excluded.
The original global stamp receiver and updater receiver are the same object
in the fixture; differing receivers are not claimed. Allocation failures,
concurrent mutations inside one call, and original invalid-coordinate behavior
are outside the native contract.

Next work is the separate **004f2ad0** static-object admission and interpolation
path, including helper calls **0049cf60** and **004ff700**, before integrating
captured object records or attempting a live replacement. No gameplay balance
change or replacement of Wine is introduced by this offline milestone.
