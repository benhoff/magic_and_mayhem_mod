# Native menu music output recovery

## Scope and production adapter

Native application policy, not a recovered original music contract. The explicit
`--menu-music FILE` selection now remains available for output recovery within
that menu session. Retry starts the same looping file from the beginning; no
playback position or old decoded buffers are resumed.

The Qt output adapter owns a QMediaDevices monitor. While a decoder is active,
removal of its selected device ID reports disconnect, and a changed default ID
reports a default-output switch. Unrelated output-list changes leave playback
alone. A non-null current default on a later device-list notification reports
availability. The existing QMediaPlayer decoder/error and unexpected-completion
path also reports failure. Device decisions use a small shared ID policy tested
with synthetic lists; actual Qt notification delivery remains a physical boundary.

Every open chooses the then-current default and explicitly sets that QAudioDevice
on the new NativePlayback output before playback. The decoder is recreated, and
accepted music gain is applied before source/start activity. Other media users
retain their existing default-device behavior. This integration does not add a
pinned-device selector or a polling/retry timer loop.

## Controller and presentation

Failures gate readiness/retry immediately, retain the first diagnostic and defer
decoder teardown until the backend callback stack has returned. Repeated failures
are coalesced. Deferred readiness, failure and availability callbacks carry a
session generation and lifetime guard; old notifications cannot alter a later
track or revive a stopped/destroyed session.

The selected absolute local path and accepted gain survive output failure. The
status bar shows its own **Retry music** action and persistent music diagnostic;
effects retry/status stays independent. Manual retry or a later availability
event changes state to recovering and reopens the file. The path is checked for
readability again. A previously valid selection remains retryable if it later
becomes unreadable, so restoring the file can allow another explicit retry.
An invalid initial selection is not retained as a device recovery request.

Availability delivered in the same event turn as output loss is retained through
failure cleanup. Repeated availability notifications cause one open attempt. A
failed reopen remains failed/retryable, with no automatic polling. A distinct
later availability event or explicit action can try again. Acceptance of music
volume while output is absent still updates and saves that independent setting;
reopening uses the latest accepted value. Recovery never emits a menu cue.

Closure, direct widget destruction and stop discard the retained track, cancel
queued attempts and stop playback. Closure during the recovering notification
wins before the decoder opens. Recovery and other controller actions are confined
to the owning Qt thread. Initial device absence also retains a validated track
and can recover when a device becomes available. The track itself is not saved
between application launches; the command-line selection remains explicit.

## Offline evidence

Validation used an isolated source snapshot pinned at the recorded committed
baseline plus this chunk's changes, excluding later/uncommitted parallel work:
[snapshot](../../working/tests/menu-music-recovery/snapshot.json).
All **26 scoped CTests** pass, including existing music, settings, menu, effects
recovery and manager/queue regressions:
[report with source/binary hashes](../../working/tests/menu-music-recovery/source/working/tests/menu-audio/run-p_lnvozk/report.json),
[CTest log](../../working/tests/menu-music-recovery/source/working/tests/menu-audio/run-p_lnvozk/ctest.log).

The new fixture covers device ID decisions, unrelated notifications, duplicate
loss, first-error retention, deferred cleanup, manual retry while absent,
availability coalescing, initial no-device recovery, same-turn default-switch
recovery, failed automatic reopen with no retry loop, accepted offline volume,
selected track/loop continuity on reopen, stale generations, thread confinement,
queued closure, reentrant closure and changed-file validation.

Address/undefined sanitizers with default leak detection pass for both the new
recovery and existing music lifecycle fixtures:
[report](../../working/tests/menu-music-recovery/sanitizers/report.json),
[recovery log](../../working/tests/menu-music-recovery/sanitizers/menu-music-recovery-test.log),
[music log](../../working/tests/menu-music-recovery/sanitizers/menu-music-test.log).
All assets/outputs are generated or fake. No original/installed assets, game or
physical audio output were used; no immutable-input experiment was needed.

Confidence is high for tested controller state, ownership, retry policy, selected
file/gain preservation and device-ID decisions. Physical disconnect/default-device
notification delivery, actual device binding/reopen, audible coexistence and
looping, Qt/driver failures not surfaced through these callbacks, recovery
latency and long sessions remain unmeasured. Original-game music routing and
live replacement are unchanged.
