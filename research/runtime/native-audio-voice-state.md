# Native audio voice state

## Scope and evidence

`audio/voices.cpp` extends native `audio::Device` with independent secondary
voice state. It consumes the already validated sample ownership foundation and
the [reconstructed engine controls](directsound-voice-controls.md). There is no
game interception, output device, resampling or mixing. No new runtime address
or live timing claim is made in this chunk. The original game still uses Wine
DirectSound; native state is tested offline.

`python3 tools/test-audio-voice-state.py` builds and runs fixture-only checks,
writing logs, source hashes and a report under `working/tests/audio-voice-state/`.
It needs Qt Core for the existing asset targets but no installation, Wine,
display server, audio hardware or original artifacts.

## State and ownership

Each secondary BufferId owns volume, pan, loop configuration, playback state
and a next-source-frame position. It shares only committed PCM storage with
duplicates. `voice(id)` returns a value snapshot; it does not expose internal
references. Primary buffers remain metadata: voice operations return
`unsupported`, and their voice snapshots are absent. Missing/released IDs return
`invalid` or absent snapshots. IDs are local to one Device, not x86 pointers.

| Operation | Native behavior |
|---|---|
| Create static | Stopped at frame zero, volume/pan zero, looping false |
| Duplicate | Copy volume/pan and share samples; new stopped voice at zero, looping false |
| Play(flags 0/1) | Start/resume current position; while playing update loop flag without restarting |
| Stop | Become stopped, retain position and loop configuration |
| ResetPosition | Position zero; playing remains playing; completed becomes stopped |
| SetVolume/SetPan | Change this voice only; position/activity unaffected |
| AdvanceFrames | Consume source PCM frames deterministically; stopped/completed consume zero |
| Release | Remove state and storage reference; invalidate pending writes owned by this ID |

Native status bits are 1 for playing, plus 2 when playing with looping enabled;
otherwise zero. These are immediate deterministic snapshots, not claims about
DirectSound device latency. A retained loop configuration while stopped does
not set either status bit.

Controls retain signed hundredths-of-decibel values. Volume accepts [-10000,0]
and pan [-10000,10000]. Invalid values or play flags outside 0/1 are rejected
without mutation. No logarithmic gain calculation or channel attenuation is
performed yet. The future mixer must interpret these as decibels, not linear
amplitudes or the game's ±3333 positional coordinate scaling.

The selected semantics are supported by Microsoft's
[Play](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/mt708933(v=vs.85)),
[Stop](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/mt708940(v=vs.85)),
[volume](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/mt708939(v=vs.85))
and [pan](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/mt708938(v=vs.85))
contracts. The duplicate's initial cursor/activity, immediate status, completion
representation and strict frame boundaries here are explicit native policies,
not experimentally established Wine behavior.

## Advancement and completion policies

One frame contains every channel: byte count / frame alignment gives the total
frame count. Mono/stereo and 8/16-bit assets advance in identical frame units.
`advanceFrames` counts SOURCE frames, not output frames, game ticks or wall time.
Sample-rate conversion and fractional phases belong to the next mixer chunk.

For one-shots, advancement consumes at most the remaining frames. Reaching the
end changes playback to completed, retains the one-past-end frame count and
reports completion for that transition only. A subsequent Play rewinds to zero
before starting. Stop after completion retains that end position; Play still
rewinds. Completed voices are retained until explicit release or replay: the
engine's independent millisecond deadline is not automatically executed here.

Loops wrap at the whole-buffer boundary and never complete automatically.
Advances larger than one loop, including UINT64_MAX, use bounded modulo
arithmetic without an overflowing cursor-plus-requested-frames sum. Switching
a playing loop to one-shot preserves its position and completes at the next
end. Zero advancement leaves state unchanged. The result reports consumed
frames and whether this operation completed a one-shot; rejected operations
leave that caller-owned result untouched.

Committed writes remain visible to all duplicates without changing any cursor.
Pending writes remain hidden. Releasing a playing source abandons its pending
write and removes only its voice: another duplicate keeps previous samples,
controls and progress. Releasing the last ID releases its samples. Device and
handles remain thread confined; no audio callback synchronization is implied.

## Validation and boundaries

Tests cover exact/oversized completion, replay, stop/resume and zero reset,
zero/multiple/huge loop advancement, loop-flag changes during playback, controls
while playing, duplicate inheritance/isolation, lock commits and abandoned
writes, release during playback, retired IDs, primary rejection, control/flag
bounds and four channel/bit-depth combinations.

An independent oracle advances one source frame at a time across 306 scenarios
(nine buffer lengths, two loop modes, seventeen block splits). A fixture-only
backend also runs the reconstructed start/busy/stop-reset sequence on real
native state, confirming that an engine cache changed by a failed control call
does not overwrite the last accepted native value. This fixture is not a
production bridge or a test of x86 calling conventions.

Confidence is high for deterministic native state and ownership under these
tests. Audible parity, hardware timing, interpolation, pitch changes, arbitrary
byte seeks, primary playback and game scheduler allocation remain outside scope.
Stereo PCM mixing and sample-rate conversion now have separate
[offline evidence](native-audio-mixer.md). Next: QAudioSink output and a
separately tested x86 adapter.

Successful fixture evidence (2026-10-04):
`working/tests/audio-voice-state/run-y8kk406m/report.json`, with all seven
audio/asset CTests passing. Native fixture SHA-256:
`e76e27026772dcf96af71d941e14fd621ea3a2e319c7af30e85bf9a804327caf`.
The standalone voice fixture also passed AddressSanitizer,
UndefinedBehaviorSanitizer and LeakSanitizer using
`working/build/audio-state/audio-voice-state-sanitize`. Leak checking ran
outside sandbox tracing. No original/game assets were consumed by these tests.
