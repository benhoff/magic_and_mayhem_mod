# Native fixed-rate scene playback (NS16)

`NP.scene-playback` is intentional native presentation policy. It does not recover
original pause/admission/scheduling (`MV.pause-clock`), change gameplay balance or
replace live original work.

## Admission and boundaries

The Qt-free `game::TickClock` admits native simulation ticks at **10 Hz**: one
100 ms interval, at most four ticks per poll. A caller supplies monotonic unsigned
millisecond timestamps. Fractional elapsed time is retained while playing. When
more than four intervals are due, excess whole intervals are discarded rather
than deferred; no simulation ticks are skipped in world state. Under sustained
load or after a stall the preview therefore runs slower than wall time. Safe
quotient/remainder arithmetic admits even maximum-width timestamps. Backwards
time is rejected before timing state changes.

New playback starts paused. Play anchors to the supplied current time without
advancing the simulation; repeated Play while running does not reset phase. Pause
clears timing debt but changes no world, route, cursor or queued commands. Resume
starts a fresh 100 ms interval without incorporating paused elapsed time. Step
advances exactly one tick only while paused and preserves paused state. Timer
polling while paused does nothing. No timing/play/control state is checkpointed;
an application loading a checkpoint always begins paused with fresh admission.
World simulation remains deterministic for the same ordered commands and admitted
ticks; wall-clock event/order timing itself is not a deterministic replay format.

The QObject playback adapter owns a 16 ms precise QTimer wake source and a monotonic
QElapsedTimer. The 16 ms wake is not a simulation tick. It supplies sequential host
tick callbacks and refreshes presentation once after a successful nonempty batch.
The adapter owns no world, sprites, widgets, legacy addresses or hooks. A separate
QWidget emits semantic Play/Pause/Step actions and reflects enabled states. The
scene host supplies `MovementSession::step`, rendering and error/status callbacks.
Movement/stop orders while playing apply at the next admitted tick; pending moves
can still be cancelled before admission. Ordinary selection/target state survives
play/pause refresh. The Save action pauses before opening its modal file picker and
stays paused even if the picker is cancelled.

Tick, clock or refresh exceptions pause timing and report an error through the
host. Each tick retains its existing transaction boundary: earlier successful
ticks in a batch remain committed, while a failed tick rolls back. The failed
batch is not retried automatically. Reentrant poll/Step/Play cannot add ticks;
pause from a callback stops remaining batch ticks. Notification failures also
leave playback paused and stay within dispatch. Window/adapter destruction stops
its owned timer; there are no detached tasks or asynchronous world mutations.

## Validation

`python3 tools/test-scene-playback.py BUILD/world-playback-test NEW_OUTPUT` retains
normal and ASan/UBSan evidence using owned synthetic multi-creature navigation.
An independent per-millisecond reference checks 10,000 irregular wake schedules,
pause resets, remainder retention and dropped-interval counts. Edge checks cover
repeated Play, backwards timestamps and maximum unsigned elapsed time.

Production buttons/controller/native sessions verify no implicit tick on Play,
paused Step only, retained selection/pending stop, pause/resume phase reset,
multiple tick batches, four-tick catch-up and absence of deferred overload debt.
An admitted-tick continuation exactly matches explicit uninterrupted steps and a
fresh-process pending-stop restore. Exceptions pause after the last successful
tick; reentrant callbacks and notification errors are bounded. Tests dispatch the
actual Qt timer with both a fake monotonic source (exact count) and its default
elapsed source (first batch, then no ticks after pause). Broad native motion,
checkpoint, picking, stop controls and scene tests run as regressions. The full
normal suite passes 108 tests; final frontend sources also pass eight directly
affected suites in each normal and sanitizer build.

Fresh [evidence](native-scene-playback.json) fingerprints current source/reports.
[Committed-history review](native-scene-playback-history-review.json) checks exact
intermediate receipts and its [extension](native-scene-playback-history-extension.json)
separately; it asserts neither past gate success nor new
runtime equivalence. Older shared-source evidence retains original fingerprints
and may remain stale; only this new native preview policy gains validation.
Main-window wiring and modal-save pause are reviewed/compiled; whole installed
window interaction, original cadence/pause equivalence, continuous-scene original
comparison, group/faction/commander gameplay and live replacement remain open.
