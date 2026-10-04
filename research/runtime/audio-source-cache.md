# Source-cache selection and destructive replacement

Reviewed 2026-10-04. Selected `0x0056f400` cache decisions and `0x00570140`
replacement/upload ownership reconstructed offline. No live game replacement.

## Evidence and confidence

`python3 tools/export-source-cache.py` exports source-cache, source-release and
static upload assembly, plus four profile/path strings, from the hash-pinned
No-CD PE32 executable:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Evidence: `working/decompiled/audio-support-_p0gupwd/manifest.json`,
`source_cache.asm`, `source_release.asm`, `static_wav_upload.asm` and
`source-cache-strings.json`. All 2927 original files verified before and after.
No executable was modified or launched. An initial export rejected a string
in a section's virtual tail; the completed exporter identifies it as
zero-initialized virtual storage rather than reading unrelated file bytes.

Confidence is high for the selected integer branches, ownership writes and
call ordering. Profile/WinMM implementation internals, manager configuration
and live timing/ownership remain separate contracts.

## Sound ID versus metadata ordinal correction

The caller sound ID remains in EBX in the loader, and in ESI in the admission
path after group resolution. Wrapper +4 is compared with that ID; upload
writes that same ID to +4 at `0x00570338`. The bsearch table ordinal is stored
separately in scratch `0x006f4328` and selects the class from manager +0x24c.
It does **not** replace the sound ID. Nonconsecutive IDs must remain distinct
from their positions in the sorted table.

The earlier admission model incorrectly returned the ordinal after a source
table hit. `admissionSoundId` now retains the resolved ID; an unknown admission
ID still falls back to ID `0x19a`. The loader itself has no such fallback:
an unknown ID returns `0x80004005`. `VoiceWrapper::sourceIndex` remains the
historical annotation name, with an explicit comment that its value is an ID.
Admission fixtures now store nonordinal IDs, and the new integration loads
ID 20 with class-table ordinal 1. [Admission evidence](audio-voice-admission.md)
has been corrected accordingly.

## Loader and cached hits

Disabled manager +0x1c/+0x18 gates return zero, writing manager +0x254 to a
supplied output. A group-ID hit recursively loads **all** members in order,
with no member output and no RNG. An empty group returns zero. The first member
failure stops traversal and becomes `E_FAIL`; already loaded members remain
loaded. Decoded cyclic group graphs reject explicitly rather than exhaust
the original stack.

A nongroup ID must exist in the sorted source-ID table. Search the source
ring forward from its head for the first wrapper whose +4 matches the ID.
GetStatus success without `0x2` returns zero immediately, even if currently
playing. This cached hit does not write the caller output or promote the head.
A successful buffer-lost (`0x2`) query writes +0x0c = 2 and enters replacement
selection; query failure also enters selection. The original issues this
query even for a matching null buffer; backend invalid-buffer semantics remain
observable. Healthy looping status `0x5` is not a lost buffer.

## Backward cache selection

`0x0056f56c..0x0056f69c` starts at the source-ring tail (+0x18 from head),
then visits previous links. Initial rank is 1 and advances for every record,
including skipped records. Compute the unsigned divisor and step before scanning:

```
divisor = uint32(manager.field234 - manager.field22c)
step = 240000 / divisor
```

The physical meanings/initialization of these two manager fields are not yet
established. Zero divisor is an original divide fault; the model rejects it.

- +0x0c = 0 pins the record: no status query or score.
- A buffered record is available only if its own status query succeeds without
  playing bit `0x1`, and every buffered duplicate is likewise idle. Query
  errors count as busy. Bufferless duplicates skip querying. A bufferless
  **root** skips the entire descendant check, preserving the original branch.
- Available +0x0c = 2 returns immediately, ahead of prior scored candidates.
- Otherwise compute the wrapping DWORD score, interpreted as signed:

```
score = int32((5000 - requestedVolume) * 18 + rank * step + field08)
```

Lowest score wins only when strictly below the prior best, initially 999999.
Ties retain the first backward candidate. It is possible to reject every idle
source because none beats that ceiling. +8 receives the helper-reported sample
byte count on upload; this is not a conventional LRU policy or native cache
improvement. Requested volume (+0x10), not cached volume (+0x14), enters the
score. No Stop, position reset or scheduler clear occurs in this selector.

## Profile path and destructive upload

After selecting a candidate, query a numeric decimal ID key in profile section
`Sounds`, with output capacity 260 and manager +0x124 as profile filename.
The prior file-API audit confirms the call through IAT `0x005c507c` as
`GetPrivateProfileStringA` in
`working/tests/file-api-audit/run-x62yzy6t/nocd/report.json`.
Default pointer `0x005faf28` is zero-initialized in the image's virtual tail;
later runtime assignment is not established by this export. A zero returned
count fails without destroying the candidate.

The first semicolon in the returned value triggers a backward scan to a
preceding apostrophe, then truncation immediately **after** that apostrophe.
This is not generic INI comment stripping or quote removal. Malformed scans
and fixed-buffer overflow reject in the host model rather than emulate unsafe
original reads/writes. Windows profile quote/default/truncation behavior is
delegated to `profileValue`; no Qt profile parser is implemented here.

The path concatenates manager +0x20, `"\\"` at `0x005d9058`, the processed
value and `".wav"` at `0x005f0448`. Preserve the decoded root and original
concatenation; native AssetStore resolves explicit Windows prefixes and casing.

`recycleSource` reconstructs the selected replacement sequence:

1. Release/free descendants from the end of the duplicate chain, then release
   the old root buffer and reset its contents. Keep root identity and source
   links. If the root was already bufferless, the original skips its reset.
2. Open the replacement WAV. Failed open closes the WAV wrapper and returns
   `E_FAIL`; old storage stays destroyed. The original message-box diagnostic
   is outside this model.
3. Get data size and format, create the `0xea` static secondary, Lock the full
   first region at offset zero, read data and Unlock with the data helper's
   reported byte count. Second regions/flags remain absent. `Descriptor32`'s
   format address is a host placeholder here; format travels separately.
4. Create/Lock/Unlock error with a valid returned buffer releases that new
   buffer, zeroes the root buffer, closes WAV and returns the exact error.
   The original unconditionally dereferences the buffer during cleanup. Failed
   creation with a null output is an unsafe original fault domain: the model
   throws explicitly, without pretending a normal close/return occurred.
5. On success get duration and store reported bytes, caller sound ID, class
   selected by table ordinal and duration; close WAV. Only then promote the
   source head and publish the output. Failure never promotes/publishes.

The selected WinMM data helper returns its requested/bounded chunk length,
not the `mmioRead` transfer result. The model preserves the helper boundary;
it does not claim exact unchecked short-read, allocation or malformed-format
behavior. Original SEH scaffolding and WinMM chunk parsing remain delegated.
`uploadStatic` remains a separate strict native policy with safer cleanup.

## Validation and integration boundaries

Run `python3 tools/test-source-cache.py`. Evidence:
`working/tests/source-cache/run-kqtk07o1/report.json` and logs. All 22 current
audio/asset CTests passed, including the corrected admission fixtures. An
independent index/arithmetic oracle covers 625 backward-selection combinations,
score wrap, rank weighting, pinned/idle/busy/lost states and query errors.
Additional fixtures cover score ceiling/ties, duplicate guards, bufferless
branches, cached/disabled outputs, missing profile/ID, group traversal/cycles,
destructive open failure, valid-buffer Create/Lock/Unlock failures, unsafe
null cleanup rejection, descendant disposal order and metadata publication.

A generated WAV goes through the existing Qt-backed AssetStore and strict
WAV reader, using an explicit Windows root prefix and mixed-case request.
The cache loads ID 20/class ordinal 1 and the admission adapter plays the
source plus a duplicate. Exact stereo PCM is `[8000,8000]`; a playing duplicate
blocks recycling of its stopped parent, then stopping it permits selection.
Teardown releases both buffers and owned sample storage. This adapter exists
only in the fixture; no runtime hook was installed.

`working/build/audio-output/audio-source-cache-sanitize` passes address,
undefined-behavior and leak sanitizers outside sandbox tracing, including
Qt-backed file integration. Fixtures read only generated files, not game assets;
they use no Wine, audio device or live game. Native services do not depend on
the recovered cache; reconstruction consumes backend interfaces. Valid linked
rings, acyclic unique duplicate ownership and coherent sorted catalog/class
tables remain preconditions. Next offline work: manager profile/table loading
and source-pool initialization, before any live adapter promotion.
