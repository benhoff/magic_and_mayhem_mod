# Native menu music integration

## Application policy and boundaries

Opt in with an explicit, readable local file:

```sh
./tools/run-qt-shell.sh --main-menu --menu-music /absolute/path/to/track.wav
./tools/run-qt-shell.sh --main-menu --menu-audio --menu-music /absolute/path/to/track.wav
```

Relative paths resolve against the shell's current directory. Empty, missing,
directory and remote-URL paths are rejected before decoder startup. No original
music track, CD playback choice or game event mapping is inferred. File access
is read-only; Qt's installed media backend determines supported codecs.

`MenuMusicController` owns one music output, independent of the effects manager.
Its production output reuses `NativePlayback(false,true)` and requests infinite
looping. The shared adapter now lives in `mnm-native-playback`, linked separately
by the shell's existing media flow and menu music. Widgets know only semantic
accepted preferences and presentation status. Native audio services and recovered
contracts do not depend on this application policy or QSettings.

Navigation and page changes leave the music channel untouched. Preferences
acceptance changes gain without stopping/replaying the track. Closure or widget
destruction stops playback and invalidates queued notifications. During widget
destruction, both music and effects controllers clear their presentation pointer
before stopping, avoiding updates after the derived widget has been destroyed.

The states are stopped, loading, playing, failed and recovering. Loaded-media acceptance
advances loading to playing; this status is not measured evidence of audible
output. Errors stop the channel, retain the accepted gain and display a separate
persistent music diagnostic. Effects status/retry remains independent. Controller
notifications are deferred off the decoder's callback stack, coalesced by session
generation, and guarded against closure, restart and controller destruction.
Missing devices fail startup clearly. The subsequent
[music recovery chunk](menu-music-recovery.md) adds a separate Retry music action
and availability/default-device recovery while retaining the selected track and
accepted gain; each retry starts from the beginning.

## Preferences

Music uses `audio/v1/musicLevel` in the same user-scoped `MagicAndMayhemMod/QtShell`
QSettings INI as effects. The shared application helper validates integer
attenuation from -10000 through 0, disables settings fallback locations, and
uses -1500 for missing/invalid music settings without rewriting the file.
The gain is `10^(level/2000)`; -10000 explicitly mutes in this native preview.

Restored volume precedes decoder/output startup and seeds lazy Preferences.
Only accepted settings write and synchronize the music key. Cancel changes
neither the active gain nor saved bytes. Acceptance while music is failed still
updates/saves the value for a later start. Effects has its separate key and
controller; neither gain is derived from the other's value. Write failure reports
through the controller failure callback (stderr in the shell), retaining active
session gain. Track selection is explicit on each launch and is not persisted.
Other preferences, original settings import and original music routing remain
outside this integration. A narrower asset-configured slider range still uses
the existing bounded snapshot validation error rather than clamping a saved gain.

## Evidence

Run `./tools/test-menu-audio.py` for the synthetic regression suite. All **25
scoped CTests** pass. The new music fixture covers startup gain/loop request,
readiness, navigation continuity, independent accepted gains/keys, byte-identical
Cancel, mute/full-volume endpoints, offline acceptance, explicit restart,
fresh-process restore, stale callbacks, direct widget destruction, reentrant
closure, bad paths, malformed settings and settings-write failure.

[Report and source/binary hashes](../../working/tests/menu-audio/run-zu2_pz5e/report.json),
[25-test log](../../working/tests/menu-audio/run-zu2_pz5e/ctest.log),
[address/undefined sanitizer and leak-check report](../../working/tests/menu-music/sanitizers/report.json),
[sanitizer log](../../working/tests/menu-music/sanitizers/music.log).

The extracted adapter's existing silent media path also passes an independent
exact SHA-256 comparison for generated stereo PCM and a bounded missing-file
error check under Xvfb:
[decoder report](../../working/tests/menu-music/decoder/report.json),
[decoded WAV](../../working/tests/menu-music/decoder/wave.json),
[missing-file result](../../working/tests/menu-music/decoder/missing.json).
These checks do not exercise the audible music output or real infinite looping.

Confidence is high for the fake-backend controller, settings, lifecycle and
synthetic finite decoder results. Actual music codecs/loop continuity, audible
music/effects coexistence, device disconnection/default-device changes, latency,
long sessions and live original-game music replacement remain unverified.
No game, installed/original assets or physical audio device was used; no
immutable-input experiment was required.

The original evidence above records the initial integration milestone. Current
output recovery and its additional fixture are recorded in
[Native menu music output recovery](menu-music-recovery.md).

## Native CD music in live menu sessions (2026-10-12)

The native music worker and selected PE CD adapter have separate headless
fixtures. Live staging/admission and original playlist scheduling are being
integrated under HOST.native-menu-music. Prepared track assets and their TOC
remain session owned and hash checked. Native music starts before Wine; inherited
music paths cannot enable a new session. Original CD initialization uses the
guarded zero-mixer policy, while its playlist/control callbacks remain active.

Live PCM, original playlist notification delivery, music preferences and normal
Quit retirement are pending. No original driver/timing or whole Wine-library
retirement claim follows from the synthetic fixtures.

The current opt-in command adds `--native-music` to a native menu/command
session. `--music-assets` may select a prepared catalog under working/. The
default prepared catalog is working/assets/native-cd-audio-20261012. Staging
verifies and copies its manifest/track hashes and TOC; the worker validates exact
PCM headers before admission. Inherited music paths, redirected/stale channels,
changed DLL/catalog/TOC and unready workers refuse. Seven admission fixtures
pass. MusicVolume0 remains muted; an isolated volume15 source clone was used
for playback validation, leaving the user's original working preferences intact.

[native-menu-music-media-public-pass-20261012.json](native-menu-music-media-public-pass-20261012.json)
records complete Intro0/Intro1 playback, native music/voices/WAV loading, campaign
and normal Quit with stable inputs/sources and owned Wine cleanup. All CD hook
features are installed, no unsupported CD or legacy-other MCI calls occur, and
no native audio request/fallback fails. Output is requested and available; actual
human listening is not claimed. The original playlist scheduler remains active;
its end-of-track window delivery and full scheduling equivalence remain pending.

The earlier music-only live pass is preserved separately. The complete run
includes the dispatch fix: synchronous service notifications cannot publish an
unfinished command ACK. A47-command independent peer test, including16 repeated
replacements, and the actual PE fixture pass the final code. Bounded local
serialization also avoids rejecting controls during notification polling.

Remaining boundaries include default music/media selection, stable-snapshot
admission retries, stale-host status retirement, live device recovery, consumed
output timing, arbitrary COM/MCI controls, fades, broader/long-run scenarios,
original manager replacement and traced Wine API retirement. Earlier failed
pre-launch compiler declarations, muted-volume and STOP0 failures, and the
intermittent unfinished-ACK PE fixture are retained under working/tests.

Native music/media are now defaults for the validated live native menu/command
route. `--native-music` remains an alias; `--no-native-music` disables the CD
route and `--legacy-media` selects the legacy media policy. `--skip-movies`
retains native file sounds while suppressing staged intros. The current31-feature
producer includes gain query and heartbeat retirement. See native-audio-defaults.md
and native-audio-defaults-public-pass-20261012.json for fresh full default proof.
Earlier opt-in/15-feature results retain their historical scope and hashes.
