# Original screen recovery: master integration

This merge combines recovery `b6a69d45` with the completed cursor/evidence and
commit-tooling integration `d306d926`. All engine implementations from both
parents are retained. The ledger, central register and exact review journal are
merged by stable identity, preserving historical evidence and receipts. The
combined source census is reviewed separately from execution acceptance.

The incoming evidence tooling changes a declared Python dependency. Fresh
[screen services](screen-services-results-20261025/master-services01.json) and
[class11 lifetimes](screen-services-results-20261025/master-lifetime01.json)
therefore rerun against the actual combined source bytes. Optimized i386,
i386 ASan/UBSan and native64 ASan/UBSan agree on192 selection,9984 cursor,
18432 control,110 tick-transition and4608 lifetime cases per configuration.
The original executable hash and both original-manifest bookends pass. The
original bodies remain unchanged; the established allocator/platform/other-class
boundaries remain explicit. All recorded required sources match this merge.

New SSR1.master and SSR2.master evidence IDs preserve prior results rather than
refreshing their hashes. The22-table47-store132-target static census remains
static. No native takeover, whole-session equivalence, real heap/failure
reachability, indirect alias closure or other screen-class lifetime acceptance
is added. Understanding, implementation, comparison, integration and replacement
statuses remain independent and unchanged.

The [committed-history review](screen-services-results-20261025/master-history01.json)
checks actual parent/current payload hashes and exact receipts for every touched
file in the incoming committed range. Pending launcher changes in the main
checkout are outside this engine merge and are preserved separately during
master advancement. Local verified SQLite backups retain the new payloads and
review records. Nothing is published.

The [combined audit](screen-services-results-20261025/master-audit01.json) and
[ordinary dashboard reader](screen-services-results-20261025/master-ui01.json)
pass1,421 registered behaviors without source-census drift. All six implemented
screen contracts have current scoped original-comparison support. The
[generated summary](coverage/summary-screen-master-integration-20261025.md)
retains historical staleness and incomplete dispatch independently.
