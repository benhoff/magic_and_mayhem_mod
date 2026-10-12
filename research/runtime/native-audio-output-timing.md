# Native output-consumed voice timing

Intentional native policy, October 12, 2026. This is not an original DirectSound
driver comparison or a measurement at the loudspeaker.

The live voice worker associates each rendered PCM block with its voice state,
fractional resampling phase, pitch and logical generation. Qt's processed
microsecond clock advances the reported voice position and completion, bounded
by whole stereo frames accepted by the sink. Fractional byte writes do not count
as complete frames. Backend time is rounded down to output frames; status may
lag by less than one frame. Regressing clocks are ignored within one sink epoch.

The renderer can finish a one-shot while its queued tail still reports playing.
Repeated Play during that interval does not rewind it. Pitch changes take effect
at subsequent rendered blocks, while already queued blocks retain their original
pitch. Static play and write cursors remain equal; this is not a streaming ring.

Stop retains the consumed source position and fractional phase. Seek/reset and
release invalidate earlier queued generations. Logical controls are immediate;
already submitted PCM can still be audible until the backend buffer empties.
The requested sink buffer is approximately 50 ms, with backend-defined actual
capacity. Native rendering retains at most 256 journal entries and two seconds
of outstanding output frames; exceeding either limit refuses further rendering.

Retiring a sink first samples its clock, then discards the backend queue and
rewinds each surviving voice to its consumed state, preserving current gain and
pitch. A new sink starts a separate frame epoch. This enables bounded restart
without skipping queued material; automatic live device recovery is separate.
Offline/headless commands continue to use the generated clock.

`tests/audio-output-timeline-test.cpp` includes an independent per-output-frame
phase oracle, completion boundaries, pitch changes, loops, stop/resume, seek,
release, partial writes, wire queries, sink epochs and both memory bounds.
Its `--backend` mode uses silent PCM with the current Qt output device to check
queued completion and restart. Physical failure, speaker latency, original
driver behavior and audible comparison remain pending.
