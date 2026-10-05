# Retrospective committed coverage accounting

Reviewed the committed range `ffaf1b018123be400293cee0931cf12fb4cc20fc..359cb95e78eb72cbef79965425f1777b74ebf8a2`
against the retained frozen gate baseline and current central register. The gate,
hook and central register are still uncommitted, so no immutable implementation
commit boundary exists. The selected boundary uses the baseline's local creation
time and matching effect-lighting/build hashes; the local timestamp is supporting
evidence rather than an immutable historical claim.

The [machine-readable review](committed-history-20261005.json) records both Git
commit IDs, every parent/committed file hash, exact receipt matches, current
behavior links and whether the working-tree file still matches its commit.

One commit and all 13 changed files were reviewed. Twelve files already had
exact receipts. The committed ledger was an intermediate version; the existing
receipt covers its later working-tree contents. Added an append-only receipt
for that committed ledger version. No existing receipt, baseline or original
result was rewritten. The current ledger contents remain unchanged.

Common effect placement and native refusal policies have implementation/test
links. TL05 comparison/integration evidence and the three effect scenarios are
registered. Type-specific setup, alternate/creator/type-21 creation and
movement/recount remain explicit gaps. Neither this review nor the selected
common-parent fixtures establish a complete effect lifecycle or live replacement.

The central mappings currently live in the working tree; the historical commit
contains the focused register, comparison results and focused review artifact.
This review does not assert that the historical commit passed a gate or CI.
Existing evidence assertions/fingerprints were checked; game experiments were
not rerun. Uncommitted map-navigation work is outside this historical review.

To reproduce the file delta, use `git diff-tree --no-commit-id --name-only -r
359cb95e78eb72cbef79965425f1777b74ebf8a2`. Obtain each prior/current blob with
`git show <parent-or-commit>:<path>`, calculate its SHA-256 and compare the pair
with the append-only change reviews. Current accounting can be checked with
`python3 tools/audit-re-coverage.py` and `python3 tools/check-re-coverage.py`.
