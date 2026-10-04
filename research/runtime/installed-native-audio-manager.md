# Installed catalog through the native audio manager

`./tools/test-installed-audio-manager.py` exercises the installed catalog through
`NativeManagerBackend`, the reconstructed manager lifecycle/upload/admission
contracts, Qt-backed file loading and native PCM storage/mixing. It launches no
game, installs no hook and opens no sound output device. This is offline native
integration evidence; it does not validate original audible behavior.

## Reproducer and evidence

Default input is `working/game-nocd/Sounds`. The unchanged 15,672-byte
`Sounds.ini` has SHA-256
`72f9c5b34a3325976562247bb16be1e720ad47636a7d546c2b31df171ce1c412`.
The runner verifies the original manifest before/after, including failure paths,
and hashes all 356 installed WAV files before/after to check input preservation.
It uses Python's independent INI/WAV readers for expectations, not native loader
output as the PCM oracle. Evidence stores sources/binary/input hashes, native
case records, return codes, diagnostics and per-case signed stereo dumps.

Normal run: `working/tests/installed-audio-manager/run-byq62cge/report.json`.
Address/undefined/leak sanitizer run:
`working/tests/installed-audio-manager/run-eqd8xq60/report.json` and `native.log`.
The latter uses the same runner's `--executable` option with an instrumented
fixture; leak checking remains enabled. Native manager, reconstruction, asset
and PCM implementation sources are instrumented; prebuilt Qt/system libraries
are not. The report hashes every successful case's PCM dump.

The regular audio/assets regression suite also passes all 37 tests:
`working/tests/audio-native-manager/run-3t4n60ym/report.json`.
No production behavior was changed in this chunk. The new CMake executable is
an offline fixture linking the native manager and Qt Core, without audio output.

## What the sweep checks

A controlled clock (100) and caller-supplied RNG initialize the installed
412-source/69-group manager at its configured limit of 12 schedules. The output
clock is 48,000 Hz; the recovered primary format request remains metadata.
Installed map-1/map-2 preload sections are empty, yielding 22 dynamic source
nodes and zero pinned bytes. These inputs do not exercise real permanent preloads.

The sweep has **995 cases**:

| Stage | Cases | Scope |
|---|---:|---|
| Direct upload | 412 | `recycleSource` for every catalog ID, explicitly bypassing group resolution; full committed PCM hash/byte count and recovered duration arithmetic checked for successful files. |
| Admission | 412 | Every catalog ID through cache/voice admission with RNG zero, looping playback and caller-slot publication/retirement. |
| Group members | 168 | Every member of all 69 groups selected through admission using each deterministic RNG index. |
| Overlap base, duplicate, master gain | 3 | Two distinct installed voice identities, rewind, summed/clipped output and primary attenuation -2000. |

**918 successful cases** match an independent 256-frame stereo PCM expectation
(235,008 frames / 470,016 samples), including sample conversion, rate conversion,
looping, overlap/clipping and master gain. These are exact preview matches, not
an independent comparison of every resampled frame in each full one-shot.
All **340 successful direct uploads** additionally drain to completed status and
then produce silence. Full committed input PCM hashes are checked independently.

Between admissions the fixture retires schedules, stops/resets their voices and
clears stable caller slots. The final shutdown clears both overlapping slots,
releases source/duplicate storage, retains only the singleton primary and mixes
silence. Map-2 startup reuses that primary; destruction clears schedules and
retains one host-owned primary pending backend/device RAII cleanup.

## Failures and remaining compatibility gaps

**77 failed attempts** are observed and checked as file-open failures with
`E_FAIL`, rather than counted as PCM matches:

- 67 direct uploads use logical group names with no WAV. Normal admission resolves
  these IDs to group members; all 168 selected group-member cases succeed.
- IDs **812, 813, 1017, 1018** fail both direct upload and admission (eight attempts).
  The profile's inline comments leave matching apostrophes in the processed leaf:
  `'Picview1'`, `'Picview2'`, `'Death'`, `'Ranged combat'`. Corresponding unquoted
  WAV filenames exist. This is a native pipeline compatibility gap, not a missing
  installed payload and not evidence that the original engine fails identically.
- ID **1016** (`Stream`) fails direct upload and admission (two attempts).
  `Stream.wav` is absent in the selected installation. No guessed fallback or
  synthetic replacement is introduced.

The four quoted paths require further evidence at the recovered profile/comment
and original filename-open boundaries before changing reconstruction or choosing
an explicit native adaptation policy. The original `sourceEntryName` model is
preserved; permissive filename dequoting is not silently added to AssetStore.

Confidence is high for these native outcomes, independent PCM comparisons and
cleanup under the controlled fixture. Actual original schedule timing, tuning
conversion, nonempty installed map classifications/preload budgets, COM/primary
ownership, active-map changes, live thread/cadence and audible replacement remain
unverified. No game configuration or gameplay balance was changed.

The [filename boundary investigation](audio-source-filenames.md) confirms the
selected recovered scan/file-wrapper behavior and provides an explicit native
compatibility mode for the four quoted leaves. Literal-default evidence above
is preserved; Stream remains a missing payload.
