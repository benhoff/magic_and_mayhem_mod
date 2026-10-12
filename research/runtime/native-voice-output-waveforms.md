# Native voice output waveform validation

Native policy, prospective scope: capture the actual owned software endpoint
monitor after native VoiceBroker v5 controls, and compare nonzero PCM windows
against an independent periodic source oracle. Cover stereo, voice/master gain,
hard pan, frequency conversion and clipping with a second voice, plus explicit
release/ready withdrawal and empty graph/owned process cleanup. This is actual
backend integration, not physical listening or original DirectSound equivalence.

[Actual endpoint result](native-voice-output-waveforms-endpoint-20261012.json)
passes all7 cases. Each independently predicted2048-frame contiguous window
matches exactly (max S16 error0), while allowing unknown monitor-start phase.
The endpoint produces16320 captured frames per case.40 native control ACKs
include exact512-byte WAV acquisition, primary playback, voice/master gain,
pan, frequency and second-buffer controls; release and ready0 retirement pass.
Private graph starts/ends empty and all owned monitor/player/daemon processes
stop. Captures and prospective compiler/source bindings remain under
working/tests/native-voice-waveforms/run-mefzyczr. No production change was
needed. This strengthens backend output evidence beyond storing/acknowledging
controls or zero-valued PCM; it does not measure physical speakers, original
DirectSound output, latency/tail or arbitrary resampler/hardware equivalence.

The fresh focused compiler/CMake closure result is
[native-voice-output-waveforms-focused-endpoint-20261012.json](native-voice-output-waveforms-focused-endpoint-20261012.json).
It repeats all7 exact2048-frame windows without unused music implementation
inputs in its declaration. The earlier broader result remains unchanged.
