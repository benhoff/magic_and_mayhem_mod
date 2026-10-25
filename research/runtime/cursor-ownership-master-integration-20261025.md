# Cursor lifetime integration into master

The checked cursor merge `9d5b8086` preserves cursor recovery `0922ff4c` and
screen-services master `618cc26c`. The final reconciliation uses actual current
master `35ae1c4fc4d8f8d11568d2876dab0b32b4650475` as its other merge parent.

Background helpers remain in `screen_cursor.hpp`; foreground/timer clients and
borrowed deferred lifetime remain separate in `cursor_clients.hpp` and
`deferred_surface_copy.hpp`. The incoming native alternate-canvas owner, terrain suppliers and static
review-input pins remain independent. No cursor implementation is duplicated or
changed. All three cursor contracts and their complete required sources still
match retained execution02 evidence, including optimized and ASan/UBSan32 results.

Stable-ID metadata reconciliation preserves both actual parents' behavior/evidence
records, append-only exact receipts and ledger findings. The combined census is a
source-link review, independent of execution. The [extended history review](cursor-ownership-merge-results-20261025/history04.json)
adds newly committed transitions to the retained276-transition and43-transition
reviews. These are exact payload-aware accounting reviews, not historical gate
passes or new original/native/live experiments.

The276-transition review found one artifact-only omission: the retained failed
alignment fixture source lacked a receipt because its .bin suffix is outside the
normal census. A retrospective pending receipt accounts for its verified payload
without asserting a historical gate or new execution. The initial gap report and
failed source remain immutable. The intermediate pending reconciliation against
`568c7304` was superseded while another merge was active in the main checkout;
its reports and exact local SQLite checkpoint remain preserved as pending inputs.

Native allocation/source-alias ownership, unknown writers, arbitrary concurrent
shutdown, owning native integration and live replacement remain pending. Recovered
queue records own rectangles and borrow wrapper addresses; resolving current
interfaces does not establish recovered allocation ownership.

Master's separate pending launcher/documentation/coverage edits remain outside
this batch. Before master advances, exact bytes and reviewed overlays are retained
in local SQLite with verified backups. The contributor's pending source-census
edit remains for its owner to reconcile before committing. Evidence and verified
SQLite backups are local; nothing is published.
