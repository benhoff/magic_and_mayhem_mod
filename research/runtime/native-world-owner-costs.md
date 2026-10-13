# Native World owner costs

`MNM_WORLD_OWNER_PROFILE=1` enables bounded per-completed-World CPU wall timing
inside native resident consumption. It measures queue validation, producer
apply-or-skip records by opcode, entry/publication/CPU-knowledge discard, draw
admission, completion, return snapshot and native presentation. Completion also
reports nested identity/catalogue/resource preparation/binding resolution and
scene admission/drawing phases. Nested costs must not be added to their parents.
The outer total includes the renderer batch exit; timer/accounting/loop gaps
remain a residual rather than original simulation CPU attribution.

Disabled diagnostics add no new clock calls or gameplay waits. Existing producer,
prepare and submission observations retain their definitions. Wire contracts,
source FIFO, original returns, producer operations, GPU order, history guards and
simulation pacing are unchanged. These are native CPU wall times including
driver waits, not original-engine CPU or GPU busy times.

`tools/profile-native-world-owner.py` reads a bounded contiguous prefix from a
retained native source corpus, verifies installed input hashes and original
manifests, builds current O2 sources, and uses a private accelerated TITAN display.
Profile-off/on executions compare full native-word hashes for every World.
Diagnostic readbacks occur outside operation timings; checkpointed runs are not
live throughput or presentation benchmarks. First profiling result is pending.

## Initial captured-input result

`run-tljqzhls` compares every full native-word hash for 128 consecutive captured
Worlds with diagnostics off/on on TITAN RTX/Zink; all match. Sources, original
manifests and installed input fingerprints remain stable. In generation 64+ of
this quieter prefix, mean owner total is 7.59119 ms: completion 4.08008 ms,
producer apply-or-skip 1.79986 ms, append 0.79858 ms, entry 0.03585 ms, return
0.00231 ms, presentation 0.04018 ms and validation 0.01166 ms. Residual includes
loop/timer/batch-exit work. Nested completion costs include scene drawing
1.98366 ms, admission 0.20672 ms, identities 0.69477 ms, catalogue 0.44768 ms,
resource preparation 0.03810 ms and binding resolution 0.54423 ms. Producer
copy opcode 6 contributes 1.28398 ms and minimap opcode 22 0.31985 ms.

This is not the busier live path's roughly 13 ms result, and CPU wall includes
driver waits. Work spans several phases; moving native World production away
from the UI presentation owner is the next structural experiment. Keep native
FIFO/two-source credits and immutable GPU ownership rather than dropping updates
or speeding the original clock. Live integration and sustained 60 FPS remain
pending. Changed shared-source historical evidence remains historical.
