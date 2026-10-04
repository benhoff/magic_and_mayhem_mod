# Audio source filename boundary and native compatibility

The four commented sound names found by the installed manager sweep are not a
mistake in `sourceEntryName`'s backward scan. The pinned original code retains
the last apostrophe before a semicolon. This chunk preserves that reconstruction
and adds an explicitly selected native adaptation at the file-open boundary.

## Recovered evidence

Run `./tools/export-audio-filenames.py`. Evidence:
`working/decompiled/audio-support-ntj52luj/manifest.json`, `source_cache.asm`,
`source_pool.asm`, `wave_open.asm`, `file_open.asm` and `crt_fopen.asm`.
No-CD PE32 SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The exporter verifies originals before/after and checks the executable hash;
no executable is modified or launched.

At `0x56f700–0x56f712` the loader scans backwards from the first semicolon to an
apostrophe and writes NUL at `quote + 1`. It concatenates the retained string
between the sound root/backslash and `.wav`, then calls upload at `0x56f7d6`.
Upload calls WAV open `0x58fbb0` at `0x5701bf`. WAV open passes its filename to
file wrapper `0x4a1360` at `0x58fc2c`. The wrapper passes the original filename
unchanged to its CRT fopen wrapper `0x59cae2` at `0x4a139a`; its later CD-root
retry also copies the filename without dequoting it. The selected file-wrapper
body contains no apostrophe removal.

Confidence is high for these static instructions and argument flow. This does
not prove that historical Windows profile strings, all CRT internals or original
live sound calls have the same outcome as the current native/Wine observations.
The Wave wrapper loads through the file helper before memory-backed WinMM work;
assuming `mmioOpen` itself repairs the filename would skip this earlier boundary.

## Explicit native adaptation

`NativeSourcePathPolicy::literal` remains the `NativeManagerBackend` constructor
default. `NativeSourcePathPolicy::dequoteMissingLeaf` opts into a narrow native
policy shared by WAV opening and file-size reads:

1. Attempt the literal path first. An existing apostrophe-bearing filename wins.
2. Retry only if that lookup returns `notFound` and the final leaf is a nonempty
   single-quoted basename immediately followed by `.wav` (case insensitive).
3. Remove that pair of apostrophes from the leaf and resolve/open it through the
   same trusted-root AssetStore. Directory components and other names are unchanged.

Ambiguity, traversal and other resolver failures remain failures. Non-WAV,
unbalanced-quote and genuinely missing payloads are not repaired. Profile reads
remain literal. The recovered comment scan, generic asset resolver, game INI and
installed files are unchanged. This is a native adaptation, not newly recovered
original behavior or a live hook.

Reproduce the adapted installed sweep with:

```sh
./tools/test-installed-audio-manager.py --dequote-source-leaf
```

Literal baseline evidence remains in
[the installed manager sweep](installed-native-audio-manager.md).
Adapted evidence: `working/tests/installed-audio-manager/run-lw_hoy9c/report.json`.
All 995 cases complete: **926 independent exact PCM previews**, **344 one-shot
completions**, all **168 group selections**, overlap/gain and cleanup/restart
checks. IDs **812, 813, 1017, 1018** now load the existing `picview1.wav`,
`picview2.wav`, `Death.wav` and `Ranged combat.wav` and match their full committed
PCM plus independently computed previews. There are 69 expected failed attempts:
67 direct logical group names and two attempts to open the missing Stream file.

All 39 current audio/assets CTests pass:
`working/tests/audio-native-manager/run-dqo4pvp3/report.json` and `sanitizer.log`.
Fixtures check the literal default, adapted commented-name load and stat, literal
file precedence, missing/unbalanced/non-WAV/traversal failures and ambiguity.
Installed sanitizer evidence:
`working/tests/installed-audio-manager/run-z74qonqw/report.json`.
Address/undefined/leak checking remains enabled; prebuilt Qt/system libraries are
not instrumented. Input hashes and original-manifest guards pass before/after.

## Stream asset finding

ID **1016** names `Stream`. No `Stream.wav` exists in the selected installation.
The recorded historical unshield log tests all 356 WAV names and extracts 1,937
files; it contains the four normalized names above and no `Stream.wav`.
The exporter preserves and hashes that log (`cabinet-list.log`) and records
cabinet SHA-256 `87e7db3c613dfbc6224fb85b00ce7334dc74983b10baa22bd23082f607ce6f71`.

`unshield` is unavailable in this environment, so this evidence uses the historical
extraction log, not a fresh cabinet listing. Cabinet-wide/media-wide absence is
not established. No replacement, alias or fallback sound is guessed. Stream
admission still returns the documented missing-file failure and cleanup succeeds.

The next offline chunk can preflight and report playable catalog entries and
unresolved assets at manager startup. The application can then explicitly select
native compatibility and surface the missing Stream asset before playback; live
caller ordering, audible behavior and replacement remain separate milestones.
