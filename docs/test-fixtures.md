# Shared test fixtures

Use `tools/retained_artifacts.py` to load retained observations through an
invocation-scoped `coverage_evidence.Reader`. `read_artifact(reader, acquired,
path)` accepts an exact artifact path or a unique path suffix, then verifies
the recorded byte count and SHA-256. An ambiguous match refuses even if one
candidate is an exact match. Pass `max_bytes` when a check needs a declared-size
preflight before reading; the returned bytes must still match the declaration.
The budget does not impose a streaming limit on the SQLite reader itself.

`scoped_artifacts(acquired)` selects only the recorded `temporary_root`,
`session`, and `owner` directories. A caller may supply another explicit
`root_names` tuple. The result includes the excluded paths and does not mutate
the original acquisition. Scoping uses the producer's recorded `--repo` or
pinned absolute `run-native.sh`, without resolving historical paths in the
reader's checkout. Unknown launchers, foreign/escaping roots, duplicate owned
paths, and missing/ambiguous repositories refuse. Select the case's scope before
a suffix lookup when an acquisition retains observations from overlapping runs.
The existing menu/scene helper entry points delegate to this module and retain
their diagnostics for historical callers.

Use `tests/fixture_support.py` for small synthetic fixtures:

- `command_stream(operations)` writes consecutive MNMCMD05 records. It does not
  append END, allowing terminal-prefix tests to exercise a missing normal end.
- `menu_publication(...)` supplies the menu header and engine publication lane,
  including the existing v2 and v22 layouts. Each test owns its screen payload.
- `owned_menu_session(root)` creates one fresh synthetic owner admission and
  original-session directory beneath a temporary test root.
- `u32(...)` packs unsigned little-endian fixture words.

Builders stay independent of production readers. Keep invalid versions,
sequences, ownership, and payload mutations visible in their owning tests. The
fixture bodies and negative assertions remain local; only common construction
is shared. Synthetic records supply control inputs, not execution evidence.

The 2026-11-08 extraction has 275 passing focused controls in
`research/runtime/shared-fixture-support-results-20261108/controls-01.json`.
Nine dependent player/storage controls also pass in `dependent-controls-01.json`
in the same directory, for 284 checks in total. Reports and exact logs resolve
through SQLite. Existing original/native
execution hashes, accepted milestones, and pending limitations remain retained.
Changed analyzer/support source bytes require targeted applicability review or
retained-data reanalysis before claiming current equivalence for those old
proofs. This refactor adds no player acceptance row or original-game execution.
