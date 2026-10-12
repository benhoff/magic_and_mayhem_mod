# Native audio defaults and liveness

The full native movies/music/voice/WAV campaign is validated separately from
default selection and failure behavior. Native music/media defaults, independent
opt-outs, movie skipping and inherited-channel scrubbing are the next application
policy milestone. Current live CD/music remains opt-in until those controls pass.

HOST.native-music-watchdog is seeded for a thread-confined monotonic32 heartbeat
watch. Unready startup, frozen heartbeat, withdrawal, clock/counter rollover and
permanent retirement after a failure are explicit branches. PE status, requests
and notifications must use it under their common local owner lock. Pure helper
tests alone do not establish actual PE or live failure handling; physical device
loss and original driver/timing equivalence remain separate boundaries.

## Current implementation and bounded checks

Live native menus/commands now select Qt media and native CD music without
explicit flags. Native voices are required for automatic music selection.
`--legacy-media` and `--no-native-music` opt out independently; explicit native
flags remain aliases and contradictory flags refuse. `--skip-movies` suppresses
staged intro preferences while retaining the native file-sound channel. Readback,
legacy and non-live routes retain their narrower policies. Standalone OpenGL
launches scrub inherited music paths. A prepared CD catalog is still required.

Startup music admission retries an odd/changing consumer response snapshot for
250ms, then refuses instead of mistaking an in-progress publication for stale
ownership. Eight admission checks pass. The public CLI matrix passes10 voice
admissions/two refusals and six media/music admissions/three refusals without
game or audio output. Three focused Qt menu/media lease/preferences tests pass.

The actual PE frozen-host fixture passes: the native process is SIGSTOPped with
readiness1 still published; unchanged heartbeat causes status failure276 and a
local notification8. Resuming the host heartbeat does not revive ownership. Pure
helper checks cover startup/unready, unsigned clock/counter rollover, timeout
and permanent retirement. Explicit music-path mapping/identity/startup failure
now fails DLL initialization; no CD fallback is selected for a broken configured
channel. Source/inputs remain stable and shutdown withdraws native readiness.
Evidence: native-music-watchdog-synthetic-20261012.json. This is host loss, not
physical device loss or real-window notification delivery.

The guarded getter at0x4818b0 now returns native gain0..15 while preserving
original initialized/active gates and ret4/output-pointer ABI. The zero-mixer
policy otherwise makes the original getter report0. The PE fixture verifies
level7, inactive/uninitialized0, invalid output and LastError preservation,
changed prologue refusal and original RX page protection after patching two
entries on a shared page. A new feature bit16 records query installation; all
selected CD features total31. Evidence: native-cd-volume-query-synthetic-20261012.json.
The fixture volume-set tail remains authored; full original mixer equivalence
is not inferred.

Native file-sound success now stops Wine only for known legacy async ownership.
The actual PE/Qt fixture asserts zero original sound calls for the first native
success and exactly one legacy async stop on takeover, with no duplicate or
inactive stop. Exact PCM/video, loop/stop/no-stop/path/LastError and explicit
unsupported memory fallback checks pass. The first cold-prefix lease failure is
retained; the passing fixture warms its private Wine prefix before the timed
server and terminates that prefix afterward. Evidence:
native-file-sound-retirement-synthetic-20261012.json. Unsupported resource/alias/
memory calls and whole WinMM/library retirement remain distinct boundaries.

Full default-selection live validation passes both intros/soundtracks, native
CD music, voices/WAV loading, campaign and normal Quit without explicit media/
music flags: native-audio-defaults-public-pass-20261012.json. Features31 are
installed and unsupported-CD, legacy-other-MCI, voice unsupported/fallback and
failed request counters are0. Native output is requested and available; source/
input guards, owned cleanup and CLI policy checks pass.
Physical listening/faults, consumed-output timing, advanced voice/MCI controls,
fades/full playlist end delivery, full manager/config replacement, wider/long
scenarios and executed Wine API census remain outstanding.
