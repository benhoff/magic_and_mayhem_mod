# Native audio catalog preflight

Status: native read-only preflight, separate from original behavior and live
replacement. Implementation: `reconstruction/audio/catalog_preflight.*`;
synthetic checks: `tests/audio-catalog-preflight-test.cpp`;
installed command/report validation: `tools/test-audio-session.py`.

Preflight binds a Qt-backed `Sounds.ini` snapshot, decodes the recovered ID
and randomized-group tables, validates the supported schedule count, and reads
one WAV at a time through `NativeManagerBackend`. No source/primary buffers,
scheduler ring, output device, game process or hook channel are created.
Playable file sources require supported complete PCM and a nonzero payload
within the existing 16 MiB per-buffer limit. Reported duration follows the
recovered unsigned 32-bit byte-count multiplication, including its wrap boundary. WAV/profile parsing errors and
missing files retain diagnostics; the scan continues after individual source
failures. Fatal profile/catalog/configuration failures make the report invalid.

Randomized IDs are logical requests, not missing WAV filenames. An overlapping
Sounds ID is reported as a group instead of probing its pseudo-name. Groups
report every member and its effective source: the recovered one-step admission
selection preserves known source IDs and maps unknown members to fallback ID
410. Empty groups, unavailable choices and nested group sources are unavailable
for this application preview. Group-only IDs are included. A group is marked
ready only when **every** choice is playable. This conservative application
gate does not change the reconstructed loader or RNG and does not claim that
nested groups have been reconstructed as safe playback paths.

Literal filenames remain the default. The explicitly selected
`dequote-missing-leaf` policy uses AU21's adapter for both preflight and playback;
no independent quote-repair implementation or silent asset substitution is
introduced. Successful preflight describes the selected stable asset tree; it
cannot guarantee that files remain present/unchanged until later playback.
The report is not a capacity guarantee for arbitrary simultaneous voices or
permanent preload budgets.

The shell's `--audio-preflight` exits 0 when all reported requests are ready,
3 when the valid catalog has unavailable sources/groups, 2 for invalid
arguments/profile/catalog and 8 for report-write failure. It emits JSON to
stdout or `--audio-report FILE`. `--audio-catalog` names the **Sounds directory**.
Use the offscreen Qt platform for headless checks:

```sh
QT_QPA_PLATFORM=offscreen working/build/qt-shell/mnm-qt-shell \
  --audio-catalog working/game-nocd/Sounds --audio-preflight \
  --audio-path-policy dequote-missing-leaf \
  --audio-report working/audio-catalog.json
```

Installed asset experiments must run the original manifest guard before and
after reading installed data. The repeatable validator does this automatically:

```sh
./tools/test-audio-session.py --sounds-root working/game-nocd/Sounds
```

Validation evidence and installed counts are recorded after successful checks
in the accompanying Qt session document. Remaining boundaries include other
installations/encodings, live original caller ordering, active-map permanent
budget behavior and missing `Stream.wav`; no asset is invented to fill a gap.

Validated selected installation: 344 file-source entries and 69 logical groups
(168 member choices), with schedule limit 12. Literal policy reports 339 ready
sources and five gaps: IDs 812, 813, 1016, 1017, 1018. Compatibility policy reports
343 ready sources and only ID 1016 (`Stream.wav`) unavailable. All 69 groups are
ready in both policies. These counts exclude group pseudo-names, including one
which happens to have a same-named WAV; original ID table/upload counts are not
the same denominator as request availability.

Evidence: [offline/installed report](../../working/tests/audio-session/run-o2evf5__/report.json),
[literal scan](../../working/tests/audio-session/run-o2evf5__/installed-literal.json),
[compatible scan](../../working/tests/audio-session/run-o2evf5__/installed-dequote-missing-leaf.json).
The independent WAV oracle checks all ready leaf PCM sizes/durations; input
hashes and original manifest guards verify no changes. The catalog fixture also
passes address/undefined sanitizers with leak detection enabled; see
[catalog sanitizer log](../../working/tests/audio-session/catalog-sanitizer.log).
Confidence: high for this selected native policy/installation and synthetic
edge cases; original/historical and live behavior remain separate.
