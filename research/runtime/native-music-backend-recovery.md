# Actual native music endpoint recovery

Prospective native policy scope: actual Qt music/default topology under an owned
private PipeWire/Pulse graph, initial real output, removal/frozen cursor,
caller pause while unavailable, replacement, seek, non-null default switch,
retained track/loop/gain/pause, resume and owned graph/process cleanup. Include
startup MusicBroker teardown/provider lifetime. No physical/original comparison
or uninterrupted exact decoded stream/tail is claimed across source re-admission.

The first actual endpoint run-j0q4n70d correctly freezes loss at371ms but
reports the removed endpoint available; its raw failed record is preserved.
After membership-based reporting, run-ojvqdqby passes loss/freeze/replacement
and1200ms paused seek, but re-admission on non-null default switch advances to
1385ms before caller-paused state is restored. Both failures retain owned daemon
cleanup and original source bindings. Paused source admission now loads metadata
without first starting audible playback. Validation of that fix is pending.

[Fresh actual endpoint evidence](native-music-backend-private-endpoints-20261012.json)
now passes:371ms is frozen on actual loss, replacement remains caller-paused,
1200ms seek survives a non-null default change exactly, then resume advances
to1478ms. Track2, loop and gain12345 remain intact, with2 losses/2 recoveries,
source stability, close and empty graph/owned cleanup. Reporting checks actual
output membership; paused re-admission loads metadata without starting playback.
The music regression CTest passes after a current rebuild.

[Current healthy full game](native-music-backend-healthy-public-20261012.json)
passes complete native intros, WAVs, voices, music, campaign and normal Quit
with current source/input/original/private cleanup fingerprints and no audio
errors or fallback. Historical no-output game fingerprints are retained as
older source after the music change; original/physical waveform and manager
replacement remain separate.
