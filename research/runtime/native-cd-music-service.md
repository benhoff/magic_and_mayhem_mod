# Native music service

Behavior `HOST.native-music-service` was seeded before implementation. Native
music lives under `audio/`, with no widget, game address or recovered MCI type.
`MusicCatalog` reads a prepared manifest, validates contiguous CD track spans,
frame/byte metadata, fixed stereo signed16 PCM44100 WAV headers and streaming
WAV hashes. Data track1 is retained as metadata and cannot be played. Inputs
stay read-only; decoded playback remains separate from original CD contracts.

`MusicService` owns its output backend and generation-bound callbacks. It exposes
track play, pause/resume, seek, looping, per-music volume, completion and stop.
Output loss retains music position, loop/pause intent and volume for one deferred
recovery when output becomes available. Refused requests are not queued for
replay. Cancelled/stale callbacks cannot revive a stopped or closed session.
The Qt output uses audio-only QMediaPlayer/QAudioOutput, with independent decoded
PCM capture. Decoder cleanup is deferred beyond its notification stack.

[Fresh native service validation](native-music-service-synthetic-20261012.json)
passes14 catalog admissions/refusals, generation/lifecycle guards, fake device
loss/default availability recovery, unavailable-play nonreplay, reentrant close,
end-boundary recovery and real silent Qt playback. Decoded stereo signed16
PCM44100 matches the independently authored 44,100 frames/176,400 bytes exactly.
Looping crosses the track boundary; pause holds position; seek/resume and stop
pass. Recovery retains position, pause/loop intent and volume. End-boundary loss
clamps to the last playable millisecond, an explicit native continuity policy.
All62 audio/assets CTests pass. This is headless native integration and synthetic
device policy, not actual unplug/speaker listening or original driver proof.

The original CD primitive comparison is
[renewed against this build](native-cd-controls-music-build-original-20261012.json)
without refreshing its prior evidence hashes. Earlier source-bound service runs
remain retained under `working/tests/native-music-service/run-r6ley_d5` and
`run-ocbimmsp`; they do not assert validation of the later build recipe.

Legacy
MCI command translation, original notifications/playlist/catalog, live routing,
defaults, physical listening and original timing equivalence remain separate.
