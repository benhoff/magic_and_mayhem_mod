# Native alternate canvas ownership integration

This merges native ownership commit `47d07b77` with current rendering master
`568c7304`. Stable-ID reconciliation preserves both registers, every exact review
receipt, the native copy/history work and the cursor provider recovery. The
combined source census is reviewed independently. No implementation or test
source of the owner/compatibility adapter changed during integration.

The [first history review](alternate-canvas-merge-results-20261025/history01.json)
checks 242 exact first-parent file transitions across the copy/history,
assurance and ownership branches since `b48a88c3`. The
[follow-up review](alternate-canvas-merge-results-20261025/history02.json) checks
41 transitions after the cursor merge landed, including rechecking the owner.
The [final metadata review](alternate-canvas-merge-results-20261025/history03.json)
checks another 28 transitions after the rendering review pins landed.
Every transition has an exact receipt; the append-only journal is excluded from
its own file hash. Large historical raw inputs were streamed and SHA-256 verified
from their existing retained local SQLite stores, without redundant copies or
publication. Existing verified raw-store backup catalogs remain retained.

Both reviews also check the complete current compiler-source closure and the
behavior/scenario contracts of `ACO2.native-owner-20261025`. These still match
native02: 24 frame cases, 61 refusals, full visible and alternate planes,
maximum extents, and release plus 32/64-bit ASan/UBSan. This reuses unchanged
execution evidence; it does not claim a new original/game execution. Native01
remains historical and stale where its test source differs. The exact historical
source version is retained in SQLite rather than having its hash refreshed.

The native owner retains scoped implementation and headless integration.
Original alternate allocation, natural publication, source/base relationship,
complete writers and retirement remain unrecovered. Original comparison and
live replacement are unchanged; the native layout and clear remain explicit
native policy. No full-frame, performance or live-game acceptance is added.

A refreshed combined audit/summary and exact merge reviews accompany the merge.
Local SQLite evidence and bookkeeping backups are verified before committing
through the normal staged-tree hook. Existing main-checkout launcher,
documentation, coverage and untracked changes are preserved separately through
an exact SQLite snapshot and reviewed overlays. They are not absorbed into this
merge. Their pending contributor census remains pending; the committed merge
has its own reviewed combined census.

The first combined summary remains an immutable snapshot of the earlier cursor
merge base. The final combined summary accounts for the later review pins. A
verified task-local SQLite read store avoids contention on the shared mutable
evidence pool. The temporary pending merge on main serializes the final master
advance; all original tracked edits are separately backed up and stashed, while
untracked files remain untouched. No evidence publication is required.
