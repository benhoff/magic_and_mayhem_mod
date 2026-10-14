# Native command rule session

This adapter connects the recovered owned move/target/coordinate, queue and
follow/clear context to `game::CreatureRuleSession`. It needs no Wine, PE image,
Qt widget or original assets. Applications supply stage-only behavior, geometry,
effect and frame providers; there is no implicit substitute for missing original
World services. The test backend is synthetic and does not establish complete
original frame timing, movement, campaign or live replacement equivalence.

`Runtime` owns an initial canonical actor/execution profile, an exact opaque
profile identity and a provider factory. The native game service owns the public
session, generations, staged tick/command publication and outer checkpoints.
Semantic commands use `CreatureIntent`; queued move/attack/coordinate orders go
through recovered append admission. Clear and follow use their recovered ingress.
Callbacks retain their own supplied/idle/move/target/coordinate binding independently
of later behavior-word edits. Returned events are published only after success.

The adapter publishes actor position/type/owner/target projections to native
entities and saves the complete canonical continuation in `State.systems`.
`CMD1` version1 stores all actor/execution fields, logical order serials, callback
selectors, scheduler/environment, seed, generations and opaque backend bytes.
See [the format](../../research/formats/native-command-continuation.md).
Restore rejects profile/geometry/actor-generation/projection differences, malformed
or excessive data and unsafe queue continuation. Failed commands/ticks/terrain
arithmetic, stale identities/events, recursive queues and topology changes preserve
all previously published state. Clear can recover a refused queue.

The current adapter uses a fixed dense1..128-actor pool. Actor creation/removal,
original-save rebinding, complete original lower services and campaign scheduling
remain separate pending contracts. The model does not read host object layouts
or store pointers/functions in checkpoints. Providers must keep every mutable
continuation in the staged model/backend bytes and avoid external side effects.

Build and test with the repository storage guard:

```sh
python3 tools/working-retention.py run --reserve-gib 1 -- \
  cmake -S apps/command-session -B working/build/native-command-session
python3 tools/working-retention.py run --reserve-gib 1 -- \
  cmake --build working/build/native-command-session --target native-command-session-test -j2
working/build/native-command-session/native-command-session-test
```

The test exercises all five semantic command kinds, queued delivery, invalid
input and exact rollback, decoder and restore failures, three unsafe queue
rollbacks/recovery and80 ticks of byte-identical snapshot-resumed execution.
These are real native session/application operations with explicit synthetic
lower providers, not full original encounter equivalence.
