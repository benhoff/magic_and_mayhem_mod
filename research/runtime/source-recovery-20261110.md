# Historical source recovery — 2026-11-10

The source sweep compares against master `197e9a8fb800be857b0501a1879fdd867309149f`.
This batch preserves historical source independently of bulky execution trees.
It does not authorize directory removal, change retention rules, merge historical
engine implementations, or renew rendering acceptance.

## Preserved source

`source-recovery-results-20261110/custody-01.json` is a SQLite-backed manifest of
5,814 source locations and 5,639 distinct original-byte objects (21,445,201 bytes).
Each object was decoded and compared with its source from both configured stores.
The manifest retains absolute historical locations, SHA-256, length, Git blob
identity and, for the reviewed worktree variants, checkout HEAD and relative path.
The exact preservation operator and its immutable custody root are retained too.

The reviewed worktree set contains 58 distinct historical source/UI versions,
including the additional HTML variant. Historical Python/shell operators are
preserved separately from production tools. The bounded operator scan excludes
decompiler output, builds, original media, environment/vendor internals, nested
registered checkouts and the current whole-player validation checkout. It scans
at most three directory levels and 2,500 directories per root; the main working
root reached that cap. This is not a complete inventory or cleanup clearance.
The current broader-player source remains owned by its existing task.

## Dispositions

* Recover `native-latency-20261026/tools/native_frame_latency.py` and its test into
  maintained tooling. Other trace/runtime edits in that checkout remain historical.
* Preserve copy-cost and lifetime-scan variants. Current master already has
  source borrowing and pending lifetime collection; older complete files must not
  replace newer implementations. Their old tests are historical inputs.
* Preserve campaign v23/matrix variants and their exact build/test inputs as
  historical phases, not current campaign behavior.
* Preserve CPU diagnostics, tranche/closeout tests, coverage UI variants and the
  sufficiency helper as historical research. The sufficiency helper addresses an
  old 42-row acceptance frontier; adopting it as a current acceptance checker
  would misrepresent today's contracts.
* Preserve one-off ignored migration/retention operators as source evidence.
  Machine paths and historical assumptions make them unsuitable for automatic
  production-tool adoption or execution.

## Existing committed branches

The manifest records all 19 inspected tips outside master's ancestry and their
source-path comparisons. They remain committed and reachable through their
existing branches. An absent path is a review lead, not proof that its behavior
is missing or that its historical tests validate the current engine.

Future native-engine integration should inspect the fire/spell-family branches,
native-world-population and original-wizard-controller-ns88: they contain casting,
population and wizard-planning source paths absent from current master. Separate
rendering review leads include CPU copy/stretch instrumentation, screen objects,
symbol crosswalks and the original/native whole-scene comparator. Screen-selection
recovery's three compared source paths already match master; Crossroads has ten
matching paths and one differing source path. None is bulk-merged by this batch.

## Recovered analyzer contract

The analyzer accepts the current schema-4 host trace, joins owner exports to GUI
pickup and first Qt swap by exact uint64 serial and source generation, and reports
separate timing distributions. It excludes cold/hidden/paused intervals, handles
retained-tail gaps and refuses duplicate exports, mismatched generations,
malformed identities and invalid timestamps. Original uint32 clocks have their
own adjacent-generation cadence calculation; an unobserved clock breaks a pair.
Negative original-clock residuals remain visible rather than clamped.

Recovery tightens the old draft's generation join and numeric checks and removes
unsupported claims of schema 1/2/3/5 compatibility. Twelve synthetic tests validate
these bounded contracts. No old trace was reanalyzed; current live measurement,
physical scanout, GPU completion and performance conclusions remain unvalidated.
The old source bytes remain in the custody manifest unchanged.

Run `python3 -B tests/test-native-frame-latency.py`. Consumers can call
`native_frame_latency.summarize(report)` on an already loaded schema-4 report.
The module does not launch the game or write a report itself.

## Restore an exact historical source

Use `coverage_evidence.read_json` to load the custody manifest and select the
exact `source_records` entry by `path`. In a new task-owned output directory,
open a new output file with `xb`, then call
`coverage_evidence.Resolver(root).copy(record['sha256'], record['size'], output)`.
Close the resolver afterward. Copy verifies the original object hash; compare
the restored Git blob identity with the manifest when preparing a recovery commit.
Do not restore over another task's checkout. Existing source files and branches
remain present, and no old pin has been retired.
