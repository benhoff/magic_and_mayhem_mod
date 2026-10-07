# Recovery overlapping lifecycle transitions

Recovery now uses the same scoped lifecycle lease as startup and shutdown. Its
existing admission, candidate claim, checkpoint/reset, retry worker launch and
cleanup remain under the shared serialization word. The common cleanup releases
ownership on ordinary refusal and success; candidate storage is still closed
before that release. A contended recovery returns false before opening or claiming
a candidate and emits `command_recovery_lifecycle_busy`, preserving LastError.
Startup and shutdown already reject contention before changing tracked state.
This is intentional native lifecycle policy, not recovered application ordering.

The implementation does not add waits, change candidate identity/session rules,
recycle IDs or make fresh recovery a checkpoint. Ordinary recovery still requires
retired worker storage and quiet independently tracked ownership, discards owned
pixels and requires new complete inputs. Administrative recovery still obtains
its separate exclusive callback gate and validates preserved checkpoints.

A SELFTEST-only one-shot barrier pauses the actual startup, shutdown or recovery
entry after lifecycle ownership and before mutation. It publishes a fixture
rendezvous file and waits for release with a finite5-second fixture budget. The
pause control/export/import and waits are absent from production. Fixture peers
supply no producer pixels, metadata or resource identities.

## Validation

Three schedules each establish two actual same-process sessions with fresh native
consumers and independently generated complete original fixture pixels:

- During held startup on an active session, competing startup, shutdown and
  recovery must all return false. The original session stays active and the
  future candidate is byte-for-byte unchanged. Releasing the owner lets startup
  succeed, then clean shutdown and retry of the same candidate publish fresh
  complete pixels.
- During held shutdown before capture closure, all competing entry points must
  refuse without closing the active session or claiming its future candidate.
  Owner shutdown succeeds after release; the refused candidate can then be
  recovered normally.
- After retiring the old session, held recovery blocks competing startup,
  shutdown and a second recovery candidate. Even an otherwise cached completed
  shutdown cannot bypass an active recovery's lifecycle ownership. The owner
  claims its untouched candidate only after release, establishes a new session
  and publishes independently complete native inputs.

Each schedule checks exact LastError and four transition results `(0,0,0,1)`,
original Lock/Unlock counts, frame hashes, increasing session identities, unique
per-session CREATE/DELETE/END, frozen old channel/archive hashes and terminal zero
consumer resources/readbacks/uploads. Both candidates are checked byte-for-byte
while the owner is paused; the rejected spare remains unchanged and unclaimed at
exit. This verifies real serialized entry points, not merely a manually occupied
lock word. Existing candidate guards, alias continuation and finite session budget
are separately rerun under fresh evidence IDs.

Production and SELFTEST DLL builds pass. The production export table retains only
normal public functions and excludes the pause/guard exports. Historical evidence
and source hashes remain unchanged.

Drawing during direct recovery, original factory execution and borrowed interval
collisions, simultaneous long joins/failed joins, repeated contention at every
recovery stage, abandoned owners, administrative worker shutdown/unloading and
original application/driver/full-frame equivalence remain pending. No default
rendering replacement or gameplay change is claimed.

Exact committed history `e7c56c2..0c80a96` was reviewed against parent/current
file/behavior receipts with no unresolved entries and prior receipt order intact.
This accounting review does not assert past gate passes or new validation of those
historical versions.

Fresh [overlap evidence](opengl-recovery-races-native-20261006.json) passes3
cases/12 complete frames in six fresh sessions. The [recovery matrix](opengl-recovery-races-recovery-20261006.json)
passes10 cases/72 frames; [lifecycle regression](opengl-recovery-races-lifecycle-20261006.json)
passes6 cases/12 frames; [administrative checkpoint regression](opengl-recovery-races-checkpoint-20261006.json)
passes6 cases/186 frames. Intervening Surface2 access-model work changes no
execution fingerprints tested here and is covered by the separate exact history
review, with no inferred validation of that model from these tests.
