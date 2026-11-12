# Scoped evidence applicability

This batch separates immutable execution provenance from current applicability.
Existing behavior/scenario identities and source hashes remain unchanged. The
audit can preserve an already accepted milestone through an exact, immutable
source applicability review. Raw source drift remains visible. The normal gate
still requires fresh execution evidence for new source validation or promotions.

`EA05.scoped-applicability-controls03` passes 287 synthetic tests, including the
real pre-commit hook, audit integration, immutable anchors, phase planning,
conservative Python dependency projection, atomic CLI preflight, SQLite reading,
append-only supersession and portable AST fingerprints. The earlier controls01/02
results are retained at its original source phase. Development failures remain
retained; none asserts engine equivalence.

The two concrete preservation reviews cover the existing
`CS05.cleanup-storage-closed01` and `SW05.sustained-world-closed01` claims. Their
plans preserve the accepted scope despite whole-file drift in one and three
sources respectively, with no source action required. Revised reviews02 retain
the original reviews01 and explicitly label unchanged analysis/comparison
dependencies. Historical alternatives do not create revalidation work when accepted current proof
already covers a milestone; unbound observations remain diagnostic history.
No original game is launched, historical source hash rewritten,
rendering requirement added, or existing engine status promoted. The native
acceptance pack remains 16 of 156 rows; the retained pre-Save admission failure
and reliability gap remain open.

Use the plan in [the workflow](../../docs/evidence-applicability.md) before
collecting another session. Controls request affected controls; assertions request
retained-data analysis; oracle changes request retained-capture comparison.
Uncovered dependencies and native/runtime source changes request the affected
scenario. Contract/scenario expansion requires new proof. Reviewer-selected roots
must cover the scientific claim and external dependencies; this is conservative
local dependency analysis, not a complete external call graph or proof of current
CLI routing/test discovery. Historical failed runs and broader unknown behavior
retain their existing boundaries.

Evidence: SQLite references in `evidence-applicability-results-20261105/` and
`coverage/applicability/`; register behavior `TOOL.scoped-evidence-applicability`.
