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

## Native game factory and live movies

The game import descriptor for `DSOUND.dll` ordinal1 is now redirected to
`MnmAudio.dll`, which exports `DirectSoundCreate` at ordinal1. The patch script
requires the pinned executable hash and exact descriptor, ordinal lookup and
original thunk bytes; it retains the game's original IAT/thunk addresses.
The PE adapter publishes an independently checked native-factory flag. Local
unsupported COM calls have a saturating counter, including calls that never
reach the host request queue. This is a game-factory boundary, not a complete
absence proof for all audio APIs.

Wine Quartz still imports Wine DirectSound transitively. Disabling that module
is an explicit `MNM_NATIVE_DISABLE_DSOUND=1` diagnostic, never a default launch
policy. [The initial module-disable run](native-audio-factory-disabled-movie-dependency-failure-20261012.json)
and [the media-enabled retry](native-audio-factory-media-dependency-failure-20261012.json)
fail during Quartz startup, before useful media commands. Both retain cleanup,
source and immutable-input checks. These failures do not establish retirement.

Live menus admit `--native-media` with the native command renderer. The session
prepares and owns the media channel before Wine launches; staging enables
startup movies, scrubs inherited channels and fingerprints AVI inputs. Qt movie
frames use the viewport, release/suspend forwarded game input and record video,
soundtrack, decoder and completion history at session finish. Unsupported movie
control bytes and preacceptance errors retain the existing bounded fallback.
Native file sounds still retain supported/unsupported WinMM transition policy;
this milestone does not retire every WinMM call.

Full startup playback exposed three distinct integration boundaries. The
[first run](native-menu-media-startup-lease-failure-20261012.json) exhausted the
menu startup lease; the [second](native-menu-media-render-frame-lease-failure-20261012.json)
exhausted the initial rendering frame budget; the
[third](native-menu-media-command-drain-failure-20261012.json) exposed producer
ACK starvation while movie display paused command reads. Native movies now
pause both startup budgets while bounded native command reads continue. Game
GPU presentations are held during the movie and only the latest complete frame
resumes afterward. Two focused lease/lifecycle checks and a synthetic stream
larger than the command queue pass; the latter checks unchanged movie pixels,
zero escaped game presentations and a single idempotent latest-frame resume.
[Fresh public campaign evidence](native-audio-factory-media-public-pass-20261012.json)
passes both complete startup movies, campaign entry/resume and normal Quit.
The two movies deliver 3,870,087 captured PCM soundtrack frames and 2,781
presented video frames with no decoder errors, native output requested and
an available audio device. The game uses its native-only factory, all eight
WAV helper redirects, seven loads/uploads totalling349,862 bytes,12 plays and
116 request/ACK pairs. Local unsupported-COM, submitted-failure, adapter-error
and fallback counters are zero. Readiness is withdrawn, private Wine stops and
all source/compiler/media/config/original-input checks pass. This validates a
bounded live journey; human listening, original timing/waveforms, every movie
control/fallback branch and the complete audio manager remain separate.
Nine audio and four media admission checks, two Qt lease/lifecycle checks and
the GPU command-drain regression pass. Native factory staging and synthetic
PE32 COM/WAV/stopped-host fixtures also pass; their raw results remain under
`working/tests/audio-staging/run-046lwoul` and
`working/tests/audio-bridge/run-nkni0yrl`.

## Remaining stages

Native configuration/catalog and manager integration, positional event routing,
live menu/music/media integration, output-consumption timing and observed COM
controls, physical device recovery, and original/live/long-session comparisons
are pending. Trace executed original audio API paths before claiming retirement.
The requested final scope is not achieved by enabling native playback alone.

## Native CD music live integration

The native catalog/service, versioned music worker and guarded CD/MCI adapter
now compose with native movie startup, native voices and WAV loading in a full
public menu/campaign/normal-Quit journey. See native-menu-music.md and
native-menu-music-media-public-pass-20261012.json. The original playlist/control
scheduler remains active. Music defaults, device/consumed-timing recovery, full
manager integration, broader controls and traced API retirement are still pending;
this live observation does not close those separate milestones.

## Default movie/music routes and failure controls

Native movies/file sounds and native CD music now default in live native
menu/command sessions. Both intros and soundtracks, CD PCM, voices/native WAV
loading, campaign and normal Quit pass without explicit media/music flags.
Independent opt-outs/skip policy, coherent startup admission, actual frozen-host
PE retirement/no revival and guarded native gain query are tested. Successful
native file sounds no longer call Wine merely to stop an inactive legacy channel.
See native-audio-defaults.md and its fresh records for scope and remaining work.

## Extended static voice controls

Native secondary pitch, aligned byte seek and generated play/write cursor
queries are implemented through version4 native voice transport and PE COM
methods. Native ramp/phase/duplicate tests, real PE controls and63 CTests pass;
installed WAV and current native campaign/normal Quit are renewed. See
native-audio-controls.md. Generated position remains distinct from actual
output-consumed position; physical recovery and original timing remain pending.

## Backend-consumed voice progress

Live audible voice status and static cursors now follow Qt backend consumption,
bounded by accepted whole stereo frames. A rendered one-shot tail remains
playing until consumed. Queued blocks preserve their own pitch/phase; seek,
stop and release cannot receive stale progress. Sink retirement rewinds queued
material and starts a fresh clock epoch. See native-audio-output-timing.md for
the independent phase oracle and silent Qt completion/restart protocol.
Physical speaker latency, automatic device recovery and original-driver
comparison remain distinct milestones.

## Recoverable voice output and version5

An admitted output can now disappear while the live native voice channel keeps
its PCM, identities, consumed positions/phase and preferences. Output selection
changes and a500ms retry recover compatible current devices; version5 adds
truthful Restore and buffer-lost status. Fake endpoint cases, controlled silent
Qt endpoint loss/automatic recovery,66 regressions and real PE Restore dispatch
pass. The version5 native WAV transport again matches362 installed requests and
12002990 PCM bytes. See native-voice-output-recovery.md. Physical disappearance,
platform default-event delivery, initial missing-device policy and original
driver equivalence remain separate; a stopped/frozen host cannot be revived.

## CD stop/fade control order and native gain

The original stop/fade caller is reconstructed and compared separately with
scripted callee boundaries:62,208 cases,93,568 events and full receiver guards.
A guarded five-byte converter hook supplies native music gain so the zero-mixer
policy can retain original descending steps, clock polling, stop and restoration.
Its PE stack/conversion/LastError/drift/page-protection checks pass. See
native-cd-fade-gain.md. Original callee recursion, clock wrap, actual waveform
and whole playlist behavior remain distinct from this caller comparison.

## Native nonzero movie controls (2026-10-12)

The selected movie adapter now sends every control through native playback.
Its low byte selects presentation: zero delivers frames; nonzero consumes video
and soundtrack without delivering video to the viewport. Media wire v1 adds
flag1 without changing layout/version. Busy ownership, Escape cancellation,
input suspension and menu/render lease suspension apply to both modes. The
presentation intent is available before playback starts, avoiding a viewport
switch for hidden video.

Fresh PE/Qt synthetic evidence in
`native-movie-hidden-video-synthetic-20261012.json` checks controls1/-1/256:
the two nonzero low bytes decode10 exact RGBA frames and22050 exact PCM frames
while presenting zero;256 presents all10. Eleven requests also cover completion,
skip, file-sound ownership, unsupported/path/stale refusal, LastError and ABI.
Fresh `native-movie-controls-public-pass-20261012.json` records both ordinary
intros (1777/1004 presented frames), native voice/WAV/factory/CD music, campaign
and normal Quit. It does not establish live use of nonzero controls. Original
global skip/setup failure/placement details, codec/driver timing and Quartz
startup dependencies remain distinct. Retrospective8358104a reviews26 exact
transitions with zero receipt gaps; no historical validation is renewed.

## Native startup availability and Wine DirectSound module retirement (2026-10-12)

The pinned startup check0x483f30 creates a DirectShow FilterGraph/IGraphBuilder
even when selected movies are native. Its unchanged75-byte caller passes49
signed HRESULT combinations/121 scripted COM events in
`native-media-startup-original-20261012.json`; COM/codec implementations remain
outside that comparison. The guarded selected call0x4e95d1 now asks the native
media broker for availability, preserving registers/flags/LastError and returning
unavailable on a missing or frozen host. Installed native movie ownership refuses
preacceptance legacy replay. Media v1 adds operation4 without layout changes.

`native-media-startup-synthetic-20261012.json` passes14 broker records and PE
call-byte guard/all-register/carry/stack/LastError tests, strict movie refusal,
and renewed full10-frame RGBA/22050-frame PCM hidden-control regression. Ten
audio and four media admission tests pass. Native factory plus marked native
media staging disables Wine DirectSound by default; legacy/no-media/unmarked
staging keeps prior overrides, with explicit diagnostic0/1 selection available.
An owned safe launch receipt records actual module policy without environment
contents. See `native-media-startup-probe.md` for exact scope and retained failures.

`native-media-startup-default-public-pass-20261012.json` verifies actual default
DSOUND disable without an explicit module flag, one availability ACK, both
native intros (1777/1004 presented frames), seven WAV loads/349862 PCM bytes,
12 voice plays/113 ACKs, seven CD starts/120 music commands, campaign and normal
Quit. Audio unsupported/fallback/errors are zero; consumed voice frames are
11023359 versus11024736 generated. Sources/inputs/original manifest/owned
cleanup pass. Prefix FilterMapper2 registration still logs blocked Quartz
attempts, separately from the replaced game FilterGraph check. Unsupported
WinMM calls, other COM paths, full manager/config/cache/event migration, initial
missing-device/physical recovery, long playlist/waveform/listening and a complete
executed Wine API census remain pending. Exact76c88ab0 history reviews21
transitions with zero receipt gaps, without renewing historical validation.

## Observed original import call census (2026-10-12)

The optional pinned PE32 observer installs after existing native adapters and
forwards all190 original import slots with saved flags, atomic low/overflow
counters and unchanged registers/arguments/stack/targets. Target pointers remain
private; version1 MNMAPI01 stores RVAs, labels, classifications and counters.
Staging checks original hash/catalog and runtime guards/readability/protection
before mutation. Final decoding requires stopped private process and exact
original metadata/reserved/header/classification checks. See wine-import-census.md.

The fresh synthetic PE checks pass stdcall/cdecl/fastcall, arithmetic flags,
ECX/EDX/LastError, overflow and10000 repeats, plus ten malformed/active decoder
refusals (wine-import-census-data-synthetic-20261012.json). Five shared protocol checks
pass. Current full native default voice/WAV/factory/media/CD/render/menu/campaign
and normal Quit pass with all190 counters installed and DSOUND disabled.
wine-import-census-data-public-pass-20261012.json records 89
distinct exercised direct Windows/Wine import targets, 13756840
calls, 8 exercised native shim imports and
1 legacy codec import. Counts are read after owned
Wine shutdown. This is a defined import boundary, not a full Wine API count:
dynamic, COM/vtables, shim forwarding/cached pointers, transitive and prefix
activity remain excluded. Audio adapter unsupported/fallback/errors remain zero.
Exact0144cb6c history reviews28 transitions without receipt gaps; no historical
validation is refreshed. Native manager/config/cache/events, initial no-device
policy, media/physical recovery, long playlist/waveform/listening and broader
executed API coverage remain pending.

The initial code-address classification for the import table was corrected to a
data relationship before this fresh prospective run. The earlier successful
public result retains its original claim fingerprint and is historical.

## Native movie/file-sound output retention (2026-10-12)

NativePlayback now follows default-output topology through an owned native seam,
defers/coalesces notifications, pauses the decoder while an endpoint is absent,
and rescans every500 ms. It retains the player/source/position/loop/volume and
caller pause intent; recovery rebinds the device without seeking or replaying the
source. Explicit device selection leaves recovery ownership with the caller.
Cancellation/destruction stop retries; decoder errors remain terminal.

Fresh native-media-output-recovery-headless-synthetic-20261012.json records actual Qt
movie/file-sound decoder/output tests at muted gain with injected topology:
initial absence without advancement, rescan without notification, active loss,
frozen decoder position, retained caller pause, explicit ownership and cancel.
Three complete44100-frame PCM outputs and all20 movie RGBA match exact fixture
hashes. The raw manifest-tokenizer failure and earlier scope remain retained.
Fresh native-media-output-recovery-public-pass-20261012.json completes both
1777/1004-frame native intros with2476804/1393283 decoded PCM frames,12 native
voice plays/116 ACK, native WAV/CD/menu/render/campaign and normal Quit.
DSOUND remains disabled by default; errors/unsupported/fallback are0, all
source/input/original checks and owned Wine cleanup pass. The healthy endpoint
has no losses/recoveries; this is composition evidence, not physical recovery. Physical unplug,
distinct nonnull default-device changes, driver faults without topology, audible tail/latency and original equivalence
remain separate. Initial no-device voice admission still fails strictly.
Exact d94bd66f history reviews26 transitions without receipt gaps. Native
manager/config/cache/events, long playlist and broader gameplay remain pending.

Wider casting/combat remains unvalidated. The software attempt failed at native
menu lease retirement with about1.66FPS; the isolated NVIDIA attempt sustained
about20–72 nativeFPS but failed to produce the requested original Zombie.
Both reports retain stable sources/original manifests and owned Wine cleanup,
and are preserved under native-audio-casting-combat-{software,hardware}-failure-20261012.json.
Their stopped voice channels show ACKs and0 adapter failures/fallbacks/unsupported
calls; these partial observations do not validate casting/combat or diagnose its
input failure. No successful execution evidence is registered from either run.

The scenario level was corrected from evidence kind native_integration to the
coverage schema level headless before this fresh declaration/execution. The
earlier successful synthetic record retains its original claim fingerprint;
no historical hash is rewritten to match the correction.

## Actual native media software endpoint recovery (2026-10-12)

Fresh native-media-backend-private-endpoints-20261012.json tests actual Qt movie
and file-sound recovery with an owned private PipeWire/Pulse graph, stock configs
under unique names, no automatic sinks/restore modules, policy-only WirePlumber,
private runtime/config/state/cache and no desktop session bus. It verifies the
owned server and empty graphs before/after; all owned daemon groups stop.

Actual initial absence, endpoint arrival, active removal, paused replacement
and distinct nonnull default switch pass with1 loss/2 recoveries/1 switch and
one media acceptance. Both preserve exact66150-frame decoded PCM; the movie
preserves all30 RGBA frames. No source replay or logical-position/caller-pause
loss occurs. Gain is muted; these software endpoint/decoder observations do not
establish physical hardware, audible waveform/tail or original-driver equivalence.
The existing production playback source is unchanged from1a5aa59d.

Native voice backend/Restore/consumed-timeline actual endpoint validation,
initial no-device voice admission, native manager/config/cache/events, wider
casting input and long original listening/playlist scenarios remain pending.
Exact1a5aa59d history reviews22 transitions without receipt gaps.

## Actual native voice software endpoint recovery (2026-10-12)

Fresh native-voice-backend-private-endpoints-20261012.json records33 real
version5 native host requests with one expected DSERR_BUFFERLOST Restore result.
It loads/compares576000 native WAV bytes, creates/uploads/duplicates static PCM,
retains root45000Hz/clone40000Hz and primary-2000 gain, and preserves IDs/ready
through actual endpoint removal, replacement and distinct default switch.
Consumed byte cursor32960 advances to36368 before loss settles, stays frozen,
then resumes to70928. Status carries playing|looping5 and lost bit2 truthfully;
release/stop retires ready0. Graphs start/end empty and owned daemons stop.

The initial test's incorrect looping status1 expectation is retained; the
corrected test passes without production source changes. Current-epoch output
counters are generated7520/consumed5119. This host software-endpoint result
does not establish PE32 ABI, audible waveform/tail/latency or original driver
equivalence. Initial no-device voice admission remains strict; native
manager/config/cache/events, wider casting/input and long playlist/listening
remain pending. Exact5285cf12 history reviews12 transitions without receipt gaps.

## Initial absent output

Native voices, movie/file-sound decoders and music now start silently when the
initial default output is absent and attach automatically when it appears.
[Scoped evidence and unresolved full-game launch](native-audio-initial-silence.md)
separate exact private-endpoint tests and healthy full-game validation from the
empty-output game launch that currently stalls before hooks initialize.

The final empty-output whole-game run now passes complete native intros, audio,
CD music and normal Quit without any physical output. Optional Wine installer
dialogs in the isolated empty-cache prefix caused the earlier pre-hook stalls;
the reproducible private fixture suppresses those installers. See the fresh
[empty-output evidence](native-audio-initial-silence-empty-public-20261012.json).
