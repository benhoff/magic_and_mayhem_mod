# Integrate native rendering batches 24–26

This local merge combines actual master `568c7304` with the completed batch chain
`5fa079ec` → `c0ac30b3` → `f3ca7de1`. The chain contains 30 scoped implementations:
ten surface services, ten native BMP domains, and ten owned native text services.
Master's rendering assurance, storage-cause, caller-bound copy and cursor-provider findings remain
intact. Shared register/journal conflicts are reconciled by stable IDs and paths;
no engine implementation conflict was resolved by choosing one side wholesale.

Fresh combined-tree execution is separate from the three historical batch proofs:

| Batch | Fresh controls per Release / ASan+UBSan run |
| --- | --- |
| 24 | 429 checks; five unchanged-original copy-wrapper argument paths, 966 cases; independent original bar supplier |
| 25 | 175 CPU/GPU BMP fixtures; 50,792 checks, 25,539 refusals, 44,800 bounded mutations |
| 26 | 413 checks, 122 refusals, 36 independent CPU/GPU text pixel cases |

The exact immutable reports are under
`rendering-batches-24-26-merge-results-20261025/`, stored through SQLite.
`MERGE2426.surface-native-20261025`, `MERGE2426.bmp-native-20261025` and
`MERGE2426.text-native-20261025` bind current implementation sources and scopes.
`MERGE2426.copy-original-20261025` binds only the five successful copy-argument
contracts; identity/ABI, actual driver pixels, original failures/retries, bar
allocation, full scene, physical hardware/performance and live replacement are
excluded. Original artifacts verify before and after the bounded original-input
experiment. Native color/ownership/cache policies stay separate from recovery.
External Qt/driver leak scanning is disabled; owned resource balances are tested.

Nine combined-tree Python suites pass 128 tests, including 38 coverage planning
controls and lifecycle, camera, campaign, copy, storage and acceptance-policy
controls. The first immutable history audit retains its missing README receipt.
A retrospective exact-file receipt repairs that provenance gap; its successor
checks all 511 transitions across eleven commits without asserting a historical
gate pass or granting new engine validation. Both actual parent histories and
historical source hashes remain available. The existing current-master evidence
snapshot is checked by complete file SHA-256, SQLite integrity and its verified
catalog; 139 base-only payloads are present there. Incoming/fresh payloads and
review records receive separate verified pinned backups at normal completion.

The source census is regenerated for the merged tree. Broader partial surface,
BMP, font/actor and full rendering milestones remain independent. No source push,
remote backup or evidence publication is part of this merge. Main's uncommitted
launcher/bookkeeping edits are preserved through a local SQLite file/overlay
plan, verified backups and exact identity checks around the final fast-forward.
They are not part of this merge's committed source or validation claim.

The candidate refresh from `3c450f6d` to concurrent master `618cc26c` preserves a
verified SQLite snapshot of the prior candidate metadata. No source in the
three native execution fingerprints changed. Final history, audit and planning
controls include the new cursor-provider commit; old results remain immutable.

The imported `CSR1.helpers`, `CSR1.providers`, `CSR1.helpers02`,
`CSR1.providers02`, `CSR1.copy-live01`, `CSR1.helpers06` and
`CSR1.providers06` evidence records (20261025) retain their exact historical
source hashes and statuses. Their broad BMP/build-source fingerprints are stale
after integration; validation on the combined source remains pending for those
cursor claims. The fresh scoped batch proofs above do not renew them.

A second verified candidate snapshot retains the refresh to `568c7304`, including
its incoming `ece850f9` static-review pins. All native implementation fingerprints
remain unchanged. The final audit has no errors or source-census drift; the
historical cursor limitations above remain pending. Shared evidence contention
is retained in failed control reports; a verified, read-only baseline snapshot
is used for the successful retry and normal local completion.
