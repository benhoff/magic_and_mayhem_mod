# Functional rendering integration into master

This merge combines master `7a7b1b3c` with functional lifecycle commit `994555da`.
It preserves the bounded terrain closures, machine-local instruction policy,
functional FPS policy, original callback readiness correction and all retained
passing and failed results. Original game behavior is unchanged by this merge.

The register and review journal are reconciled by stable IDs and paths. Both
parents' records and hashes remain intact. The source census is generated from
the combined tree. Historical execution sources are never refreshed to match
newer code; inherited milestones retain their original scope and provenance.
The merged source bindings report names any changed inputs explicitly. A pending
current validation state is separate from an inherited historical stage.

The functional branch passed strict native-only 10/30 second lifecycles and a
public NVIDIA lifecycle. The broader public Fireball damage case remains failed.
This integration runs Python functional, readiness, campaign and coverage UI
controls, plus the coverage audit and staged merge gate. It does not rerun the
original game, establish wider combat acceptance, explain the historical NT17
failure, quantify instrumentation overhead or close original recovery.

The [merge records](rendering-functional-merge-results-20261025/metadata-resolution01.json)
retain the exact metadata resolution. Additional records in that directory cover
merged controls, committed-history reviews, source bindings and audit results.
SQLite payloads and review bookkeeping have verified local backups. Publication
is separate from this local master integration.

Master's pending launcher, documentation and bookkeeping edits are outside this
commit. Before advancing master, preserve their exact bytes and base versions in
a verified SQLite snapshot, compute reviewed overlays and check for concurrent
changes. Restore those overlays after the fast-forward and verify every existing
untracked file. A pre-existing uncommitted source-census edit remains its owner's
pending review; the committed merge has its own reconciled census and gate pass.
