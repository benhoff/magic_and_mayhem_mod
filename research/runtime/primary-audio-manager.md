# Primary audio and manager lifecycle

Reviewed 2026-10-04. Static reconstruction and native/x86 fixture validation;
no live game or audible equivalence claim.

## Recovered blocks

Reference: No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
`python3 tools/export-primary-audio.py` exports hash-guarded assembly and verifies
2927 immutable original files before and after. Evidence:
`working/decompiled/audio-support-16h1kfgo/`, including manager initialize,
shutdown, disable, primary controls and lifecycle helper exports.

| Block | Confirmed behavior |
| --- | --- |
| Startup `0x0056e4ff`, `0x0056e520` | Save primary GetVolume into manager +0x258, then looping Play. Nonzero HRESULT fails initialization. |
| GetVolume `0x0056fd10`; SetVolume `0x0056fd30` | Gate on +0x1c only. Disabled getter leaves caller output untouched. |
| Start `0x0056fd50` | Gate on +0x1c, skip if +0x18 already set, Play(0,0,1), set +0x18 only for zero result. |
| Disable `0x0056fd90` | Require both gates; stop/reset selected scheduled voices, clear records, clear +0x18 before primary Stop. Return primary Stop result. |
| Shutdown `0x0056e640` | Gate on +0x1c; restore saved primary volume first, ignoring its result. If active, retire selected records and stop primary. Release owned voices before device. Clear initialized state and return zero. |

For nonempty scheduled records, select looping deadline `0xffffffff` or unsigned
`now < deadline`. Query the clock separately for each nonloop record. Expired
records are cleared without Stop. Disable queries secondary status; failed query
or playing status attempts Stop, and only successful Stop attempts zero reset.
Secondary failures do not abort record clearing or traversal. Output slots are
cleared; list links are retained, deadline/voice/output become zero, volume becomes
-5000, coordinates become -1. The primary error is retained at the modeled
last-result slot corresponding to `0x006f4370`.

`reconstruction/audio/manager_contract.*` models these selected blocks with
host annotations, not a binary manager layout. Shutdown delegates wrapper/list
retirement at `0x004de090` and accepts a caller-supplied buffer release inventory.
It does not reconstruct that entire callee, engine list allocation, heap cleanup,
outer initialization failure cleanup, or full COM primary/device destructor
ownership. The lifecycle helper export contains recursive wrapper release;
it does not alone establish complete primary ownership. Confidence is high
for the listed static branches and ordering, conditional for broader lifecycle.

## Native controls

Device-wide primary state is separate from the metadata BufferId API. The
native mixer applies primary attenuation to the summed stereo samples before
final clipping. Volume accepts -10000 through zero hundredths of a decibel;
-10000 is the existing native hard-mute policy. Muting advances secondary
voices normally. Initial native volume zero is a policy, not a measured original
system volume. Original primary volume affects hardware wave volume; this
implementation affects only its own Device. [Microsoft primary-buffer behavior](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/bb318673(v=vs.85)).

Primary Play requires looping flags. Primary Stop clears explicit primary play
intent and does not mute or pause secondary voices. At the game's priority
cooperative level, secondary playback can keep the primary mixing;
[Microsoft Stop semantics](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/mt708940(v=vs.85))
explain why blanket muting would be incorrect. Primary status reports explicit
play intent, consistent with the [current Wine primary implementation](https://raw.githubusercontent.com/wine-mirror/wine/master/dlls/dsound/primary.c),
not proof of the installed Wine version or 1998 hardware behavior. Manager
Disable stops selected secondary voices itself.

Requested primary PCM format is validated and stored as metadata. It does not
change the already negotiated Qt output clock or open/reconfigure QAudioSink.
Primary caps byte length remains zero scaffolding. Primary pan, unsupported
cursor/seek/frequency contracts, device changes and full hardware emulation
remain outstanding.

The PE32 proxy routes volume, Play, Stop, status and SetFormat through the host.
Channel version is now 2 (magic `MNMAUD01`, unchanged 128-byte header and 16 MiB
payload). Old producers/hosts are rejected; rebuild both sides together.

## Offline validation

```bash
python3 tools/test-audio-bridge.py
python3 tools/test-audio-staging.py
```

Neither command launches Chaos.exe. The first uses synthetic samples and a
silent native host plus x86 Wine COM fixture. The second consumes a guarded
working executable for disposable staging and runs original-manifest checks.

Evidence: `working/tests/audio-bridge/run-u_wkspql/report.json`.
All 12 audio/asset CTests pass. `audio-primary-test` covers 36 initialization
combinations, 32 disable combinations and eight shutdown combinations, including
positive nonzero failure results and pre-Stop flag clearing. Wire-to-PCM tests
check attenuation before clipping, mute, restoration, format rejection and
primary Stop while secondary voices continue. The PE32 fixture checks COM
slots, master volume round trip, invalid volume/format, required looping,
secondary state after primary Stop and existing upload/duplicate/lifetime paths.
The stopped-host fixture checks timeout and permanent retirement.

`working/build/audio-output/audio-primary-sanitize` passed AddressSanitizer,
UndefinedBehaviorSanitizer and LeakSanitizer outside sandbox tracing. Qt shell
rebuilt; help/startup and primary-control tests passed. Guarded staging passed:
`working/tests/audio-staging/run-hdjjr9o6/report.json`, production DLL SHA-256
`7adfdaac172bc66e6890855fd6dd7482c87021685b84224e7426c2323954ad1e`.
Source and staged hashes were checked and all original files verified before
and after.

No game was launched or audio device opened for this fixture validation.
Live initialization, scheduled retirement, transitions, audible output and
original timing still need separate evidence. The manager reconstruction is
not injected into the original engine; selected primary COM calls use the
native implementation when the optional adapter is selected.

Selected retirement, wrapper destruction and reusable-ring transforms now have
[separate offline reconstruction](audio-voice-lifetimes.md). Full shutdown
allocation and source-list ownership remain outside this manager model.
