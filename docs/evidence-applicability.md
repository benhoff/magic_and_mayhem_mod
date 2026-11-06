# Evidence applicability and targeted revalidation

A code commit triggers an impact review. It does not erase a verified behavior or
require another full player matrix. A current accepted proof suppresses rerun
work for historical alternatives; unbound observations remain diagnostic history. Raw source fingerprints remain immutable and
raw drift stays in `stale_evidence` / `evidence_freshness`. Separately,
`current_validation` may name `reviewed_applicability` when the exact existing
contract is still supported by an unchanged reviewed callable boundary.

Before acquiring another original session, run:

```sh
python3 tools/evidence-applicability.py plan \
  --evidence CS05.cleanup-storage-closed01 \
  --behavior NP.cleanup-storage-closeout --require-applicable
```

The plan returns a hash-pinned `retained_input`, scope preservation, and actions:

| Source impact | Required next work |
| --- | --- |
| No relevant change, including unrelated reviewed callable/test additions | Reuse the accepted parent proof and retained inputs |
| Control code changed | Run the affected controls |
| Analysis/assertion code changed | Reanalyze retained evidence under a new result ID |
| Oracle/decoder changed | Recompare retained captures with the current oracle |
| Runtime/input change or an uncovered dependency | Validate the affected original scenario |
| Expanded behavior/scenario/milestone | Obtain scope-appropriate proof; preservation does not grant it |

An applicable accepted parent can feed a new retained-data analysis. Do not treat
an older acquisition/comparison command's whole-file freshness guard as an order
to recollect the game. New facts still need their own assertions and exact review;
missing data may justify a targeted acquisition. Input/build/provider boundaries
and unresolved failures remain attached to their original records.

`evidence-applicability.py create` requires an ancestor commit containing the
exact existing evidence definition, accepted status and unchanged bound contract.
It writes immutable SQLite-backed source/status anchors and a review under
`research/runtime/coverage/applicability/`, then appends its reference to the
central `evidence_applicability` list. Use a new ID/report and `--supersedes <id>`
for a revised complete boundary. The gate preserves every old reference and
rejects deletion, rewriting or ambiguous duplicate reviews.

Repeated `--callables <analysis|controls> <path> <root>...` declares reviewed
Python roots. The projection includes reachable local definitions/globals,
imports and potential module initialization effects. It rejects dynamic namespace
inspection and unsafe rebinding; unknown/custom initialization stays conservative.
New unselected passive functions and standard unittest classes do not enlarge an
old proof. Canonical `__main__` dispatch is outside callable scope; this mechanism
never validates current CLI routing or discovery of newly added tests.
Reviewers must establish that the roots cover the claimed behavior and that
external dependencies are complete; the tool does not infer scientific adequacy
or a complete external call graph.

`--full-source <phase> <path>` records a dependency's revalidation phase without
relaxing its hash. Native/runtime directories always require full execution
provenance. Uncovered paths default to affected-scenario validation. Scoped logic,
imports, globals, runtime changes, altered contracts and newly declared stages
cannot inherit a preservation waiver. Old applicable evidence also cannot satisfy
recorded validation of new source bytes or a status promotion: the normal gate
still requires fresh, appropriate execution evidence. Neither reviewed source
boundaries nor phase planning retire a failed run or imply whole-player acceptance.

