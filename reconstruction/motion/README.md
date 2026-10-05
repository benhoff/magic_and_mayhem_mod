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
