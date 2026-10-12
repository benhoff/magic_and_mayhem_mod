# Native audio end-to-end implementation

The requested target is native ownership from game/menu sound events through
configuration, sample loading, voice selection, positional controls, music and
movie soundtracks, PCM output and cleanup. This work proceeds in separately
validated commits. Implementation, original comparison and live replacement
remain distinct milestones.

The first commit, `122f5c8`, defaults native voices on for native commands and
isolates channel dispatch and PCM pumping from GUI work. Its bounded live
campaign passes normal Quit with no audio fallback or failed submitted request.

## Native static-WAV input

Behavior `HOST.native-wave-loading` is seeded before implementation. The pinned
No-CD loader at `0x00570140` calls generic file/WAV helpers at eight locations:
open `0x005701bf`, failed-open close `0x005701e2`, size `0x00570216`, format
`0x00570229`, sample read `0x005702ca`, error close `0x00570305`, duration
`0x0057031d` and successful close `0x00570349`. Existing hash-guarded exports in
`working/decompiled/audio-support-_p0gupwd/static_wav_upload.asm` and its WAV
helper exports record the source sequence. Confidence is high for these call
targets, stack cleanup and DWORD duration arithmetic. The constructor at
`0x0058fa10` initializes file/resource fields to zero without allocating; the
native association therefore introduces no abandoned original input allocation.
That additional constructor/close inspection used the same SHA-256-guarded
working source, with immutable manifest verification before/after.

The adapter replaces only these calls after checking all expected original call bytes; it does
not replace shared generic file helpers or guess their other callers. Native
sample storage and the strict AssetFile/WAV parser already exist. The original
source-cache/manager and control order remain active for this intermediate step.
Malformed input and missing-file outcomes must remain explicit; a strict native
rejection does not assert equivalence with unchecked original mmio behavior.

`MNMAUD01` version 3 retains the 128-byte header and 16 MiB payload. LOAD_WAVE
sends an ASCII leaf filename; the worker opens it under the session-selected
Sounds directory, closes the input and returns owned PCM plus checked format,
size and wrapping duration metadata before publishing the matching reply.
Capabilities are published before readiness; staging authorizes only the
declared capability. The PE producer copies returned PCM while holding its
single-request guard, then keeps a local instance association through format,
size, read, duration and close calls. No process pointers cross the channel.
The original upload/create/unlock, cache and scheduler paths remain active.

The explicit native quoted-leaf policy retries a matching apostrophe pair only
after notFound. It does not retry malformed inputs, traversal, ambiguous casing
or other I/O errors. Missing Stream.wav is not fabricated. The strict parser's
existing PCM and input limits apply. The original callback/thread/fault domain
and arbitrary helper callers are outside these eight guarded calls.

[Installed transport evidence](native-wave-loading-installed-pass-20261012.json)
compares all 356 WAVs plus four quoted-leaf requests to Python wave: 12,002,990
PCM bytes match exactly. Two expected missing/escaping requests reject without
publishing stale metadata. All 362 request/ACK pairs match, inputs/sources stay
stable and immutable manifests pass. This is silent native integration.

[Public live evidence](native-wave-loading-public-pass-20261012.json) passes the
default-audio campaign through normal Quit: seven native WAV loads and sample
uploads, 349,862 bytes for each, 12 plays, nine releases and 116 request/ACK.
All eight call hooks are installed; one native device is selected; broker,
adapter and fallback counters are zero. Output starts, readiness is withdrawn,
private Wine stops and source/compiler/WAV/config input fingerprints remain
stable. This is live observation, not human listening, original waveform/timing
equivalence or complete audio-manager/WinMM retirement.

All 60 audio/assets CTests, the two focused Qt menu/broker lifecycle tests,
eight Python admission tests, 19 campaign evidence validators and 10 CLI
admissions/two refusals pass. The PE32 COM/WAV and stopped-host fixtures pass in
`working/tests/audio-bridge/run-cwc6jkk2/report.json`; the earlier sandbox-only
Wine socket failure is retained at `run-c8z20y04`. The first installed sweep
matched all 356 WAVs then failed its case-sensitive oracle lookup for Picview1;
that failed run remains at `working/tests/native-wave-loading/run-vd2s7vs2`.
The current oracle locates the unique installed spelling independently of the
native resolver. Neither failed run is counted as a pass.

## Remaining stages

Native configuration/catalog and manager integration, positional event routing,
live menu/music/media integration, output-consumption timing and observed COM
controls, physical device recovery, and original/live/long-session comparisons
are pending. Trace executed original audio API paths before claiming retirement.
The requested final scope is not achieved by enabling native playback alone.
