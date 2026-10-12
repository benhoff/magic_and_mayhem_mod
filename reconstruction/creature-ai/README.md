# Original creature AI reconstruction

`mnm-original-creature-ai` is a native C++17 library containing the recovered
controller, target and world-object selection, hazard/escape/retreat logic,
healing inventory decisions and nine autonomous substates. Its logical state and
world-service interfaces are in `../combat/autonomy.hpp`. It depends on neither
Qt nor Wine, and uses no original executable at runtime.

Keep actors owned by `AutonomousWorld::creatures`; pass that actor by reference
to mutating controller/behavior operations. Supply geometry and consequence
services explicitly. The region graph, world objects, original ordered offset
tables, matrices, diplomacy, visibility, inventory and status fields are inputs.
The library makes its own selection decisions; services perform the requested
movement, aiming, action, health and effect consequences.

Build through the repository's required storage guard:

```sh
python3 tools/working-retention.py run --reserve-gib 0.05 -- \
  cmake -S reconstruction/creature-ai -B working/build/original-creature-ai
python3 tools/working-retention.py run --reserve-gib 0.05 -- \
  cmake --build working/build/original-creature-ai
```

For paired original comparisons, prepare claims for the five `AI.*` behaviors
listed in [the research](../../research/runtime/creature-autonomy.md), then run
`tools/test-creature-autonomy.py --output NEW_OUTPUT --claims CLAIMS` through the
storage guard. The original executable is read-only comparison input. Private
Linux/i386 execution and LeakSanitizer require the appropriate environment.

Full original AI remains outstanding: complete route continuation, periodic
pre-decision/follow work, queues, scenario/strategic policy, ranged aiming,
action/animation execution, effect ownership and application integration are
separate pending contracts. Current source validation passes30,720 original
comparisons across27 bodies, matching native64/reference32/sanitized64 outcome
digests, ASan/UBSan/LSan and the standalone CMake library build. The bounded
approved run retained the default100GiB storage policy.
