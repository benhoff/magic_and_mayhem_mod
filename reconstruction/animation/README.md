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
