# Recoverable native voice output

Native policy, October 12, 2026. Host liveness and output availability are separate.
An admitted native output can disappear without retiring the still-live voice
channel or discarding static PCM, duplicate identities, gain, pan, pitch or
consumed fractional positions. `VoiceOutput` observes output-device changes and
retries unavailable output every 500 ms, using the Device's existing fixed stereo
Int16 output clock. Unsupported new output formats remain unavailable until a
compatible device is selected. Initial output admission remains strict.

An unavailable endpoint adds DirectSound buffer-lost bit2 to logical status;
playing/looping bits remain to preserve the original manager's busy/stop gates.
New Play requests return `DSERR_BUFFERLOST` without mutating playback. Storage,
loading, volume/pan/frequency, stop/reset and release remain usable. Restore checks
the secondary identity and attempts the current output; it reports success only
when output is available. Primary Restore targets the owned output service.
Recovery resumes retained active voices from their consumed positions. Events
refused during an outage are not queued for later playback.

MNMAUD01 version5 retains the 128-byte header and payload. Operation22 is Restore;
fixed capability4 is output recovery and capability8 is an audible consumed clock.
Headless capabilities are6 or7 with native WAV; audible capabilities are14 or15.
Fresh launch admission requires version5 and staged recovery metadata, rejecting
older hosts. Readiness still means the host channel is alive; output health is
reported separately. A timed-out/frozen or stopped host is permanently retired
by the PE adapter and cannot be revived by output recovery.

Synthetic endpoint cases check lost/retry/default rescan, automatic retry,
consumed phase, identity/preferences, buffer-lost status, truthful Restore, stale
IDs, strict initial admission and shutdown. PE fixtures check real primary and
secondary Restore calls through version5 RPC. Actual device disappearance,
default changes, broader real gameplay recovery, speaker latency and original
driver equivalence require independent execution evidence. This document does
not assert that those validations have run.
