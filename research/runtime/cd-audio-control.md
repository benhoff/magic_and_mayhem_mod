# Original CD audio control discovery

Build-specific static findings from pinned No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Guarded read-only `tools/export-cd-audio.py` produced
`working/decompiled/audio-support-mqt72ra8/manifest.json`, with unchanged source
hash, exported assembly/direct callers and immutable manifest checks before and
after. Export range labels are provisional; the raw instructions establish the
selected findings below. Confidence is high for these instruction arguments,
while original execution and behavior equivalence remain unvalidated.

| Entry | Selected static finding |
|---|---|
| `0x481330`, `0x481770` | Initialized gate, optional stop, mixer-array destruction and MCI_CLOSE0x804 with WAIT2; clear initialized only on MCI success. The first export label `cd_open` is provisional and does not mean this is an open operation. |
| `0x481490` | Store caller window; MCI_OPEN0x803 with flags0x2102 or0x2302, output device; MCI_SET0x80d with flags0x402 and time format10; enumerate mixer devices and publish initialized state. SEH/allocation/full initialization remain pending. |
| `0x481930` | Active/initialized gates; MCI_PAUSE0x809 or RESUME0x855, WAIT2; paused state updates only on successful API return. |
| `0x481dc0` | Active/initialized gates; MCI_PLAY0x806 with flags0xd; use low8bits of supplied track as FROM, low8bits of track+1 as TO. On failure, query STATUS NUMBER_OF_TRACKS3; when count equals supplied track, retry PLAY with flags5. |
| `0x481ed0` | PLAY0x806 with flags5 and low8bit FROM; clear paused state only on successful return. |
| `0x481e90` | STATUS LENGTH1 with flags0x102; return lowbyte of API result field on successful query. |
| `0x481f30` | Stored playlist count/index, notification flag and repeat flag decide next track, repeat first or return-3; calls the existing track-play function. Full notification wiring remains pending. |
| `0x482010` | STATUS MODE4, stop/fade branch and mixer volume reads/writes before MCI_STOP; full failure and fade timing remain pending. |
| `0x482190` | Volume input clamps above15; zero/negative stop and disable; nonzero enables. Writes `level*0x1111` to each configured mixer. The native target will scope this volume to its music output rather than the desktop mixer. |
| `0x4822a0` | Disc/file validation using MCI length/mode, path construction, file-size checks and close. Export label `cd_play` is provisional; it is not a track-play assertion. |

The manager's MCI API slot is`0x5c52dc`. Selected manager fields are
initialized+0x32c, active+0x328, paused+0x330, device+0x33c,
callback/from/to+0x368/+0x36c/+0x370 and status callback/result/item/track at
+0x378/+0x37c/+0x380/+0x384. These offsets are local original object fields;
no process pointer or object layout is a native host wire contract.

Local Wine `mmsystem.h` defines the command/flag names used above. Command
numbers and DWORD arguments are independently visible in the original export;
header definitions do not establish original runtime frequency or caller scope.
Unknown callers, indirect MCI uses outside these ranges, full catalog/lifecycle,
notifications, error paths and actual driver timing remain pending. Native CD
asset extraction is [documented separately](../formats/cd-audio-assets.md).

## Bounded original control comparison

`AU.cd_track_controls` separately owns whole track-play0x481dc0,
play-from0x481ed0, pause/resume0x481930 and length-byte0x481e90. The native
models are independent of widgets and device output. A
[fresh original comparison](native-cd-track-controls-original-20261012.json)
executes the unchanged original bodies with an authored receiver and scripted
MCI callback, against native control models:6,912 cases, exact command/flag/
argument order, returned DWORDs and all receiver/guard bytes. Gates include
nonzero raw words; tracks include0/1/2/13/255/256/UINT32_MAX/high words; errors
include positive nonzero codes and HRESULT-shaped values. The full function
bytes, source/input hashes and immutable manifests remain unchanged.

Successful last-track retry returns before the original paused-field clear;
64 matrix cases verify that this retry preserves a nonzero paused annotation.
Ordinary successful play and play-from clear it; pause/resume updates it only
on success. Length query bypasses initialized/active gates and returns a byte,
with zero on error. These details are recovered behavior, not native music
policy. Original driver output, actual windows/notifications, caller frequency,
SEH initialization, allocation/catalog, mixers/fade/playlist and live playback
remain excluded. The first sandbox execution stopped with SIGSYS before a
comparison result; its failure is retained and no pass is inferred from it.

All61 audio/assets CTests pass after integrating the native CD primitives into
the reconstruction build. Five CD extraction fixtures pass. The shared audio
CMake change leaves prior live-audio evidence historical where its source
fingerprints include that file; no old hash is refreshed. Fresh live routing
validation belongs to the upcoming native music integration milestone.

Live native music revealed two refused shutdown calls. The retained static export
confirms `0x48214d` pushes flags0 before MCI_STOP0x808 at `0x48214f`
and the `0x5c52dc` call at `0x482155`. The native adapter must accept STOP0
as well as WAIT2. This is a confirmed argument finding; full stop/fade/mixer
function equivalence remains unvalidated.

The retained0x4818b0..0x481920 assembly shows initialized+32c/active+328
gates and mixerCount+340. Zero mixers leave the queried level0, even when the
native worker has nonzero gain. A guarded native getter uses a7-byte prologue
(`51 8b 81 2c 03 00 00`), thiscall ret4/output pointer and the same local gates,
then reads its own fresh coherent music-volume snapshot. This is per-output
native policy, not original desktop-mixer equivalence. See native-audio-defaults.md
for the actual PE getter and shared-page protection checks.
