# Raw asset interface byte comparison

## Scope and reproducer

This is validation of the new native asset interface, not a new original-game
format. The read-only comparator in `assets/compare_main.cpp` compares every
requested loose file through AssetStore against independent binary
`std::ifstream` reads. Reproduce with:

```bash
python3 tools/test-asset-files.py
```

Default source/reference: `working/game-nocd`. Using the same installed file
through two independent I/O paths validates raw-byte fidelity and path
resolution; it does not establish equality to clean media. The runner can
compare separate roots with `--root` and `--reference-root`. It records their
union of filenames and full inventories without silently excluding files.

## Confirmed evidence

The initial run compared 4,834 files totaling 327,419,328 bytes with zero
differences, errors, or detected mutations. All files passed sequential and
reverse-order seek comparisons. Original-manifest verification passed before
and after against 2,927 original files. Initial evidence:
`working/tests/asset-files/run-nsh8k7s4/report.json` and `comparison.json`.

The installed no-CD tree differs from the documented clean inventory in size;
this is not a claim that patched executables or runtime-written CFGs equal
their clean-media counterparts. No files were decoded or gameplay tested.

The final-version run reproduced those counts with 13,321 successful seek
reads, all source-consistency/completion/stability flags true, three passing
CTests, and both original-manifest checks passing. Evidence:
`working/tests/asset-files/run-_wul2s40/`, including `report.json`,
`comparison.json`, and `original-before.log` / `original-after.log`.
Qt version: 6.11.2. Comparison executable SHA-256:
`dd78ba504f92c001ca7efecb40a4be3b92ce8ba889d075a0a76d15ba8865c3da`.
The runner records full inventories with each file's size/SHA-256. Inputs
remained read-only.

Confidence: high for raw-byte fidelity of the files actually compared and
tested native path policy. Original Windows loader compatibility, live game
integration, and decoded payload equivalence remain unvalidated.

## Checks and report interpretation

- Requests alternate between mixed-case relative Windows paths and explicit
  drive-absolute installation aliases.
- Sequential reads compare all bytes and compute SHA-256; reverse-order seeks
  compare all bytes a second time. Empty files and EOF are checked explicitly.
- Independent before/after source/reference hashes detect persistent mutation.
  The runner also checks whole inventories across the complete experiment.
- Per-file JSON includes sizes, first differing byte offset (zero-based; null
  when equal), hashes, completion flags, seek count, source consistency, and
  structured errors. Missing files and case collisions fail explicitly.
- Fixture CTest `asset-byte-comparison` uses independent Python hashes and
  known difference offsets, covering binary/empty/mismatched files, unequal
  lengths, missing references, invalid inputs, and output protection.

Return 0 means all equal/stable; 1 means differences; 2 means errors or changed
inputs. Hashes are stability guards, not a transactional snapshot. A transient
mutation fully reverted between observations may escape detection. Mutation
guards are not stress-tested with concurrent writers. The runner rejects
symlink inventory entries and non-ASCII relative names; resolver support for
in-root links is covered by separate fixtures.

See [CLI and report details](../../assets/README.md) and
[interface contract](asset-file-interface.md). The next milestone connects the
WAV loader and retains its separate decoded-PCM comparisons.
