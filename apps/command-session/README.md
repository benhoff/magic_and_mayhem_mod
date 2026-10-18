# Native command rule session

This adapter connects the recovered owned move/target/coordinate, queue and
follow/clear context to `game::CreatureRuleSession`. It needs no Wine, PE image,
Qt widget or original assets. Applications may supply staged providers or use `nativeProviders` for owned ANI,
geometry, motion and creature-core frame work. Lower World/health/effect services
remain explicit. The incremental milestones below retain their original scope;
the current native provider profile is described at the end. None establishes
whole original World frame, campaign or live replacement equivalence.

`Runtime` owns an initial canonical actor/execution profile, an exact opaque
profile identity and a provider factory. The native game service owns the public
session, generations, staged tick/command publication and outer checkpoints.
Semantic commands use `CreatureIntent`; queued move/attack/coordinate orders go
through recovered append admission. Clear and follow use their recovered ingress.
Callbacks retain their own supplied/idle/move/target/coordinate binding independently
of later behavior-word edits. Returned events are published only after success.

The adapter publishes actor position/type/owner/target projections to native
entities and saves the complete canonical continuation in `State.systems`.
`CMD1` version2 stores all actor/execution fields, logical order serials, callback
selectors, scheduler/environment, seed, generations and opaque backend bytes, plus complete maintenance and movement roots for recovered bodies.
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

Recovered callbacks can be installed with `Hooks.commandBodies`. The callback
receives the existing operation context; `OwnedCommandBehaviors` borrows its same
world/execution roots and the continuation's maintenance/movement vectors. Move,
target and coordinate callbacks, target maintenance and class-specific ranged
continuation use recovered bodies. Their lower action/geometry/animation/landing/
occupancy/health/removal and frame services remain explicit.

Version2 appends maintenance/movement vectors to the previous schema. Version1
checkpoints decode with absent vectors and can be restored only by profiles
without those roots. A profile using recovered bodies rejects missing vectors.
The integration fixture uses explicit synthetic route and frame services, checks
three command bodies and callback rollback, and compares80 resumed ticks exactly.
It also checks eight provider/context/arithmetic admission refusals and complete state retention.

Lower callbacks must preserve canonical backing and their admitted arithmetic
domain throughout invocation. Unsafe saved movement differences or fine-aim sums
refuse before dispatch; these native policies do not compare original overflow
or malformed-pointer behavior.


`Hooks.actionTransition` and `Hooks.actionTick` install a paired optional action
provider on the same command context. `OwnedCommandActions` supplies recovered
selection and installed ranged/ready callbacks8/9/15; their behavior continuations
use the installed command binding. Once installed, command and recovered body
requests share this selector. Applications advance it with `commands.tickAction`.
Null callbacks are inert. Other action bodies require an explicit provider;
missing bodies, malformed tokens and foreign roots/actors refuse.

ANI events, RNG, profile overlays, cast/form effects and full frame/geometry
remain supplied. The action integration test uses staged synthetic providers,
checks move/target/coordinate selection, two event/cast rollback cases, nine
context refusals and80 byte-identical snapshot-resumed ticks. Its event/RNG/cast
state lives in opaque backend bytes. CMD1/version2 already contains the needed
model state and installed callback token; this milestone changes no wire fields.


Full action profiles use `OwnedCommandActionBodies` for every remaining installed
handler: idle, corpse, motion, both melee variants, object interaction, stun,
interaction, finishing, ascent and descent. The existing ranged/ready layer
handles8/9/15. Callback/action/reset/transition consequences share the same staged
context. Typed ANI/geometry/segment/health/effect services remain explicit.
`Hooks.originalBehaviorBindings` opts into original descriptors1..16, including
null12 and binding without immediate invocation during motion completion.
Applications must provide their matching original behavior bodies.

CMD1/version3 appends all callback/motion/segment/object action roots and this
profile flag. Legacy readers invent no state; required-root profiles refuse old
checkpoints. The native corpus runs all14 handler tokens through24 resumed ticks
per profile, checks melee failure rollback and null/deferred descriptor behavior,
and retains the prior command/behavior/action session corpora. These are native
integration tests, separate from the primitive original body comparisons.

## Native provider profile (NS66)

`nativeProviders` binds immutable decoded ANI, creature scalar tables and terrain
records to the staged continuation. It owns forward/reverse playback, wrapped
geometry, footprint admission, route searches and segment motion. Creature core
passes execute maintenance, eight world ANI channels, the recovered budgeted
scheduler and the environment pass in the observed normal-frame order. The
alternate-frame flag advances only the sequence counter. No wall-clock frequency
is inferred from this ordering.

`prepareNativeContinuation` initializes the fixed actor pool and terrain occupancy;
CMD1/v4 saves animation timing, reverse cursors, original RNG, geometry, open route
searches, segment initialization and environment state. Snapshot resume rejects a
different immutable profile. The lower consequence factory must mutate only its
staged continuation, preserving transaction rollback. Health/effects, region and
owner work, hazards, ranged queries and special profile services remain explicit
inputs. Large attack animation selection and animation-kind1 repair-overlay
profiles are refused until their asset/profile contracts are supplied.

The native provider test accepts an optional installed ANI path through
`MNM_NATIVE_PROVIDER_ANI`; without it CTest uses a controlled ANI fixture. This is
a headless simulation adapter, not a full campaign launcher or a whole original
World frame replacement. Other World/network/audio work and presentation pacing
remain separate.
