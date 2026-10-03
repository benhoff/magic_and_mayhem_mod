# Qt media request channel

`MNMMED01` is a toolkit protocol, not an original game format. Version 1 is
exactly 2048 bytes, little endian, shared by the PE32 producer and Qt consumer.
The Qt shell creates a new channel for each opt-in launch. There is one
serialized outstanding request and separate movie/file-sound players.

| Offset | Type | Meaning / owner |
|---|---|---|
| 0 | 8 bytes | `MNMMED01` |
| 8 | u32 | Version, 1 |
| 12 | u32 | File size, 2048 |
| 16 | u32 | Request sequence: odd while writing, even when published; producer |
| 20 | u32 | Nonzero request ID; producer |
| 24 | u32 | Operation: 1 movie, 2 file sound, 3 stop file sound |
| 28 | u32 | Original WinMM flags for operation 2; otherwise zero |
| 32 | u32 | ANSI path length, 0–259, excludes terminator |
| 36 | 8 bytes | Reserved, zero |
| 44 | u32 | Sound cancellation generation; producer increments before legacy replacement |
| 48 | u32 | Cancelled request ID; producer publishes when abandoning a wait |
| 52 | 12 bytes | Reserved, zero |
| 64 | 260 bytes | ASCII path; only the declared length is read |
| 324 | 188 bytes | Reserved, zero |
| 512 | u32 | Response request ID; consumer |
| 516 | u32 | Response status; consumer |
| 520 | u32 | Heartbeat counter, nominally every 25 ms; consumer |
| 524 | u32 | Last accepted request ID, retained through terminal response; consumer |
| 528 | u32 | Response sequence: odd while writing, even when published; consumer |
| 532 | 1516 bytes | Reserved, zero |

Statuses are 1 loading, 2 accepted, 3 complete, 4 skipped/stopped/cancelled,
5 decoder error, 6 unsupported request/path, and 7 sound channel busy under
`SND_NOSTOP`. Readers require an even sequence unchanged across their snapshot.
The acceptance field prevents a rapid accepted→error transition being mistaken
for an error before acceptance and replayed through the legacy movie renderer.

The producer waits in 5 ms increments. An unchanged heartbeat for five seconds,
ten seconds without acceptance, or ten minutes of synchronous playback ends
the wait and publishes cancellation. A cancelled request is not started later
by the consumer. Before acceptance a failure permits legacy fallback; after
acceptance it returns failure without restarting the same movie. Concurrent
producer calls fall back rather than overwrite an outstanding request.

Movies resolve only under the trusted asset root's `FMV/`, WAV sounds only
under `Sounds/`. The final directory and filename are matched case insensitively;
ambiguous matches, traversal components, missing files, non-ASCII paths, and
symlinks resolving outside that directory are rejected. Absolute Windows path
prefixes may identify the installed asset but never grant arbitrary file access.
Memory/resource/alias sounds are not transmitted.

Evidence: `tools/test-native-media.py` creates synthetic AVI/WAV files and tests
the channel through Qt and an x86 Wine fixture. Successful evidence at
`working/tests/native-media/run-7dt8szqc/report.json` includes exact RGBA/PCM,
completion, skip, sound replacement, loop stopping, busy rejection, traversal,
unsupported fallback, stale host, and calling-convention checks. Confidence:
high for this synthetic protocol; no live game validation yet.
