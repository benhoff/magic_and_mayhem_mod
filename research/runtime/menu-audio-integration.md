# Native menu audio integration

Status: opt-in application integration, synthetic PCM and widget validation.
Implementation: `apps/qt-shell/menu_audio_controller.*`, the lifecycle ports in
`menu_preview.*`, and `AudioSession::clearVoices/setMasterVolume`.
This does not route original game calls or establish live replacement.

One application controller owns one audio session for a native menu preview.
Widgets continue to emit semantic actions and know no sound IDs, profiles,
reconstructed addresses or output-device details. `MenuPreview` announces a
screen after its navigation handlers exist, announces transitions before
changing the stack, and announces accepted window closure. The audio controller
binds each screen once. Navigation handlers run before the sound handler, so
old queued cues are discarded before one activation cue sounds in the new
screen. Keyboard and mouse actions use the same semantic signal; no global
mouse/key interception, hover sounds or typed-character sounds are added.

The default **native preview policy** maps activation to ID 822 (catalog name
`Default click`) and successful Grimoire page changes to ID 830 (`Page turn 1`).
These names are recorded in the earlier [installed catalog report](../../working/tests/audio-session/run-o2evf5__/installed-dequote-missing-leaf.json).
They are not recovered original menu-to-sound mappings. Both IDs are selectable
on the command line, including valid randomized-group IDs; startup preflight
rejects unavailable selected cues rather than substituting another sound.
Literal source filenames remain the default; missing-leaf dequoting is explicit.

Bindings cover Main/Quick/Mini menus, results, map/save/load, Preferences,
multiplayer forms/selection/lobbies, Single Player setup, Region Entry,
Character Screen, Spellbox and Grimoire. Accepted action/request/cancel/close
signals use the activation cue. Only successful active Grimoire page changes
use the page cue; failed navigation and asset initialization emit none.
Draft slider/radio changes and individual inventory edits have no separate
cue contract in this chunk.

Effects gain starts at the accepted Preferences default (-1000) and follows
accepted `soundLevel`, applied to the session's native primary/master gain.
Cancelled drafts do not alter it. Accepted changes clear queued old-gain PCM
before applying the new gain and playing their confirmation. A native minimum
level of -10000 explicitly suppresses new cues in this preview. Music preferences
remain separate accepted UI state; this session has no music playback route,
and changing music level does not alter effects gain. No settings persistence
or original engine preference application is claimed.

Screen changes stop/reset the sink and partial push queue, stop and rewind every
cached root/duplicate irrespective of recovered wall-clock expiry, clear
schedule caller slots, and restart output against the same native Device.
Manager/catalog/cache ownership survives navigation. This is an explicit host
cancellation operation, not an alteration of the recovered scheduler. A failure
to restart output tears down the audio session while navigation remains usable.
Accepted window closure or preview destruction stops output before releasing
manager samples and caller slots. Repeated visits do not accumulate bindings.

Preview (opens an audible Qt device; no game):

```sh
./tools/run-qt-shell.sh --main-menu --menu-audio
./tools/run-qt-shell.sh --grimoire --menu-audio
# Optional native compatibility / alternate catalog cue choices:
./tools/run-qt-shell.sh --main-menu --menu-audio \
  --menu-audio-policy dequote-missing-leaf \
  --menu-click-sound 822 --menu-page-sound 830
```

An unavailable device or selected cue fails explicit audio startup. Audio-only
options require a menu preview; orphan options return usage error before
output/game startup. The default menu preview remains silent unless opted in.

Offline validation:

```sh
./tools/test-menu-audio.py
```

The validator generates synthetic JPEG/config/Grimoire/WAV data and uses a fake
sink backed by the production `PcmQueue` and native mixer. Real widget clicks,
Space activation, Escape cancellation, page controls and Preferences Apply/
Cancel verify exact cue/overlap/group PCM, accepted gain, music/effects separation,
mute/unmute, partial queue discard, one shared Device, repeated-screen binding,
selected-cue rejection, output failure/recovery and window cleanup. Existing
native menu navigation and audio regression fixtures also run: **23 scoped
CTests passed**. [Report and hashes](../../working/tests/menu-audio/run-fkg65k7f/report.json),
[test log](../../working/tests/menu-audio/run-fkg65k7f/ctest.log).
Menu and session fixtures pass address/undefined sanitizers with leak detection
enabled: [menu](../../working/tests/menu-audio/sanitizers/menu.log),
[session](../../working/tests/menu-audio/sanitizers/session.log).
No installed/original assets, physical audio device or game were used in this
validation; immutable-input experiments were not needed.

Confidence: high for the tested native preview policy and ownership boundaries.
Other bindings compile against their semantic widget contracts, but independent
audio scenarios for every screen remain unmeasured. Original cue choices,
audible hardware quality, output unplug/recovery, rapid-navigation sink restart
latency, music routing, persisted preferences and live original audio routing
remain separate work.

[AU25 output recovery](audio-output-recovery.md) extends this milestone with
failure notifications, current-device refresh, persistent retry status and
accepted-gain preservation. Hardware unplug/audible validation remains open;
the added recovery evidence is synthetic.
