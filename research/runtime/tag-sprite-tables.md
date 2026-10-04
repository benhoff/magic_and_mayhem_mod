# TAG sprite-table correlation and runtime boundary

Reviewed 2026-10-04. Installed-byte evidence and native offline validation.
No original TAG reader, runtime object offsets, hook or live replacement are
established. Native code has no dependency on game addresses.

## Reproducer and provenance

```bash
python3 tools/export-tag-support.py
```

The read-only exporter checks the No-CD executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
It inventories all TAG files, validates the companion SPR version-4 frame-name
tables and per-name index sequences, rehashes every source and records script
and input hashes. Original-manifest checks run before/after, including failure.
No executable or installed asset is modified.

The reviewed report is
`working/decompiled/tag-support-15_dz7p9/report.json`, SHA-256
`75ade0c430bb2586ffa5661a8297f227b5bf4babb602315b80b529298a47e60b`.
It records 85 files, 1,570,692 bytes, 130,891 entries and 17 companion SPRs.
All names match the corresponding frame ordinal, and every index equals the
number of preceding frames with the same exact eight-byte name. Confidence
is high for this corpus correlation. It does not establish future variants
or original reader failure/ownership/lookup behavior.

## Connection to the recovered SPR format

The already documented [version-4 SPR structure](../formats/spr-native-loading.md)
has a 24-byte header, embedded 768-byte palettes, a frame-offset DWORD table,
and frame records with eight raw name bytes at frame offset 20. The exporter
reads these bounded fields independently; it does not decode pixel payloads.

For 17 TAG inputs, the companion has the same stem and `.spr` extension.
The other 68 inputs use directory-local `Terrain.spr`. The TAG contains one
12-byte name/index record per corresponding SPR frame, including repeated
names. This confirms that the leading `UA000S1`/`UC000S1` bytes are data rather
than a container header. TAG's word at offset 8 belongs to its first entry.

## Original use remains unresolved

The exporter performs only a limited ASCII literal scan of the pinned No-CD
executable for `.tag`, `.TAG`, `UA000` and `UC000`; all counts are zero.
This is negative search evidence, not proof that TAG files are unused:
constructed paths, externally supplied filenames or separate tools remain
possible. No function address is assigned from that scan.

An editor/export-side metadata role is a hypothesis suggested by the exact
SPR-name correspondence. It is not a confirmed engine contract. Do not infer
that the original game must load TAG separately, or that native sprite name
resolution must depend on TAG rather than SPR's own names.

The native reader preserves records in order and imposes only extent/resource
policies. Its bounded display-name helper does not supply a case-folding,
first/last-match, missing-name or duplicate lookup policy. Recover those
contracts if a consumer requires them, then validate native consumers and
bounded live integration independently.

See [on-disk format/native validation](../formats/tag-native-loading.md) and
[coverage ledger](coverage-ledger.md). No TAG-specific behavior is promoted to
live replacement by the installed comparisons.
