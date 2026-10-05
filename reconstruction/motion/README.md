# Recovered creature motion arithmetic

This library reconstructs a bounded forward-animation slice of pinned No-CD
`0x005104b0`. It is independent of native world policy, Qt and injected hooks.
The caller supplies rate/duration, direction, height origin/delta and a synthetic
twelve-sample animation cycle. It returns completion at 192 progress units before
original environmental/behavior callbacks. It never reads a game pointer.

```sh
cmake -S reconstruction/motion -B working/build/motion
cmake --build working/build/motion
ctest --test-dir working/build/motion --output-on-failure
python3 tests/test-original-creature-motion.py
```

The optional original comparison requires the pinned working PE and i386 compiler.
It verifies the original manifest and executable hash and uses private disposable
memory redirects. Native `move-fine` composes this through an app-owned adapter;
segment/order reset, sample-cycle events and predicted route speed are explicit
bounded policies. Complete original setup, event production, reverse/special
profiles and live behavior remain open. See
[evidence and boundaries](../../research/runtime/native-creature-fine-motion.md).

`segment_setup` additionally models admitted planar category-zero initialization
at `0x00510e80`, using recovered scalar adjustment and deterministic signed
shortest direction change. `python3 tests/test-original-segment-setup.py`
compares the complete original setup entry with controlled eligibility,
occupancy and animation/action dependencies. Same-segment continuation retains
sample and initial sample cursors separately from the animation clock; turns
reset the sample bank and residual snapshot. Native `move-continuous` composes
this through the same app adapter and uses native v4 continuation. This does not
recover the ANI event generator or the original order/occupancy lifecycle.
See [NS04 evidence](../../research/runtime/native-creature-segment-continuity.md).

An optional `MotionAnimation` supplies actual forward controller ticks/restarts.
Event 2 resets sample/residual state and substep count; unsupported events throw.
Callbacks must mutate only caller-staged state, since the arithmetic transaction
cannot undo external callback effects. `move-ani` stages an owned recovered player
and saves it with ANI bytes in native v5. The default supplied-clock contract
remains unchanged. See [NS05](../../research/runtime/native-ani-motion.md).

`segment_setup` now also admits forward category four, sloped XYZ edges and pure
vertical movement with supplied ordinary terrain heights. `ordinary_creature_height`
models the selected coordinate snap; special generator class-two height lookup
is excluded. The expanded original setup runner compares these branches and
ordinary coordinate snaps. The composed ANI runner includes vertical movement
and clamped height deltas. Native `move-terrain`/`move-terrain-ani` resolve heights
from owned frozen map inputs and opt into v6, preserving older driver policies.
See [NS06 evidence and remaining scope](../../research/runtime/native-terrain-motion.md).
