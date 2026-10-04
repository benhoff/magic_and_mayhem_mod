# ANI directional selection and phase switches

Scope: selected forward No-CD controller behavior and bounded native preview
controls. This milestone does not replace a live game controller. Findings are
specific to executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Confirmed switch contract

`0x00464e20` is thiscall `(controller, newSequence)`, callee pops four bytes.
It computes next/display record ordinals relative to the old sequence start,
then relocates both pointers to the same ordinals under the new sequence start.
It preserves active, delay, elapsed, repeat count, break flag and reverse fields.
It does not run initial controls or return an animation event. A direction
change therefore immediately selects the corresponding displayed SPR record;
the next tick resumes from the retained next record and timing state.

`NoCdAnimationPlayer::switchSequence` owns the new sequence and preserves that
state using indices. Native safety policy requires an active visible frame,
a terminal stop, valid next/display positions, and a nonnegative sprite record
at the retained display position. Rejected changes leave the player unchanged.
Inactive/null-display pointer arithmetic and out-of-range original switches are
unsafe and are not executed to claim diagnostic equivalence. Reverse mode is
still outside the implementation.

## Selection evidence and remaining interpretation

The validator at `0x005050d0` checks eight-facing groups, invoking the
walk-or-fly shape validator `0x00504cc0` for bases **0** and **176** (the latter
only when at least 184 sequences exist). Its diagnostics require 12 SPR records,
allow intervening events, then event 2, the repeated first sprite and a stop.
This establishes those two movement-shaped groups; it does not alone prove
which is walking versus flying. Group comparison `0x00504ec0` checks paired
facings and non-SPR arguments. ANI records contain no verified action labels.

Creature targeting `0x00508250` computes wrapped target deltas from creature
position, derives a direction, and stores it at creature `+0x608`. The sign
lookup helper `0x004eade0` indexes DWORDs around `0x005e41c4` with `dx+3*dy`:

| dy / dx | -1 | 0 | +1 |
| --- | --- | --- | --- |
| -1 | 7 | 0 | 1 |
| 0 | 6 | 0 | 2 |
| +1 | 5 | 4 | 3 |

The targeting caller returns unchanged for zero displacement. For diagonal
vectors where `2*max(abs(dx),abs(dy)) > 3*min(...)`, it chooses the dominant
cardinal direction instead. This is map-axis ordering, not a verified compass
or projected-screen label.

The animation-facing value at `+0x60c` is `(direction - 2*global[0x6e9850]) & 7`,
with an additional subtraction of four when byte `+0x722` is set. The complete
meaning/lifetime of that global and flag remains unresolved. Native preview
facings are already-selected ANI indices, so it does not emulate this world
selection or assign compass labels.

Selected callers demonstrate separate selection sources:

- `0x00514025..0x00514034` starts controller `+0x7d5` with the configured base
  at global-object `+0x160`, plus creature facing; `+0x164` selects the ANI asset.
- `0x00506c59..0x00506c69` and `0x0050e598..0x0050e5a8` preserve the phase of
  active controller `+0x825` using base **8** plus facing. This is a distinct
  attached/controller path, not proof that base 8 is the next movement action.
- Object callers `0x004948c7..0x004948d9` and `0x0049499b..0x004949b3` add
  facing to separate configuration-derived bases before switching.

Confidence is high for this static arithmetic and selected executed switch
states. A complete named creature action table, configuration provenance for
all bases, attachment selection, camera convention, gameplay events and live
scheduling remain outstanding. Numeric groups in the preview are validated
structural candidates, not asserted gameplay action identities.

## Preview behavior and reproduction

The scene retains owned ANI records. It offers complete aligned groups of eight
whose opcode layouts and non-SPR arguments agree, with valid paired SPR indices.
Changing a group restarts the chosen sequence as an explicit preview policy;
changing facing preserves controller progress. Controls are unavailable for an
initial sequence outside those validated groups. A stopped sequence must be
restarted through group selection before its facing can change.

```bash
python3 tools/test-animation-switch.py
python3 tools/test-sprite-scene.py
working/build/sprite-scene/mnm-sprite-scene-preview \
  --root working/game-clean --ani 'Creatures\redcap.ani' \
  --sequences 0,4 --ticks 32 --loop
# Repeatable exported change: actor 0 switches to facing 7 before tick 7.
working/build/sprite-scene/mnm-sprite-scene-preview \
  --root working/game-clean --ani 'Creatures\redcap.ani' \
  --sequences 0,4 --ticks 32 --facing-change 7,0,7 \
  --export-dir working/tests/new-facing-preview
```

Evidence counts and paths are retained in
[direction-switch evidence](animation-direction-selection.json). Original-input
manifests are verified before/after each experiment, including failures. The
reference privately maps the unchanged PE32 and calls only selected routines;
Win32 loading, Wine and the live game are not used. Native ticks compare raw
event, selected sprite, relative next/display positions and all forward timing
fields. Synthetic switches cover delays, repeat counters, break flags and signed
events; unsafe target rejection is tested only natively.

The scene oracle separately compares original selection/events and independently
decoded SPR composition/presentation. It does not claim an original whole-scene
rendering comparison. All 14 CTests and animation/scene ASan/UBSan checks pass
(leak sanitizer disabled; renderer surface cleanup checked explicitly).

Final scene evidence: `working/tests/sprite-scene/run-gommxxn5/`, retained in
[scene report](animation-direction-scene.json): 195 frames, 390 actor states,
all original selections/events and independent native pixels/presentations
match; 30 distinct SPR frames. Window/timer and output protection also pass.

Final controller evidence: `working/tests/animation-switch/run-mraqfj_r/`: **2,140
traces / 70,620 states**, covering **118 installed ANI assets** and five synthetic
control traces; all match. The retained report records per-asset coverage, source
and input hashes, static artifacts and the full generated report hash.
