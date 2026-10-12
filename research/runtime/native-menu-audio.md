# Native voices in the public menu session

Native DirectSound voices default on with native command presentation, including
`--live-menus --native-commands` and ordinary OpenGL launches. `--native-voices`
is an explicit alias; `--legacy-audio` opts out. These flags cannot be combined.
Capture/readback diagnostics and embedded Wine presentation retain legacy audio
unless native voices are explicitly requested. The ordinary OpenGL route uses the same
broker creation helper. Native movies/file sounds and CD music are separate:
the menu route still rejects `--native-media` and stages movies/CD music off.

This is a host integration policy, not a new recovered audio contract. The
adapter retains the pinned No-CD DirectSoundCreate thunk at `0x00597566` and
IAT slot `0x005c501c`, checks their bytes/base and changes only that runtime slot.
See [voice bridge boundaries](native-audio-voice-bridge.md). The original engine
still loads its WAV data through its existing loaders before uploading PCM to
the native voice service. Loading, all WinMM/mixer APIs and the DSOUND module
dependency are not retired by this change.

## Ownership and admission

Menu preparation checks the original executable hash, builds/copies MnmAudio.dll
and adds AudioAnchor in the disposable copy only. Its manifest records the
audio DLL hash and the exact session-owned `native-voices.bin` path. Compiler
provenance includes the audio sources, shared Win32 declarations and both build
recipes. Native voices require action/render staging.
The staging/test `--source-game` option and session `MNM_MENU_GAME_SOURCE`
override allow a pinned isolated installation under `working/`, preserving a
modified default executable. The default still refuses an unsupported hash.
The native-menu widget asset root remains the normal installation; isolated
validation sources must retain its same assets. Different installations remain
outside this integration proof.

After staging, the shell creates the broker/output and channel before launching
Wine. Missing preparation callbacks or failed output/channel creation stop the
launch and follow the normal finished cleanup. LiveMenuSession and the runner
remove inherited MNM_AUDIO_CHANNEL values. The final runner authorizes only the
manifest's fresh channel and matching DLL: exact size/magic/version, broker ready,
zero request/result/adapter counters, and no channel/DLL symlinks. A heartbeat
may already have advanced. It derives the Wine path itself.

The broker remains alive during native menu changes, campaign handoff and Mini
resume. Menu presentation fallback does not select a second audio backend.
Device ownership, channel polling and PCM pumping run together on an audio
worker, so GUI shader/asset work cannot starve the one-second producer deadline.
The facade's create/snapshot/error/stop methods belong to its owner Qt thread;
worker calls serialize all Device operations with mixing. Stop withdraws ready,
destroys the mapped channel/output/Device on that worker, and joins it. Failure
callbacks return to the facade owner and old-session notifications are ignored.
Normal process completion captures local successful create/upload/play/release
activity and existing adapter selection/fallback/failure counters, withdraws
broker readiness and writes `native-audio-report.json` before destroying the
broker. Errors are logged; the bridge's existing pre-selection fallback and
terminal post-selection failures remain unchanged. Output failure or silent
fixtures cannot pass the audible live evidence validator.

## Validation procedure and limits

Fixture checks use no game assets or output device:

```bash
python3 -B tests/test-native-audio-launch.py
ctest --test-dir working/build/qt-shell -R 'qt-menu-native-audio|audio-voice-bridge' --output-on-failure
```

The menu lifecycle fixture covers successful/disabled selection, failed/missing
audio preparation, inherited-channel removal, preparation before launcher entry
and readiness withdrawal. The broker fixture independently submits mapped
create/upload/play/release requests and records rejected requests separately.
It also blocks the owner thread for1.2seconds while a separate peer requires an
acknowledgement within the unchanged one-second deadline, then restarts the
worker with a new channel and checks that retired requests stay unserviced.

For live evidence use the storage guard and existing bounded campaign tool:

```bash
python3 tools/working-retention.py run --reserve-gib 4 -- \
  python3 tools/test-native-campaign.py --default-audio --normal-quit \
  --difficulty 3 --timeout 300 --hardware
```

Its prospective audio behavior/scenario declaration precedes execution. Success
requires native selections, creates, nonempty uploads, play and release, a
started audible Qt output, no fallback/failed requests/device error, and readiness
withdrawal. Source/compiler/input checks, private Wine cleanup and original
manifest checks remain mandatory. `--check-only` proves admission only.
Observed successful playback requests and a started output do not assert a
human listening check, sample-accurate timing, mixing/attenuation equivalence,
all device branches, complete COM cleanup or all original audio API retirement.

## Recorded explicit integration (historical)

The [initial explicit public result](native-menu-audio-public-pass-20261011.json) passes
native Main/Region/campaign/Mini/resume and normal original Quit on TITAN RTX:
six creates/uploads/releases, 304,942 uploaded PCM bytes, ten plays and 89
acknowledged requests. One native device is selected; fallback, adapter and
broker failures are zero. Qt output starts and the independently read final
channel has readiness zero and matching request/acknowledgement 89. Source,
compiler/input, original manifest and private Wine cleanup guards pass.
This bounded run does not renew casting/combat or long stress validation.

The default working executable is preserved at SHA-256
`1060a739ae220f4dfcf0e86b848df229fd5e37ab253d3d000e39ca2601c6c4cb`.
The retained pinned source and its verified preparation script/manifest are
`working/experiments/native-audio-input/run-h4r5z9xx/`; its game assets match
the normal installation. Launch that source with native voices using:

```bash
MNM_MENU_GAME_SOURCE=working/experiments/native-audio-input/run-h4r5z9xx/game \
  ./tools/run-qt-shell.sh --live-menus --native-commands
```

Preserved unsuccessful attempts remain separate:
[missing prelaunch declarations](native-menu-audio-declaration-refusal-20261011.json),
[unsupported default executable](native-menu-audio-executable-refusal-20261011.json),
and [extended casting/combat/resume run](native-menu-audio-source-drift-timeout-20261011.json)
that timed out before normal Quit and had source drift. None is a current-code
live pass. The earlier and incoming committed-history receipt checks cover
360 file transitions plus 13 commits/228 file versions with zero missing chains;
they assert accounting only.

## Current default and audio worker proof

The [default worker result](native-audio-default-worker-public-pass-20261012.json)
launches the public shell with no audio-selection flag and passes native
Main/Region/campaign/Mini/resume/Quit: seven sample creates/uploads,349862PCM
bytes,twelve plays,eight releases and109request/ACK. One native device is selected;
fallback and both failure counters are zero. Output starts on the audio worker,
normal original Quit passes and the final channel independently has ready0.
Sources,compiled/input fingerprints,original manifests and private Wine cleanup
pass. Ten real CLI admissions/two refusals verify the default,opt-out and
diagnostic policies without a game. The two lifecycle/broker C++ fixtures pass,
including the1.2second owner stall and retired-channel/restart checks.

The [first default attempt](native-audio-default-gui-timeout-20261012.json) freezes
at upload request38: it is later acknowledged successfully but the producer
records one failure and stops submitting. The unchanged one-second producer
timeout and GUI-serviced broker identify a transport starvation failure. The
dedicated worker addresses that failure without extending the producer deadline.
The [first worker journey](native-audio-default-worker-graphics-shutdown-failure-20261012.json)
has109matched requests,zero audio failures and reaches Quit/Main,then fails
rendering recovery before normal process completion. It remains a failed full
journey; the later pass does not establish universal graphics-shutdown reliability.
Physical device disconnection,stale queued failure notifications,human listening,
full audio timing equivalence,long stress and other loader/media/DSOUND retirement
remain outside the current proof.
