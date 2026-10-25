# Terrain light supplier integration into master

This merge combines current master `618cc26c` with supplier commit `babb7633`.
It preserves the later native copy/history work, both ledger sections, and all
prior evidence identities and exact review receipts. Coverage metadata and the append-only ledger
are reconciled while preserving both parents. The source census is regenerated for the combined
implementation/test links.

The [source-binding review](terrain-light-suppliers-merge-results-20261025/source-bindings02.json)
checks all twelve inputs of each existing supplier execution record against the
merged tree. They match exactly. The three scoped supplier contracts retain
independent implementation/comparison/integration/replacement statuses; this
merge does not expand their validated domain. Fresh native64 and ASan/UBSan
[controls](terrain-light-suppliers-merge-results-20261025/native-controls.json)
rebuild both supplier and creature-cycle fixtures and compare exact results with
the retained original output. No original process is executed again.

The [committed-history review](terrain-light-suppliers-merge-results-20261025/committed-history02.json)
checks exact parent/current payload hashes against receipts from the supplier
base through the three subsequent master commits and the incoming supplier commit.
It remains separate from uncommitted merge accounting and does not assert a past
gate pass or new engine validation. The
[coverage audit](terrain-light-suppliers-merge-results-20261025/coverage-audit02.json)
and normal staged merge gate check the final reconciled tree. Verified local
SQLite backups are retained; no evidence publication is required.

Concurrent launcher, documentation and bookkeeping work in the main checkout
is excluded from this commit. Before advancing master, exact pending bytes,
base versions and reviewed combined overlays are stored in a verified SQLite
preservation snapshot. After the fast-forward, tracked overlays are restored and
untracked bytes are checked unchanged. Any pending source-census edit remains
with its owner and does not claim validation for that dirty combined tree.

Original allocation failures, installed entity/static-type production, script
reveal/notification consequences, whole-world scheduling and live replacement
remain outside the bounded supplier comparison.
