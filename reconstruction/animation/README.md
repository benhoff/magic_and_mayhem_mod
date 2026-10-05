# ANI forward controller model

Selected No-CD forward playback, separate from native asset loading and preview
policies. The player owns a bounded sequence and returns selected SPR indices
and raw events. No Qt, game process or wall-clock rate is needed.

```bash
cmake -S reconstruction/animation -B working/build/animation-model
cmake --build working/build/animation-model --parallel 4
ctest --test-dir working/build/animation-model --output-on-failure
```

Installed/original comparison: `python3 tools/test-animation-contract.py`.
See [contract, addresses, safety policies and evidence](../../research/runtime/animation-forward-contract.md).
Phase-preserving forward switches are implemented and compared separately with
`python3 tools/test-animation-switch.py`; see
[selection evidence](../../research/runtime/animation-direction-selection.md).
Reverse playback, complete named action selection and gameplay event consumers
remain outside this implementation.

Selected sprite/attachment placement is recovered separately in `placement.hpp`:
body displacement, both attachment points and the `TileSizeXY == 2` view
adjustment. The player exposes an owned displayed-record snapshot. Run
`python3 tools/test-animation-placement.py`; see the
[helper contract and evidence](../../research/runtime/animation-placement-attachments.md).

One selected attachment path now has a recovered numeric mode-1 contract:
effect entry 36, admission base-plus-facing selection, nonzero-health visibility,
mode-gated ticks and stop/reset. Reproduce with
`python3 tools/test-animation-attachment.py`; see
[the contract and boundaries](../../research/runtime/animation-mode-one-attachment.md).

`restore(AnimationState)` validates owned PC/display indices and timer fields
against the selected sequence before committing; it does not authenticate prior
control-flow reachability. The native movement adapter stages a local player,
advances it per recovered motion substep, and preserves its continuation in native
v5. Selected forward restart at `0x464d70` is composed with the original action in
`python3 tests/test-original-ani-motion.py`; see [NS05](../../research/runtime/native-ani-motion.md).
