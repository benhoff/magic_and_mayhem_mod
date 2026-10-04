# Native audio output failure and recovery

Status: Qt host integration with synthetic failure/recovery validation. This
extends AU23/AU24; real hardware unplug/audible behavior and original-game
routing remain separate milestones.

Implementation: `apps/qt-shell/audio_session.*`, `menu_audio_controller.*`,
`menu_preview.*`, and the catalog preview in `audio_cli.cpp`. Tests:
`tests/audio-recovery-test.cpp`, the extended `tests/menu-audio-test.cpp`, and
`tools/test-menu-audio.py`.

## Output selection and notifications

`AudioSessionOutput` exposes failure and availability callbacks on its owning
Qt thread. Production `SinkOutput` forwards the existing `QtOutput` pump/write/
stopped-sink diagnostics and subscribes to `QMediaDevices::audioOutputsChanged`.
The no-argument factory follows the current default output. Removal of the
active device or a default-device change reports loss; availability refreshes
selection and capabilities before retry. An explicitly selected device remains
pinned to its ID and is refreshed from current device information if it returns;
no unrelated fallback is selected for a pinned request. Explicit null-device
fixtures do not subscribe to device monitoring.

The menu and catalog preview now use the default-following factory. Sink errors
retain diagnostics after teardown, rather than depending on a destroyed sink's
`lastError()`. Audio startup without an available device retains the validated
menu request and leaves native navigation usable with a visible retry control.
Invalid catalog, manager/map construction or selected cue failures do not create
an indefinite device-recovery request. The standalone catalog CLI still returns
its startup error if it cannot initially open output; its existing preview gets
runtime status/retry handling after a successful startup.

## Session ownership and recovery policy

The session reports stopped, running, failed and recovering states. A failed
output immediately gates new voice controls. Runtime destruction is deferred
until its failure callback returns, so a Qt pump cannot destroy its own adapter
on the call stack. Repeated failures and availability events are coalesced.
Generation checks invalidate pending notifications on stop, new startup or
failed construction. An availability event delivered in the same turn as loss
is retained until failure cleanup finishes.

Failure discards sink and partial push-queue PCM before destroying manager
buffers and clearing caller slots. The validated root, map and filename policy
remain available for explicit retry or automatic retry on a later device-change
event. Recovery revalidates the catalog, negotiates the current stereo Int16
rate, and constructs a fresh native manager/Device at that clock. Accepted
master gain is restored **before** starting output. Voices, caller slots and
mixed PCM are not resumed or replayed; only a new action produces a new cue.

Accepted effects volume can change while output is absent and is used by the
next successful retry. Controller cue IDs and filename policy are retained;
cues are checked again against the recovered catalog before reporting ready.
Music routing and preference persistence are not introduced here.

There is no polling/backoff retry loop when no device is available or a reopen
fails. A distinct availability event or explicit Retry audio action triggers an
attempt. Closing/stopping the session clears the stored request and cancels all
pending automatic recovery. Cancellation from a recovering-state notification
also wins over the queued attempt.

## Presentation and validation

Opt in as before:

```sh
./tools/run-qt-shell.sh --main-menu --menu-audio
./tools/run-qt-shell.sh --grimoire --menu-audio
```

The status bar has a persistent audio status separate from normal navigation
messages. A recoverable failure shows its diagnostic and **Retry audio**.
Successful recovery reports **Audio ready** and hides retry. Navigation remains
usable during a pause. The catalog preview's Restart session button becomes
Retry audio on a recoverable runtime failure. Startup JSON remains a startup
snapshot; it is not an event history.

Repeatable offline checks:

```sh
./tools/test-menu-audio.py
```

All **24 scoped CTests** pass, including the new recovery fixture and existing
native menu/audio regressions. Tests cover partial-write loss, overlapping/root
voice cleanup, deferred/coalesced notifications, absent devices, changed output
rate, retained filename policy and accepted/offline-updated gain, silence on
reopen, first-pump failure, manual retry, automatic availability recovery,
failure/availability ordering, stopped-session and reentrant-close cancellation,
thread confinement, persistent retry UI, and invalid-catalog boundaries.

Evidence: [report and build/source hashes](../../working/tests/menu-audio/run-bybsjuek/report.json),
[24-test log](../../working/tests/menu-audio/run-bybsjuek/ctest.log).
Address/undefined sanitizers with leak detection enabled pass for
[recovery](../../working/tests/audio-recovery/sanitizers/recovery.log),
[menu](../../working/tests/audio-recovery/sanitizers/menu.log) and
[session](../../working/tests/audio-recovery/sanitizers/session.log) fixtures.
All inputs are generated fixtures; no game, installed/original assets or physical
output device were used in validation. No immutable-input experiment was needed.

Confidence: high for tested state, queue, gain and ownership behavior. Actual
Qt backend disconnect notifications, default/pinned physical-device selection,
audible behavior, driver reopen failures, hardware rate negotiation, recovery
latency and long sessions remain unmeasured. Catalog revalidation is synchronous
and bounded; its UI latency on real installations remains unmeasured. Original
DirectSound routing/replacement is not advanced by these tests.
