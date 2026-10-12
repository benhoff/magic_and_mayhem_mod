# Selected native audio file sizes

Behavior `HOST.native-audio-file-sizes` is seeded before validation. The pinned
No-CD pool caller at0x56ed0f calls CRT stat0x59e270 with a buffer initially at
stack+0x3c. After cdecl cleanup, its successful branch reads stack+0x50 at
0x56ed1b: only output offset0x14 feeds the wrapped manager+0x230 budget. The
export `working/decompiled/audio-support-objgyz0h/source_pool.asm` still matches
its original manifest hash ec4d96291806426cee6e9174f21b6f66e0672c09e8c1956823171adb24b4fd74
and pinned executable40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168.
Confidence is high for the selected call, cdecl cleanup and consumed field.

The optional native service reads whole WAV file size through read-only
AssetFile, with the same bounded Sounds-root leaf and quoted-missing-leaf policy
as native WAV loading. It does not decode PCM. Zero and malformed WAV content
can therefore still supply file metadata. Values outside the DWORD domain fail.
Original stat structure contents, errno, arbitrary stat callers, filesystem
races and original selector/cache/manager/event rules remain outside scope.

MNMAUD01 version5 adds low capability64 and operation26 with no layout change.
A bounded ASCII leaf of1..259 bytes has no terminating NUL in the wire payload;
voice/value/PCM/flags fields are zero. Success returns whole-file bytes in word18
and no response payload; all metadata24..30 stays zero. The staged capability
must match exactly. Producer installation bit0x100000 records the selected
caller hook. Both the call bytes and consumed-field load bytes are checked
before any write. The production module base remains fixed and guarded; synthetic
guard fixtures allocate their own base rather than assuming an address.

The cdecl wrapper writes only the consumed DWORD on success, leaves caller
output untouched on failure, and preserves LastError as explicit native policy.
It rejects unsupported ASCII/path shapes before dispatch. Host pointers never
cross the channel. No original stat implementation or complete native manager equivalence is claimed.

## Installed transport and actual selected caller

[Installed evidence](native-audio-file-sizes-installed-pass-20261012.json) checks
all356 installed WAV whole-file sizes, four confirmed quoted aliases, zero-byte
and malformed-file metadata, and expected missing/suffix/traversal refusal.
365 native requests include363 successes and two expected host failures.
Only output+0x14 changes; surrounding bytes and LastError native policy match.
Both prewrite guard refusals, reinstall refusal and an executed patched cdecl
call pass. Source/input stability, immutable manifests and private Wine cleanup
pass. The setup-filename and missing VirtualFree fixture failures remain retained
at run-2f9csjlp and run-qb0qqu8q; they are not successful execution evidence.
All67 current audio CTests and13 launch-admission tests pass.

[Controlled live evidence](native-audio-file-sizes-controlled-public-pass-20261012.json)
uses a private copy of the previously validated music-enabled input, adding
permanent residency for existing sound812/picview1 in map0/1/2. Its file is
19,084 bytes; this input size is known, while the returned value and pool budget
are not directly logged in the live report. The selected original caller makes
one successful native file-size query. Eight native WAV loads/uploaded buffers
contain368,902 PCM bytes;12 plays and201 request/ACKs finish with no audio errors,
fallbacks or unsupported calls. Complete movies/CD/native presentation/campaign
and normal Quit, source/input/original stability and owned Wine cleanup pass.
The data change affects audio residency only. It is controlled caller coverage,
not unmodified-default configuration or whole-manager equivalence. Input
provenance is retained at working/experiments/native-stat-preload-input/run-3vjdclqe/report.json.

The unchanged installed permanent sections are empty, so a healthy default run
can establish hook composition without executing this stat branch. That
regression is recorded separately. Selected file-size routing defaults in native
voice sessions; independent legacy/audio opt-outs retain their existing policy.
Generic stat callers and original selection/cache/scheduler/event decisions remain
in the original executable.

[Fresh unchanged-audio-configuration regression](native-audio-file-sizes-default-public-pass-20261012.json)
passes normal Quit with the native hook installed, zero size queries as expected,
76/4/1 native profile reads, seven WAV loads/349,862 PCM bytes,12 plays/197 ACKs
and no audio errors. Full native movies/CD/presentation/campaign, source/input/
original stability and owned cleanup pass. Both fresh controlled and unchanged
configuration censuses record88 distinct direct Wine targets out of190 original
imports; cached/dynamic/COM/shim forwarding/transitive/prefix activity is excluded.
This is a direct-import observation boundary, not a total Wine API count.
