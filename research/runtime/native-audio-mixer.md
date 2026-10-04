# Native stereo PCM mixing

Reviewed 2026-10-04. This is a native policy implementation, not recovered
DirectSound DSP or demonstrated Wine equivalence. The game still uses Wine
DirectSound. Implementation: `audio/mixer.cpp`, with sample ownership in
`audio/buffers.cpp` and playback advancement in `audio/voices.cpp`.

`Device::mixStereo(frames, output)` replaces output with interleaved signed
16-bit stereo integer values. The constructor fixes a nonzero output rate,
default 48000 Hz, independently of primary-buffer metadata. Input supports PCM
unsigned 8-bit or little-endian signed 16-bit, mono or stereo. Mono feeds both
channels. Empty or stopped voices contribute silence.

Each voice retains an integer source frame and a fractional numerator over
the output rate. Every output frame adds the source sample rate, advances whole
source frames and retains the remainder. This avoids accumulated floating
position drift and produces identical PCM across block boundaries. Linear
interpolation uses the next source frame; loops interpolate across the wrap,
while one-shots hold their last sample until the source duration is consumed,
then become completed and contribute silence. Stop/resume retains the fraction;
reset and restart after completion clear it. A duplicate starts stopped at
frame/fraction zero with copied controls and shared committed samples.
Explicit `advanceFrames` advances whole source frames, preserving any fraction
unless completion clears it; callers should not also advance for time already
consumed by mixing.

Volume and pan use hundredths of a decibel, with gain `10^(value/2000)`.
Positive pan attenuates the left channel, negative pan the right. Volume
`-10000` and fully attenuated pan endpoints are hard mute by native policy.
Voices accumulate in ascending ID order using double precision; only the final
sum is clipped to [-32768,32767] and rounded to the nearest integer, ties away
from zero. This avoids clipping intermediate voices before cancellation.

Mixing reads committed storage directly, without PCM snapshots. Pending writes
remain invisible; unlock commits become visible on the next call. Controls and
release are thread confined with the Device. At most 65536 output frames are
accepted per call; an oversized request returns `Error::limit` without changing
output or playback state. Output and input-list allocations occur before voice
advancement; allocation failures throw. Values are host integers, not a defined
serialized byte stream.

## Evidence

Run `python3 tools/test-audio-mixer.py`. Tests consume synthetic fixtures only,
so they require neither original assets, Wine, a display nor an audio device.
All eight audio/asset CTests passed. Mixer coverage includes exact PCM fixtures,
volume/pan, final-sum clipping, stop/resume/reset, duplication, pending writes,
release, limits and 120 mono/stereo, 8/16-bit, source/output-rate and loop
combinations. An independent reference derives positions from absolute output
time and integer weighted samples; permitted error is one PCM unit. Full and
split output blocks must match exactly.

Evidence: `working/tests/audio-mixer/run-vldh3t7l/report.json`, with source
hashes and native fixture SHA-256
`107451bd6d9d653d8f40fce3377d60bbea7f2045cb762ee8849de3d4de87bbb6`.
The standalone fixture also passed AddressSanitizer, UndefinedBehaviorSanitizer
and LeakSanitizer as `working/build/audio-mixer/audio-mixer-sanitize`; leak
checking ran outside sandbox tracing. Confidence is high for the tested native
policies, not original audio fidelity.

## Remaining boundaries

Linear downsampling has no anti-alias filter. Numerical fixture agreement does
not establish perceptual quality. The mixer allocates per call; real-time
threading, latency and allocation guarantees remain untested. Frequency changes,
arbitrary byte seeks and original scheduler selection remain outside scope.
Next is QAudioSink output with format negotiation and lifecycle tests, followed
by a separately observed and validated x86 adapter. Audible playback and live
replacement have not been validated by this chunk.
