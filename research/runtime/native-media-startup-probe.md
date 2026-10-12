# Native media startup availability

Pinned NoCD build40209ca7 imports `CoCreateInstance` at0x5c5318, rather than
AMCreateMultimediaStream. The movie setup uses that import at0x46970f. A separate
startup check at0x483f30..0x483f7b calls CoInitialize(0); a negative HRESULT
returns0 immediately. Otherwise it creates in-process, nonaggregated FilterGraph
CLSIDe436ebb3-524f-11ce-9f53-0020af0ba770 with IGraphBuilder
IID56a868a9-0ad4-11ce-b03a-0020af0ba770. Nonnegative creation releases the returned
object and selects1; failure selects0. Both paths call CoUninitialize.
Disassembly, import/identity bytes and the selected startup call at0x4e95d1
support these findings with high confidence within those instructions. Actual
COM implementations, errors, codec availability and indirect callers remain
separate. `native-media-startup-original-20261012.json` records49 signed HRESULT
combinations and121 exact scripted COM events through the unchanged75-byte
caller. The first sandbox execution was blocked by SIGSYS; the permitted
isolated32-bit execution passes. Original manifests verify before/after.

The fresh module-disable diagnostic is retained in
native-audio-filtergraph-probe-failure-20261012.json: Wine reports failure to
load Quartz for that FilterGraph class, followed by native menu timeout. Sources
remain unchanged, the original manifest verifies and the private Wine session
stops. This localizes a remaining initialization dependency; it does not prove
all Wine audio APIs or COM calls retired.

The native policy replaces only the guarded direct startup call with
a media-broker availability acknowledgement. It returns unavailable on a missing
or frozen broker without constructing a legacy graph. Native movie decode and
output failure remain explicit. Installed native ownership also refuses a
preacceptance movie replay through the original wrapper. Compatibility fixtures
retain their separately tested original fallback.

Media wirev1 adds operation4 without layout changes. Availability requires
empty path/zero flags and responds complete without creating a player or
changing movie/input/lease ownership. The producer saves LastError; its
no-argument assembly wrapper preserves all registers except boolean EAX and
restores flags. Both the selected E8call bytes and movie prologue are guarded
before installation; acquire the startup page before mutating the movie, and
restore protection afterward. No generic CoCreateInstance interception occurs.

`native-media-startup-synthetic-20261012.json` records14 broker requests and real
PE checks for call-byte drift, all preserved registers, carry/stack/LastError,
successful availability, unavailable host and strict missing/stale movie
refusal. It renews all10 exact decoded RGBA frames/22050 PCM frames for controls
1/-1/256 and file-sound compatibility. The initial naked-test helper compile
failure remains in working evidence; its corrected fixture passes.

Marked native media plus native factory staging now disables Wine DirectSound
by default. Legacy/no-media/unmarked staging keeps its prior module policy.
MNM_NATIVE_DISABLE_DSOUND0 keeps the prior override;1 forces a diagnostic
disable. Ten audio-admission and four media-admission regressions pass, including
override ordering and malformed probe policy. The runner writes an owned,
safe launch receipt of actual module policy before Wine; it records no full
environment. Full default module-disabled campaign validation passes in
`native-media-startup-default-public-pass-20261012.json`: both intros, native
voice/WAV/CD/music, campaign and normal Quit, with actual DSOUND disable
confirmed and no explicit module-disable option.
Prefix registration may still attempt FilterMapper2 creation and log a blocked
Quartz dependency; this is distinct from the selected game FilterGraph check.
Unsupported WinMM sounds, arbitrary COM callers, physical waveform/listening,
broader scenarios and an executed Wine API census remain separate.

The separate recovered model in reconstruction/audio/media_startup_probe.hpp
implements only the two signed return gates. Fresh native-media-startup-return-
model-20261012.json binds that model and the unchanged original caller in the
49-case comparison; COM event order remains an independent original instruction
observation. The earlier original-only report retains its exact source hashes.
