# Bounded experiment environment reuse — 2026-10-26

The previous workflow made a full private Wine-prefix or game copy per attempt.
The new host policy bounds each template family to one immutable baseline and
two locked workspaces, with verified SQLite state deltas retained per run.
See [operation and recovery](../../docs/working-retention.md#reusable-experiment-environments).

Confirmed filesystem controls exercise repeated same-inode reuse, clean reset,
settings/save/link restoration, two-slot refusal, live process exclusion,
changed template/idle tree refusals, pin preservation, corrupted delta refusal,
normal process-exit return and crash recovery. An isolated Wine11.16 registry add/query control also passes: a second loan keeps
the same static file inode, excludes the first run's key, and restores that key
from its exact retained state. The template stays unchanged. The control runs
in a private user/mount namespace and executes no original game instructions.
All 86 filesystem and existing prefix-owner, borrowed-fixture,
launch/audio/music/preferences and source-isolation regression methods pass;
114 declared Python sources compile. Both controls reports bind exact sources.

Changed capture/test entry points use the central helper. Explicit completed
fixture producers and standalone preparation/stage-only commands retain full
stages for their later consumers. Programmatic menu/scene and OpenGL session
staging reuse their environments after in-process checks. Native campaign
tests reuse admitted prefix templates when explicitly supplied. Fresh-prefix
initialization stays unchanged because DirectPlay requires admission before the
prefix exists; a pre-bootstrapped unmarked prefix is correctly rejected.

Confidence is high for the exercised filesystem and isolated Wine controls and
provisional for complete Wine/game integration. No original artifacts or game instructions were used by
these controls. Actual-game comparisons, Wine registry normalization, save/load
session acceptance and exceptional CLI lifecycle compositions remain pending.
All earlier evidence fingerprints and engine milestones remain historical.

The main checkout has a separate unresolved merge. The implementation is owned
in its own worktree; any source application must preserve that merge's staged
changes and conflict resolutions. The source tree and required SQLite payloads
must be available before local completion; publication is not required.

The [regression report](environment-reuse-results-20261026/controls06.json),
[Wine report](environment-reuse-results-20261026/wine06.json) and
[raw Wine baseline/receipts/logs](environment-reuse-results-20261026/wine-raw06.tar.gz)
are SQLite references. The immutable baseline and changed-state receipts retain
the bytes used by the restoration control. The other fresh Wine trees are
temporary execution output.

The final snapshot also preserves root-directory metadata, ownership and extended
attributes, including Wine DOS attributes. Preliminary controls01/Wine01 retain
their original hashes and remain historical after that change; exact preliminary
helper/control sources are retained as SQLite-backed source objects.

The final Wine06 bundle names Wine02 as its verified baseline parent. Reconstruct
unchanged baseline file bytes from Wine02 and overlay Wine06; pool.sqlite3 retains
the complete final baseline metadata. This avoids another full duplicate Wine
input bundle. Earlier reports/snapshots remain immutable historical results.
The main drive uses its own 90 GiB copy guard; provisioned output filesystems use
the caller's capacity guard, avoiding a 90 GiB requirement on a smaller RAM mount.

Automatic pooling is limited to temporary tests/experiments/build locations.
The newly added native player's dedicated prefix remains permanent, including
first-launch cloning from a template. Wine06 also passes on an actual same-device
bind-mounted output group; device IDs alone cannot establish rename boundaries.
Prior results and exact source versions stay historical after each correction.
The committed main range since the task base is reviewed separately in
[the hash-bound history report](environment-reuse-results-20261026/main-history01.json);
its watched transitions all match exact receipts. This does not assert a past gate
pass or account the main checkout's still-uncommitted merge.

The storage integration now combines the alternate-ownership launcher's pooled
game staging with the newer Save-directory cleanup. The fresh controls08 report
binds this composed source; controls07 remains historical for the earlier merge
that retained Save cleanup but omitted the reuse call. This correction does not
claim new Wine/game execution or engine validation.
