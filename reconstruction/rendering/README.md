# NoCD sprite queue model

Build-specific depth-key construction and exact signed-key quicksort permutation,
independent of Qt, asset loading and renderer surfaces. Entries own their keys
and opaque numeric payloads; callers retain ownership of sprite/frame storage.

```bash
cmake -S reconstruction/rendering -B working/build/sprite-queue
cmake --build working/build/sprite-queue
ctest --test-dir working/build/sprite-queue --output-on-failure
python3 tools/test-sprite-queue.py
```

See [recovered contract and evidence](../../research/runtime/sprite-queue-order.md).
The bounded model rejects negative height, invalid views and queues over 65,536
entries. It does not implement original visibility masks, lighting, queue
allocation or world submission policies. Equal keys deliberately retain the
original unstable permutation rather than introducing a stable tie policy.
