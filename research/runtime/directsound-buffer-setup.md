# DirectSound device, buffer and sample upload

Pinned working no-CD executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
All virtual addresses below apply only to this PE32 i386 image at base
`0x00400000`. Interface/object pointers remain launch-dependent.

`tools/export-audio-support.py` verifies that full hash before/after export and
records selected assembly windows, call-site contexts and direct callers. This
is static evidence, not a runtime frequency or complete-path coverage claim.
Successful evidence is
`working/decompiled/audio-support-f81pypl9/manifest.json`.

## Confirmed creation and upload paths

| Address | Observation | Confidence |
|---|---|---|
| IAT `0x005c501c`, thunk `0x00597566` | DSOUND ordinal 1 / DirectSoundCreate | High: imports, thunk and installed Wine SDK |
| `0x0056df60` | Audio manager initialization; substantial configuration work precedes device creation | High for observed call path; configuration not reconstructed here |
| `0x0056e496` | DirectSoundCreate(null GUID, manager +8 output, null aggregation) | High: pushed arguments |
| `0x0056e4d1` | Device vtable +0x18: SetCooperativeLevel(manager +4 HWND, 2 / priority) | High: pushes and SDK method order |
| `0x0056fe80` | Primary setup helper | High: assembly and independent call-order model |
| `0x0056fecf` | Device +0x0c: CreateSoundBuffer with size 20, flags 0x81, zero data length/format pointer | High |
| `0x0056fefb` | Device +0x10: GetCaps, structure size 96 | High |
| `0x0056ff71` | Primary +0x38: SetFormat | High |
| `0x0056ff8a` | Primary +0x0c: GetCaps, size 20; HRESULT ignored, byte length stored in manager +0x10 | High; byte value after failure is unknown |
| `0x0056ff9a` | Device +0x1c: Compact; its HRESULT becomes helper result | High |
| `0x00570140` | WAV-backed static secondary loading wrapper | High for observed success path |
| `0x00570289` | Device +0x0c: CreateSoundBuffer, size 20, flags 0xea, WAV data length/format | High |
| `0x005702af` | Secondary +0x2c: Lock(offset 0, WAV data length, flags 0), null second-region outputs | High |
| `0x005702ca` | WAV helper copies into returned first pointer, bounded by first-region byte count | High |
| `0x005702e1` | Secondary +0x4c: Unlock(first pointer, helper-reported bytes, null/zero second region) | High |
| `0x00572194` | Device +0x14: DuplicateSoundBuffer on an existing secondary voice | High |
| `0x0058ff10`, `0x005900d0`, `0x00590140` | mmio-based format, data-size, and sample-read helpers | High for selected operations; unchecked error behavior not fully modeled |

Method identities and constants were cross-checked against the installed
`/usr/include/wine/windows/dsound.h`. The 20-byte descriptor is the old
DSBUFFERDESC1 layout, not the modern GUID-bearing descriptor. Flags `0x81` are
PRIMARYBUFFER|CTRLVOLUME. Flags `0xea` are STATIC|LOCSOFTWARE|CTRLFREQUENCY|
CTRLPAN|CTRLVOLUME. The latter exposes the controls the next mixer chunk must
preserve; it is not permission to change balance or sound timing.

The primary format sets PCM tag 1, 22050 Hz, average byte rate 88200, alignment
4 and cbSize 0. Channels become 2 if DSCAPS_PRIMARYSTEREO (2), otherwise 1;
bits become 16 if DSCAPS_PRIMARY16BIT (8), otherwise 8. The fixed average and
alignment are not recomputed for those fallbacks. The reconstruction preserves
that discrepancy; the strict native implementation rejects inconsistent PCM
rather than inventing corrected original behavior.

Manager +8 holds the device, +0x0c the primary buffer, +0x10 its reported bytes.
The device is also published at global `0x006f4378`. The static loading path
uses descriptor `0x006f41d0`, WAVEFORMATEX `0x006f41f0` and a size/read-count
scratch value at `0x006f3da4`. It explicitly zeros format cbSize before creating
the buffer. Secondary wrapper +0 holds its COM buffer; +4 saves the first Lock
pointer after Unlock, +8 the helper's reported copy length, +0x0c the first
Lock-region length. Saving a pointer does not establish that it remains valid
after Unlock; native storage never retains that borrowed pointer as an asset.

The manager subsequently queries status and starts the primary looping near
`0x0056e520`. Shutdown/release paths are present at `0x0056e640`. Those playback
and scheduling operations remain outside this setup model.

## Ownership and implementation boundaries

DirectSound duplicate objects share sample storage and retain it until the last
reference is released; their playback controls are independent. This is the
documented [DuplicateSoundBuffer contract](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/mt708944(v=vs.85)).
[Lock](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/mt708932(v=vs.85))
returns write regions and requires Unlock; its pointers are not a stable sample
read interface. The native `audio::Device` therefore owns committed sample
vectors, gives each voice an opaque ID, shares samples for duplication, and
commits temporary writes through checked owner/ticket/region contracts.

Circular writes are prepared and tested as a native storage capability, not
claimed observed in the static WAV upload. Native shared-storage locks are
serialized. Release abandons pending writes by that owner; another duplicate
continues to own the previous committed bytes. Primary metadata, limits and
unsupported flag handling are explicit native policy, not full COM emulation.

`reconstruction/audio/dsound_setup.*` contains pinned request fields, primary
call order and successful static uploads. `audio/` contains independent native
storage and a strict RIFF reader. It does not intercept the game or implement
device output; the Qt media chunk and Wine DirectSound still operate as before.

## Offline checks and next work

`python3 tools/test-audio-buffers.py` builds the native libraries and compares
sample dumps with Python's independent WAV reader for all 356 installed WAVs.
Evidence: `working/tests/audio-buffers/run-b2dc_l93/report.json`;
11,908,772 PCM bytes matched exactly, including 8-bit mono at 22050 Hz and
16-bit mono at 11025/22050/44100 Hz. Original-manifest verification passed
before/after. Synthetic tests also exercise stereo, explicit 18-byte formats,
packed x86 layouts, primary error exits (including positive nonzero HRESULTs),
ignored primary GetCaps failure, shared duplicate lifetime, circular and partial
writes, invalid/repeated unlock, release while locked, and allocation limits.
The lifecycle suite also passed with AddressSanitizer, UndefinedBehaviorSanitizer
and LeakSanitizer in `working/build/audio-sanitize`.

The native offline pipeline now reads through Qt-backed AssetStore and
`audio::loadWave(AssetFile&)`, closing input before reconstructed upload.
Updated run `working/tests/audio-buffers/run-fbodze_l/report.json` again matches
all 356 WAVs / 11,908,772 PCM bytes through mixed-case relative and explicitly
mapped Windows paths. Five combined audio/asset CTests and original-manifest
checks before/after pass. See [input integration evidence](../formats/pcm-wav-loading.md).
The adapter preserves the parser and static upload policies; it is not a
reconstruction of original manager file loading or a live hook.

Confidence is high for the selected static contracts and offline ownership/
sample-byte behavior. Actual call coverage, COM reference counting, device
cooperation, audible output and live voice timing are unvalidated.

Selected [voice-control contracts](directsound-voice-controls.md) now cover
play/stop/loop, volume/pan, status/reset and deadlines with offline tests.
Next: independent native voice state, then an offline mixer with independent
PCM output expectations. Frequency/cursor extensions remain conditional. Only
after that should an isolated x86 adapter route observed game buffers into the
native mixer and Qt audio sink. No gameplay algorithm, configuration, binary or
audio balance changes were made in this chunk.
