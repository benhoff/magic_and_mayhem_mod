# Native world ownership and deterministic checkpoints

Milestone implemented 2026-10-04. This is native policy/synthetic integration
evidence, not recovered whole-engine equivalence or live replacement.

`game/simulation/world` provides an owned, bounded slot table with lifetime
generations, three entity families, explicit target references, cleanup and
release, queued lifecycle commands and staged ticks. `game/persistence` supplies
native-v1 checkpoint encoding/decoding and a POSIX store. The headless
`apps/world-sandbox` runs an idle native world independently of the original
game, Qt and Wine. State restoration includes active/inactive generations,
references, pending commands, counters and caller-supplied subsystem bytes.

## Evidence and confidence

The cleanup/release distinction and non-compacting slots are informed by
[static entity findings](entity-lifetimes.md). The selected coarse tick order,
sequence/counter distinction, decision expansion budget 53 and phase rollover
20/90 are informed by [static tick findings](world-tick-loop.md). Native
generation handles, a unified pool, all-state transactional commits and the
native save schema are intentional implementation choices. No unchanged
original routine is executed in these tests; no timing, complete phase bodies,
death-state, allocator or ABI equivalence is claimed.

The native C++ test covers:

- All three entity families, capacity exhaustion, cleanup versus release,
  incoming-reference repair, stale-handle rejection and retired generation overflow.
- Admission rejection, ordinary/alternate phases, counter wrap, 20/90 phase
  wrap, selected budget initialization and commands visible between phases.
- Callback exception/recursive mutation rollback, malformed commands, storage/
  command limits, queued operation restoration and 200 continued idle ticks.
- Every incomplete checkpoint prefix, corruption, invalid restored references,
  invalid scheduler fields and unchanged state after failed restoration.
- Atomic publication, overwrite refusal, explicit replacement, directory errors,
  staged file restoration and temporary cleanup on publication failure.

The independent Python oracle constructs expected bytes without calling the
native encoder, imports separately encoded input, exercises every single-byte
mutation and incomplete prefix, and recomputes checksums on unsupported-version,
trailing-data, invalid-phase and dangling-reference cases. Fresh processes
compare 200 uninterrupted ticks with 73 + 127 resumed ticks, including a pending
cleanup and sequence wrap. Output bytes agree exactly.

Confidence is high for these bounded native ownership/wire/continuation
contracts. Ordinary Debug CTest passes both tests. ASan/UBSan validation uses
the same tests; LeakSanitizer cannot operate under this environment's tracing,
so leak detection is disabled explicitly and no leak-check result is claimed.
Source/build fingerprints and detailed run results are in the companion
`native-world-foundation.json` report.

Reproduce:

```sh
cmake -S game -B working/build/native-world -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/native-world -j4
ctest --test-dir working/build/native-world --output-on-failure
cmake -S game -B working/build/native-world-sanitized -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build working/build/native-world-sanitized -j4
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  ctest --test-dir working/build/native-world-sanitized --output-on-failure
```

All fixtures are synthetic. No original artifacts are consumed, patched or
launched. These tests do not require immutable-manifest checks because they
operate entirely on generated native data.

## Remaining boundary

This milestone restores a native lifecycle/idle world, not gameplay from a
legacy save. There is no AI, route execution, combat/damage, spell logic,
campaign trigger interpreter, resource recreation, network simulation,
original-compatible writer/importer or live adapter. Campaign/system blobs are
owned storage, not implementations of campaign/script state. The phase callback
contract offers a place for independently validated systems; their mutable
continuation state and schemas must be added explicitly.

Complete original tick ordering, expansion debits/resume selection, live thread/
pause/cadence and original entity cleanup/reference semantics remain open.
Physical crash/power-loss durability, injected I/O fault matrices, cross-platform
publication and real-time performance are unmeasured. Atomic tick rollback
covers world state only, not external callback side effects.

Next bounded step: define owned creature/action state for one selected recovered
behavior, compare its lifecycle against pinned original evidence, and prove its
commands/state survive checkpoints and produce identical continuation. Add
movement/combat/spell/campaign systems incrementally; retain original gameplay
until separate live replacement evidence exists.
