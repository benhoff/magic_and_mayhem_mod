# Native terrain-aware creature movement (NS06)

Validated 2026-10-05. This extends the frozen one-creature headless slice with
ordinary terrain heights, sloped edges, pure vertical movement and forward
category-four segment setup. It does not load a complete installed MAP world,
render creatures, replace live gameplay or implement AI/combat/spells.

## Recovered contracts and evidence

Pinned No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

At `0x00511301..0x00511336`, complete segment setup `0x00510e80` computes
height delta as destination grid Z times 16, minus current fine Z, plus the
destination terrain definition's signed DWORD `+0x94` when its cell definition
WORD is nonzero. The current fine Z becomes height origin at `0x00511342`.
Thus `+0x94`, previously used as a classification in navigation, also contributes
the selected ordinary height. These are fields of the resolved runtime terrain
record; this milestone does not establish its complete on-disk/caller mapping.

Category four selects `0x00511a96`: forward animation by facing and the same
direction-parity twelve-sample bank as category zero. The existing scalar helper
uses the actual XYZ edge delta; category four bypasses its slope adjustment.
Pure vertical movement doubles each admitted sample and produces no XY travel.
Matching action/direction/vertical/category retains speed, accumulator, sample
and controller continuation and subtracts 192 from completed progress. Vertical
history has zero XY sample displacement, so this carry does not invent XY travel.
Category or vertical changes select reset setup.

Ordinary coordinate snap `0x005070e0` stores grid XYZ, fine XY at 32 units per
cell, and fine Z at 16 units per layer plus the current terrain record's `+0x94`
when defined. Generator type record `+8 == 2` instead calls `0x004f3260`; that
special height branch is explicitly refused by the new adapter mode.

Executed comparisons:

- **55,296** previous planar setup regression cases and **5,760** new complete
  setup cases agree. New cases cover categories 0/4, eight directions, ascending,
  flat and descending edges, pure vertical up/down, origin/destination offsets
  0/8/16, reset/carry, two accelerations and two prior rates.
- **192** ordinary coordinate snaps agree, including zero definitions and terrain
  heights 0/4/8/16 at three layers and multiple XY positions.
- **576,000** composed original action/ANI start/tick/restart transitions agree
  across categories 0/4, planar/vertical, non-grid height origin 24, height deltas
  -32/-16/0/16/32, eight directions, delays, rates, durations and repeat controls.
- The existing arithmetic/consumption runner passes **960,696** transitions and
  **136** route-consumption/snap regressions.

Confidence is high for these tested isolated setup, scalar, height and forward
controller contracts. Complete setup executes with controlled hazard/eligibility,
accepted category, occupancy, animation selection and action transition helpers;
their full side effects and rejection behavior are not recovered here. Composed
ANI comparison redirects completion and supplies inter-segment continuation,
rather than claiming full original world-loop equivalence. The snap comparison
executes the ordinary original branch unchanged. All three runners verify the
2,927-file immutable manifest before and after, pin the executable and redirect
only private disposable memory. Hash-checked setup/snap assembly exports accompany
the setup report. No installed executable is patched.

## Native integration and explicit policies

`move-terrain MAP OUTPUT SX SY SZ TX TY TZ TICKS` selects continuous setup with
terrain-aware height inputs. `move-terrain-ani MAP ANI BASE OUTPUT ...` also
selects the existing owned forward ANI controller. Prior `move`, `move-fine`,
`move-continuous` and `move-ani` modes retain their byte layouts and pacing.

The app adapter resolves definition WORDs and terrain heights from the exact
owned frozen `MNMWLD01` input, using its row/layer tables. Native simulation sees
only its navigation interface, positions and owned continuation; it does not
interpret terrain record offsets, ANI opcodes or legacy addresses. The new
`finePosition` navigation operation supplies the snapped position outside an
active fine segment. Trace output therefore includes terrain offsets at startup,
segment boundaries and arrival as well as during interpolation. Structural
`inspect-json` remains resource-independent and emits `fine: null` for a
terrain-aware actor without current fine state; `trace FILE 0` resolves it.

Selected native bounds remain one creature, 4,096 frozen cells, forward categories
0/4, adjacent edges, terrain offsets -16..16 and segment height deltas -32..32.
The recovered action clamps interpolated vertical displacement to -16..16;
completion uses the separately validated terrain snap. Completed progress must
remain 192..383, and sample reads must remain within 48 owned values. Unsupported
profiles fail before tick commitment. These bounds are admission policies, not
claims about every original profile. Heights outside the slice can remain in
unvisited map cells; the adapter validates cells it actually resolves.

Directional ANI base remains explicit caller policy. New orders and cleanup
clear history while retaining mode selection. Fine origins come from the frozen
terrain snap, not dynamic environment callbacks. The supplied twelve-frame clock
remains available without ANI; no wall-clock tick rate is introduced.

## Checkpoints and validation

Native **v6** persists terrain selection and the completed segment's grid origin,
including after route-prefix exhaustion. ANI binding is optional in v6 and remains
owned when selected. Restore binds the exact map, checks previous edge/profile/
heights, reconstructs setup, and replays the ongoing segment and controller before
committing. Structural decoding alone does not establish valid restored resources.
See [the wire format](../formats/native-world-snapshot.md).

**32 normal and 32 ASan/UBSan CTests** pass. Leak checking is disabled in the
traced sanitizer environment. The new independent Python oracle compares complete
pending, intra-cell, boundary and carried v6 bytes and resumes a Python-authored
save. **264 fresh-process restarts** compare complete checkpoint bytes and every
remaining trace across terraces, turns, seams, ascending/descending slopes,
vertical up/down (including successive segments), category-four planar movement, fractional speed and sixteen-point
prefix replanning, each with and without ANI.

Checks also cover modified/missing maps, owned ANI source deletion, unsupported
height/special/reverse profiles, recomputed-checksum corruption of heights,
cursors and history origin, invalid mode flags and refusal before publication.
A C++ fixture verifies planning/tick rollback retains queued orders and world
bytes, failed in-place restoration retains resources/state and subsequent behavior,
successful restoration, and mode retention across replacement orders/cleanup.
Existing independent v1-v5 wire and continuation tests remain passing.

Reproduction:

```sh
cmake -S game -B working/build/native-terrain-motion -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/native-terrain-motion -j4
ctest --test-dir working/build/native-terrain-motion --output-on-failure
python3 tests/test-original-segment-setup.py
python3 tests/test-original-ani-motion.py
python3 tests/test-original-creature-motion.py
```

For a synthetic demonstration without original assets:

```sh
python3 tools/create-movement-fixture.py /tmp/terrain-motion.bin --terrain-profile slope
working/build/native-terrain-motion/mnm-world-sandbox move-terrain \
  /tmp/terrain-motion.bin /tmp/climbing.mnw 1 1 1 5 1 2 6
working/build/native-terrain-motion/mnm-world-sandbox trace /tmp/climbing.mnw 18
```

Use unused output paths; the generator and checkpoint writer refuse overwrite.
Machine-readable accepted reports, source/build hashes, commands and logs are in
[native-terrain-motion.json](native-terrain-motion.json).

## Next boundary

Connect this owned movement slice to a bounded terrain-and-creature scene, with
explicit resource/action selection, fine position, facing and depth placement.
Installed MAP navigation/entity admission, original named-action configuration,
special height helper, reverse and other motion categories, gameplay events,
dynamic occupancy, shared scheduling and live agreement remain separate work.
No gameplay balance changes are introduced.
