# Native DirectSound voice bridge

Reviewed 2026-10-04. This optional adapter routes selected game-facing audio
contracts into the native Device mixer and Qt output. Offline COM/PCM tests and
disposable import staging pass. No live game session, speaker audibility or
original timing equivalence is established by this chunk.

## Implementation and selection

`runtime/audio/bridge.c` builds as freestanding PE32 i386 `MnmAudio.dll`.
`audio/voice_bridge.*` hosts a separate mapped-file command channel on the Qt
thread, owning Device, VoiceCommands and QtOutput. The shell creates the broker
only with `--native-voices`; its launcher receives `--voice-channel`, copies
an isolated DLL and adds its `AudioAnchor` import to the disposable executable.
The launch runner verifies staged executable, renderer DLL and audio DLL hashes.
The original installation/executable is not patched.

The source must match no-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Runtime requires base `0x00400000` and thunk bytes `ff 25 1c 50 5c 00` at
`0x00597566`, then replaces IAT `0x005c501c` (DirectSoundCreate). No fixed COM
object pointer is assumed. The staging script performs all executable changes.
Original-manifest checks are part of the staging validation protocol.

Native ownership is selected once for a default, unaggregated device after a
broker handshake. Other device requests and a failed initial handshake call
the retained Wine DirectSoundCreate. After selection, proxy methods never call
Wine playback: unsupported methods or connection failures return an error.
This avoids unacknowledged replay through two backends. It does not provide
seamless mid-playback fallback or recovery.

## Supported contracts and limits

| Path | Native adapter policy |
| --- | --- |
| COM identity/lifetime | IUnknown and selected IDirectSound/IDirectSoundBuffer IID, atomic local references, final release retires the native voice |
| Setup | Old 20-byte descriptor, primary flags `0x81`, secondary flags `0xea`; device caps report PCM mono/stereo and 8/16-bit capability bits |
| Primary | Metadata format, caps/status and Play/Stop locally; zero byte length, no sample output; nonzero primary attenuation rejected |
| Static samples | Secondary PCM mono/stereo 8/16-bit; offset-zero Lock with flags zero and at most the buffer length; private zeroed staging memory; checked Unlock copies only reported first-region bytes |
| Duplication | Shared native committed samples, stopped independent cursor, copied volume/pan; original release leaves duplicate alive |
| Controls | Play flags 0/1 and zero reserved arguments, Stop, zero reset, volume/pan, GetStatus; cached getters for volume/pan/format/source rate |
| Other methods | Frequency changes, nonzero seeks, cursor queries, secondary SetFormat/Initialize and additional interfaces unsupported; Restore is a no-op for owned memory |

Caps and primary metadata are explicit scaffolding, not measured Wine primary
buffer size, device capability or hardware behavior. Primary volume/pan/master
mixing, geometry, scheduler selection and full engine calls remain unvalidated.
GetStatus reflects generated native PCM, ahead of device consumption by queued
output. A game relying on unsupported methods may reject the adapter. A pointer
returned by Lock is valid until Unlock/release; no post-Unlock pointer lifetime
is promised. Arbitrary ring-buffer writes are not part of this x86 adapter.

The native sample limit is 16 MiB and voice limit 128. Channels/formats are
validated before narrowing into native fields. Native functions own committed
bytes; process pointers never cross the channel. A voice ID is a session-local
32-bit wire handle for the host's native BufferId; it is not a stable address.

## Channel lifecycle

`runtime/audio/protocol.h` defines `MNMAUD01`, version 1, a 128-byte header and
16 MiB upload region. All wire words are little-endian aligned DWORDs.
Commands are serialized with one outstanding request; payload/arguments are
written before release publication of the request ID. The host acquires it,
copies upload bytes, performs the command, and release-publishes the matching
response ID after HRESULT/value. There are no asynchronous pointer loans.

A 2 ms Qt timer services requests. The x86 caller waits at most approximately
one second; contention returns busy. Timeout or withdrawn readiness permanently
retires that producer channel, preventing late replies from applying to a new
request. Native voices then return errors. IDs cannot wrap into a reused request.
Broker destruction withdraws readiness and retires the whole native session.
This diagnostic bridge can stall gameplay and is not a low-overhead production
transport. Same-thread Qt event processing must remain responsive.

Header words 21/22 count native device selections/pre-selection fallback
attempts; word 23 counts failed admitted requests. These are bounded-scope
counters, not whole-program call coverage. Unsupported local COM methods do
not issue host requests. Unknown operation IDs and malformed/truncated payloads
are rejected. The channel is private experiment input, not a hardened service
for hostile producers.

## Reproduce

```bash
python3 tools/test-audio-bridge.py
python3 tools/test-audio-staging.py
cmake -S apps/qt-shell -B working/build/qt-shell
cmake --build working/build/qt-shell --parallel 4
working/build/qt-shell/mnm-qt-shell --repo "$PWD" --renderer opengl --native-voices
```

The first command runs native audio/asset fixtures and a Wine x86 COM fixture;
it never launches Chaos.exe or reads game assets. The native dispatcher fixture
checks exact PCM, volume/pan, completion, duplicate lifetime, rejected controls,
malformed payloads and mapped readiness/acknowledgement lifecycle. The x86
fixture exercises stdcall vtable slots, primary metadata, static upload,
independent playback state, reference counts and unsupported pitch/seeks. A
second fixture against the stopped host checks bounded timeout and permanent
producer retirement. The silent broker does not open an audio device or advance
voices; native completion/PCM is checked independently by the dispatcher test.

The second command stages a disposable game with both renderer/audio imports,
checks all staged hashes and unchanged source hash, and verifies 2927 original
files before and after. It launches no game. The final command opens the shell;
press Launch game only when ready for a separate live experiment. Without
`--native-voices`, the launcher removes inherited channel settings and retains
Wine audio. Existing movies/WinMM sounds are separate paths.

Evidence: `working/tests/audio-bridge/run-6o2_dv38/report.json`, with all 11
combined audio/asset CTests passing, 16 matching request/ack IDs, one native
selection and zero fallback attempts during the successful fixture. Selftest
DLL SHA-256: `5e04bd206cfbb4946f1fd088598ceda73499970f374d68362e2708aca79a427c`;
x86 fixture SHA-256:
`20d029a3c46c158d3c009d77774dce70c9b9a43ffc015bc6140767917efc18d5`.
The native bridge fixture passed AddressSanitizer, UndefinedBehaviorSanitizer
and LeakSanitizer as `working/build/audio-output/audio-voice-bridge-sanitize`,
with leak checking outside sandbox tracing. Qt shell build, help/startup tests
and `--native-voices --smoke-test` passed without launching a game.

Current-DLL staging evidence:
`working/tests/audio-staging/run-ozafusnq/report.json`, with production DLL SHA-256
`8c4a4662d73b99721432801f84f8e0b39cfadc648e5396dad545e7822f55ce32`
and staged executable SHA-256
`61f89cf0a257ef3551daa28352399593f1ac08b222fb2818dd0899ed3e6b3dab`.
A staged import is not evidence that the live game selected or used the proxy.
Confidence is high for exercised native/ABI contracts, conditional for complete
engine compatibility. Next live evidence must cover menu/map startup, overlapping
sounds, loop retirement, transitions/exit, unsupported calls, actual selection
versus fallback and audible/temporal comparison against Wine.
