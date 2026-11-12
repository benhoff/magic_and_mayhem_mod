# Native camera integration on master, 2026-10-27

This merge combines camera branch `c9cf63ae` with master `432d8b36`.
The existing SceneSession camera/resource ownership, linked generation retirement,
queued once-per-successful-plan rotation and spectator source-clock connections
merge without production-source conflicts. Stable coverage IDs and both parents'
exact historical receipts are preserved; the combined source census is regenerated.

Fresh native64 installed-scene and Clang ASan/UBSan executions are registered as
`CAMERA-MASTER.native-20261027` and `CAMERA-MASTER.sanitized-20261027`.
Both cover the five declared scene behaviors and eight exact CPU/software OpenGL
frame comparisons. The native installed run additionally confirms Forest 40x40x19,
77 retained placements, 766 draws, identical checkpoint/CLI images and immutable
original-manifest and installed-input hash bookends. New reports retain their own
source fingerprints; historical result hashes are unchanged.

The first sanitizer attempt was stopped by the local disk-reserve guard, before
completion. An attempted build reuse lacked the explicit reuse option and did not
execute tests. Completed results and these partial outputs are retained together
in `camera-master-validation-raw-20261027.tar.gz`, a SQLite evidence reference.
Builds and gate staging use private RAM; durable evidence and verified SQLite
backups remain on disk. The verified reader snapshot copies immutable bytes only
and does not change gate enforcement. Because the configured cold store is below
its 100 GiB reserve, this batch uses small verified local SQLite backup files;
no cold-store writes or network publication are performed.

`camera-master-history-20261027.json` and its independently recorded second review
retain the originally observed missing receipts. The closeout report accounts for
the exact reviewed committed range and retrospective raw native-player payload
receipts. Retrospective accounting asserts neither a historical gate pass nor new
engine validation. Original full-session equivalence, physical GPU acceptance and
live replacement remain pending. Main-checkout staged work is outside this batch.
