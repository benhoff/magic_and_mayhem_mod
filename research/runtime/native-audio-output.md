# Native Qt PCM output

Reviewed 2026-10-04. `audio/qt_output.hpp` and `audio/qt_output.cpp` connect
the native Device mixer to QAudioSink. This is a standalone native device
adapter, not a game hook or a replacement for the Qt shell's existing movie
and WinMM file-sound paths. The game still uses Wine DirectSound for voices.

## Contract and ownership

Negotiate before constructing Device: `selectOutputFormat` checks stereo Int16
at the requested rate, device-preferred rate, then 48000, 44100 and 22050 Hz,
skipping duplicate/nonpositive candidates. It returns invalid if none is
supported. No mono/float coercion occurs. Device's immutable output rate must
match the chosen format. `start` rechecks the exact clock/device pair; a null
device or unsupported clock fails with diagnostics without advancing voices.
Format support checks follow [QAudioDevice](https://doc.qt.io/qt-6/qaudiodevice.html).

Device must outlive QtOutput. Create, use and destroy the adapter and control
voices on the same Qt event-loop thread; moving it between threads is outside
scope. The adapter checks thread ownership on start/stop. QtOutput uses push
output, not an audio callback accessing thread-confined game state. A precise
5 ms timer feeds at most 1024 frames per tick according to backend bytesFree.
It requests approximately 50 ms of backend buffering; actual buffering is
platform-dependent. Idle/starvation is refillable. Unexpected stopped state,
write errors or exceptions stop output and notify the optional failure callback.
This is not a real-time guarantee or measured latency budget.

PcmQueue serializes host signed 16-bit stereo mixer values into Qt's Int16
stream. It retains unwritten bytes across partial writes, including odd byte
counts, and zero writes leave the cursor/queue unchanged. It never remixes a
pending block. No frame is generated when capacity is less than four bytes and
there is no pending data. Each block is bounded to 4096 bytes. Native sample
decoding and resampling remain in the independent mixer.

Stop cancels the timer, invalidates the writer reference, resets/destroys the
sink and discards the pending queue. Repeated stop is harmless; start first
stops any previous stream. Reset is deliberate: Qt documents that it discards
buffered sound, while stop may synchronously drain on Linux/Darwin. The returned
push writer is invalid after stopping/restarting.
[QAudioSink lifecycle reference](https://doc.qt.io/qt-6.8/qaudiosink.html).

Mixing advances voices when generating a block, ahead of speaker consumption.
Stop discards queued/generated sound without rewinding voice cursors. Restart
continues from the next generated source position. This is explicit native
policy, not DirectSound cursor equivalence. Controls affect subsequently
generated blocks; existing queued sound retains its controls.

## Reproducible checks and evidence

```bash
python3 tools/test-audio-output.py
python3 tools/test-audio-output.py --device-test
```

The default runs fixture checks only, opening no audio device. All nine
audio/asset CTests pass, including fallback/deduplicated format selection,
null devices, exact serialized PCM, partial writes, backpressure, bounded
blocks, write failures, queue discard and repeated stop/failed start.
The standalone fixture also passed AddressSanitizer, UndefinedBehaviorSanitizer
and LeakSanitizer as `working/build/audio-output/audio-output-sanitize`, with
leak checking outside sandbox tracing.

Optional device testing probes the default output and plays a quiet 440 Hz
synthetic fixture for two seconds, with stop/restart after one second. Each
device command has a 15-second timeout. No game or original assets are read.
The CLI is `working/build/audio-output/mnm-audio-output --probe` or `--tone`.

Successful evidence: `working/tests/audio-output/run-9i6aufk7/report.json`,
with source hashes, all test logs and probe/tone output. Native fixture SHA-256:
`25b58449c3855c627c22ddbc1fbbdf43a027443788a02829b9dc25e9ba6b1921`.
CLI SHA-256:
`ebd516f07d3e748cc3078cc5aa394234519bf28202341868eaaa9eac99edfa34`.
Qt 6.11.2 accepted stereo Int16 at 48000 Hz on the default host output
`Background Recorder Meeting Capture`. The backend reported 960000 processed
microseconds before restart and 1024000 afterward. This supports backend
delivery/lifecycle confidence, not speaker audibility or fidelity. Initial
sandbox probing was blocked by audio-service filesystem/access restrictions;
the bounded host check succeeded outside the sandbox.

Remaining work: choose/refresh application output devices, route native/game
voice requests into this adapter, validate an isolated x86 bridge and retained
fallback, and test audible output, device disconnection, long sessions,
underruns and latency. No automatic device-change recovery, cross-thread voice
command queue, float/multichannel output or original timing equivalence is
claimed. Existing native mixer interpolation/allocation limits still apply.
