# Cursor lifetime integration into master

This merges cursor recovery `0922ff4c` into screen-services master `618cc26c`.
The background helpers remain in `screen_cursor.hpp`; foreground/timer and
borrowed deferred lifetime models remain separate in `cursor_clients.hpp` and
`deferred_surface_copy.hpp`. No implementation is duplicated or changed.

Stable-ID reconciliation preserves both actual parents' behavior/evidence
records, append-only exact reviews and ledger sections. A combined source census
records source-link review independently from execution. The retained cursor
comparison still has the same scoped contract and complete unchanged source
fingerprints. Its optimized and ASan/UBSan32 results remain valid for that scope.
The background provider evidence retains its own independent boundaries.

The [merge reports](cursor-ownership-merge-results-20261025/history02.json)
review exact committed parent/current payload hashes since the cursor accounting
base, metadata reconciliation and current cursor source bindings. This is merge
accounting, not a past gate pass or a new original/native/live experiment.
Native ownership, unknown aliases/writers, arbitrary concurrent shutdown and
live replacement remain pending.

Master's pending launcher/documentation/coverage edits are outside this merge.
Before master advances they are preserved with exact bytes and reviewed overlays
in a local SQLite database with verified backups. The contributor's pending
source-census edit remains pending for its owner to reconcile before committing.
Local verified SQLite evidence/backups are retained; nothing is published.

The history review found one artifact-only omission: the retained failed-alignment
fixture source had no exact receipt because its .bin suffix is outside the normal
census. An appended retrospective pending receipt accounts for its verified
payload without asserting a historical gate pass or new engine validation. The
initial gap report remains immutable.
