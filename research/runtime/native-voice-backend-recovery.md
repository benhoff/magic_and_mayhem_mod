# Actual native voice software endpoint recovery

The intended fixture drives the real version5 VoiceBroker transport under an
owned private PipeWire/Pulse graph. It loads native WAV PCM, creates/uploads and
duplicates static buffers, sets primary gain and frequency, and plays a looping
voice. Actual endpoint removal must preserve transport readiness and freeze
output-consumed cursor/status, while Restore truthfully reports buffer loss.
Replacement and a distinct default switch must retain IDs/PCM and acknowledged
controls and resume consumed playback. Final release/stop retires the channel.

The actual current-source endpoint test passes33 requests with one expected
DSERR_BUFFERLOST Restore response. It loads/compares576000 native WAV bytes,
creates/uploads/duplicates buffers, retains root45000Hz/clone40000Hz and primary
-2000 gain, and freezes/resumes the consumed byte cursor. The recorded cursor
is32960 before loss,36368 while frozen, and70928 after replacement. Actual
loss/recovery/default-switch results preserve IDs and transport readiness;
release/stop sets ready0. Both graphs begin/end empty and owned daemons stop.
Generated/consumed frame counters refer to the current output epoch. Evidence:
native-voice-backend-private-endpoints-20261012.json.

The first test incorrectly expected playing status1 for a looping voice; the
implementation correctly returned playing|looping5 after recovery. That failed
assertion and its source fingerprints remain in
native-voice-backend-loop-status-failure-20261012.json. The corrected expectation
and added partial diagnostic retention pass without a production source change. This host transport/software endpoint scenario remains
separate from the PE32 ABI, physical capture/listening, original driver timing,
initial no-device game admission and wider casting/input validation. Gain/pan
acknowledgement alone does not establish audible waveform equivalence.
