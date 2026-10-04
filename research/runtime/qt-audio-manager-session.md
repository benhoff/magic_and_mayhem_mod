# Qt audio manager session

Status: application integration with synthetic output validation; audible
hardware playback and live game routing are separate milestones.
Implementation: `apps/qt-shell/audio_session.*`, `audio_cli.*`.
The native audio/output libraries do not depend on this application adapter.

`AudioSession` owns the recovered manager, native backend, stable schedule caller
slots and output adapter. All controls and output pumping stay on its QObject's
Qt thread. Production `SinkOutput` negotiates stereo Int16 through the existing
`selectOutputFormat` before constructing the native `Device`, then attaches
`QtOutput` and its actual `QAudioSink`. The requested original primary format
remains recovered metadata; mixing uses the negotiated sink clock.

Startup performs AU22 preflight, constructs the manager/source pool for an
explicit map ID, and starts output. Individual missing sources are reported and
blocked at play; they do not prevent use of the rest of a valid catalog.
Permanent preload failures or invalid configuration abort startup. No unknown
request is silently replaced with a guessed sound in this application API.

Caller slots have stable storage for the whole initialized session. There is
one spare slot beyond the configured schedule limit so a new admission can
retire/evict an occupied schedule before publishing. Recovered admission can
return success without publishing when the scheduler is full; the application
reports this as unavailable, not as a playing voice. Unlinked disposed duplicate
annotations are collected after admission. Clock policy is Qt elapsed
milliseconds; RNG is an explicitly seeded native generator for this preview,
not a reconstruction of the original game RNG.

Stop first resets the output sink/partial PCM queue, then destroys recovered
manager resources and finally the host backend. Device samples and caller slots
outlive output callbacks and manager teardown. Restart fully reconstructs the
session, rechecks the catalog, and can change map, filename policy and negotiated
rate. Failed partial construction is reclaimed by host RAII rather than invoking
original cleanup outside its valid schedule domain. Unsupported cross-thread
controls throw before touching the manager. An asynchronously failed Qt sink
stops its own queue; `running()` reflects the stopped output, controls fail and
the caller can stop/restart the session.

A standalone shell preview lists playable and unavailable sources/groups and
provides Play, Stop Session and Restart Session actions. It does not launch Wine
or connect to the live DirectSound bridge. Selection of source compatibility is
explicit in the command and window title. Example (audible device required):

```sh
working/build/qt-shell/mnm-qt-shell \
  --audio-catalog working/game-nocd/Sounds \
  --audio-path-policy dequote-missing-leaf --audio-map 1 \
  --audio-sound 812 --audio-report working/audio-session.json
```

Select an entry and click **Play**. The report includes catalog diagnostics,
startup error and negotiated output rate. A missing sink returns a startup
error; it does not fall back to the game's device or pretend playback succeeded.
The application integration does not alter `--native-voices`, which remains
an independent live bridge experiment.

Offline tests inject a bounded writer through `AudioSessionOutput`, using the
same native `Device` and `PcmQueue` as production. They verify exact overlapping
PCM, group admission, full-scheduler publication, missing/unknown/control gates,
partial queue discard, one-shot completion, permanent preload failure/recovery,
map/policy/rate restart, stopped-output controls, sink-start failure and thread
confinement. The real Qt output null-device path is checked without opening a
physical device. Existing Qt queue, scheduler, admission, aggregate native
manager and lifecycle tests remain in the scoped regression run.

Validation command:

```sh
./tools/test-audio-session.py
# Optional installed catalog oracle, guarded before and after:
./tools/test-audio-session.py --sounds-root working/game-nocd/Sounds
```

The script records source/binary SHA-256 hashes, test logs, both policy reports
and input hashes. Installed ready WAVs are independently checked with Python's
WAV reader for raw PCM byte counts and duration. Successful evidence: [report](../../working/tests/audio-session/run-o2evf5__/report.json)
and [nine scoped CTests](../../working/tests/audio-session/run-o2evf5__/ctest.log),
including shell startup/help and the new catalog/session fixtures.
The session fixture passes address/undefined sanitizers with leak detection
enabled: [log](../../working/tests/audio-session/session-sanitizer.log).
Confidence: high for the tested native application/queue ownership boundaries;
physical audible output is not claimed.

Not established by these tests: audible physical-device quality/underruns,
Qt backend unplug/recovery behavior, original thread/caller timing, live manager
replacement, listener/camera updates, long-running gameplay or balance changes.
