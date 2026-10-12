# Native movie and file-sound output retention

Intentional native policy in NativePlayback: follow the default audio output,
pause a finite or looping decoder while it is unavailable, and retain the same
source, decoder position, loop setting, volume and caller pause intent. Topology
notifications are deferred and coalesced; a 500 ms retry also rescans availability.
Recovery changes the output device and resumes only when the caller wanted play.
It does not seek to the beginning or send a second media request.

Explicit device selection transfers topology ownership to the caller. The menu
music controller continues to use its existing recovery policy. Cancellation
and destruction stop retries; decoder failures remain terminal native failures.

The final synthetic execution passes initial absence without media advancement,
500 ms rescan without notification, repeated loss notifications, frozen decoder
position, retained caller pause after recovery, explicit device ownership and
cancellation. Each complete sound/movie retains exact44100-frame decoded PCM;
the movie retains all20 RGBA frames. The test uses a real Qt decoder/output
with muted gain and injected topology, not a physical unplug. Evidence:
`native-media-output-recovery-headless-synthetic-20261012.json`.

The initial compiler-manifest tokenizer admitted a relative Make target as a
source path and refused the run before playback. The corrected tokenizer uses
absolute compiler dependency paths. That raw failure remains retained under
`working/tests/native-playback-recovery/run-t_sm7w25/`. A later final-scope run
retains its own prospective claims; earlier records were not refreshed.

The current default native campaign completes both full intros, native voices,
WAV/CD and menus/render/loading plus normal Quit with source/input/original
checks and owned Wine cleanup. Wine DSOUND remains disabled. Evidence:
`native-media-output-recovery-public-pass-20261012.json`. It uses a healthy
endpoint, so its zero loss/recovery counters do not validate physical failure. Physical device disconnect,
switching to a different nonnull device ID, driver failures without a topology
notification, audible position/tail continuity,
and whole original movie equivalence remain separate validation boundaries.

Wider casting/combat remains unvalidated. The software attempt failed at native
menu lease retirement with about1.66FPS; the isolated NVIDIA attempt sustained
about20–72 nativeFPS but failed to produce the requested original Zombie.
Both reports retain stable sources/original manifests and owned Wine cleanup,
and are preserved under native-audio-casting-combat-{software,hardware}-failure-20261012.json.
Their stopped voice channels show ACKs and0 adapter failures/fallbacks/unsupported
calls; these partial observations do not validate casting/combat or diagnose its
input failure. No successful execution evidence is registered from either run.

The schema scenario level is headless; native_integration denotes the evidence
kind. The fresh final headless-scope run passes without refreshing the earlier
claim fingerprint.
