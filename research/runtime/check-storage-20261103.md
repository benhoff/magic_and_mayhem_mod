# Bounded campaign check storage — 2026-11-03

The four storage changes are implemented as intentional host policy. They do not
change original simulation, map scripts, balance, native world parsing or the
requirements for nominal campaign acceptance. Current instructions are in
[campaign check storage](../../docs/check-storage.md).

## Measured execution

The [source-bound closeout](check-storage-results-20261103/validation-closeout-01.json)
joins actual current executions and preserves all parent hashes:

| Check | Result |
| --- | --- |
| Default lightweight original checkpoint restore | Successful selected-field restore and normal Quit; **0 rendering-capture bytes** |
| Explicit full restore of the same pinned checkpoint | Complete existing captures; **48,895,866 bytes** |
| Independent full original/history comparison | **50 Worlds, 112 leased paints, 97,944,000 pixels, zero differences** |
| Lightweight rendering admission | Refused before the original pixel oracle runs |
| Lightweight original-save byte copy | Exact existing checkpoint hash copied successfully |
| Shared inputs | Both runs use the same verified canonical game/assets/prefix, without local input aliases or copies |
| Prefix lifecycle | Both exclusive loans return normally; forced Wine shutdown is unused |
| Current controls | **41 storage/prefix/import Python tests, 144 campaign Python tests and 7 Qt driver cases** pass |

The save remains
`9ec512d4c05a75c20e7cc7befec8398e112c2ca700917f34e93f5501099ee7c1`,
with the original selected-field checkpoint scope. The two runs are short
software640 diagnostic restores, not matched timing experiments or fresh
victory/progression evidence. Their capture-byte difference is a direct
observation of the selected storage modes, not a projected universal savings
percentage.

## Archived historical outputs

[Nine older completed lighting outputs](check-storage-results-20261103/historical-retirement-01.json)
reclaimed **9.961 GiB** of allocated storage. Two historical failures remain
failures. The review excludes tracked files and current source-literal inputs,
checks activity and old modification times, retains the latest generic trees,
and records exact file/pin manifests in both stores before real restore checks
and physical retirement.

[Four existing completed comparison outputs](check-storage-results-20261103/comparison-retirement-01.json)
reclaimed **2.461 GiB**. Every physical file matched the original immutable
comparison's artifact membership and SHA-256 before the dual-store handoff.
Historical comparison results and their original source hashes were preserved.

Together these thirteen named historical trees reclaimed **13,338,087,424
allocated bytes (12.422 GiB)**. This is separate from net filesystem free-space
change, provisioning costs, new evidence and this batch's temporary outputs.
Whole source checkouts, current inputs, unique sources, canonical prefixes and
the older archive catalogs remain held. This is not a global worktree cleanup.

The [new queue verification](check-storage-results-20261103/queued-handoff-01.json)
also retired twelve completed trees from this validation batch, including the
full comparison's automatically queued output, after exact restores. It reclaimed
another **5.141 GiB** of temporary allocated output. Both shared stores grew by
about1.3MiB during that handoff; the report retains actual per-filesystem sizes
and free space. Historical cleanup totals above exclude these newly generated
temporary trees.

## Policy and boundaries

`--evidence-mode diagnostic` is the default, with a 120-second driver deadline
and cleanup grace. It retains semantic/original observer records, resource and
source checks, but requests no viewport/history/World-source capture. The report
kind is distinct and both rendering comparison tools reject it explicitly.
`--evidence-mode full` retains the existing lossless capture and independent
comparison requirements, with its existing 900-second default. Preparation emits
explicit full-mode acceptance commands. Existing full historical reports remain
readable without refreshed hashes.

`check_inputs` creates one fingerprinted project input set and small per-checkout
SQLite catalogs. Catalog-only mode preserves existing private installations and
saves. Campaign tooling passes canonical game/music sources explicitly and
borrows the canonical stopped prefix through the existing bounded exclusive
pool. Source drift and conflicting bindings refuse reuse. Source aliases are
resolved before private runtime copying; an existing runtime alias refuses
execution. Real shell controls prove a write changes only the private copy.

Comparison processes now own separate process groups through normal completion
and failures/timeouts. Their queue admission checks stopped comparison proof,
exact retained artifact membership/hashes and retained source bytes. Wine queue
admission continues to require stopped Wine and returned-prefix proof. Batch
finish preserves every member and pin, verifies both stores and a real restore,
and leaves a pinned historical-path restore stub. Future physical consumers must
restore that path before reuse. Launch does not archive output.

The first lightweight probe failed at the music source's old checkout-only
path guard. Its actual failed result and exact source phase are retained in
[negative01](rendering-progression-results-20261101/storage-light-restore-01.json).
The correction accepts only the verified shared catalog role. Subsequent probes
and source variants remain separate records. A guard command started before
preparation finished also refused its missing input; the completed preparation
then passed the guard suite. Neither failure is relabelled as a pass.

Forced private Wine shutdown command selection has headless controls; the live
normal completions here do not establish forced-timeout equivalence. Populated
lifetimes, native save parsing, unit/order targeting, fresh nominal campaign
victory/Realm/region2 transitions and the full twelve-case matrix remain separate
milestones. Historical engine evidence affected by edited source paths retains
its original hashes and any current-code staleness.


The [final coverage audit](check-storage-results-20261103/coverage-audit-03.json)
passes with zero errors. Its only newly stale storage evidence is the preserved
initial music-admission failure, whose old source phase is intentional. Early
accounting audits01/02 retain their metadata failures separately. The current
headless policy binding pins the complete508-source control/compiler closure;
its minimal required-source draft remains retained and supports no current stage.
