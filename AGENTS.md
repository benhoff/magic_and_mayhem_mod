# Magic & Mayhem Reverse Engineering

## Evidence acceptance and finite retention — user approved 2026-11-15

Follow [the repository-wide evidence lifecycle](docs/evidence-retention.md).
This replaces older hybrid/perpetual retention and conflicting local exceptions.
Evidence is required to accept the exact change; custody is finite:

- Require focused checks, exact source/contract accounting, normal staged hooks
  and the checked combined candidate. Prose needs proportionate review. Engine
  validation promotions require fresh appropriate scope-bound execution proof.
- Keep supporting evidence through integration plus 14 days. Observe integration
  to start the clock; pending integration never expires merely by age.
- After expiry, review consumers/pins and commit an exact retirement declaration
  while reports remain available. Preserve identity, result, reproduction inputs,
  source/integration state, reason and lost capability; then reviewed bytes may
  be deleted without a replacement copy or restore. References do not imply
  perpetual custody. Longer holds need owner, purpose, budget and review date.
- Preserve immutable original artifacts, unique source, source history and saves.
  Keep current operational accounting inputs readable while consumed; accepted
  review exports survive, not every historical database/schema/draft/cache.
- Retired proof grants no fresh validation or promotion credit. Scoped gates
  preserve historical results and explicit unavailability; full audits still
  require the original payloads. Unrelated expired evidence cannot block work.
- Routine finish verifies used evidence and retains compact acceptance records;
  it does not require full SQLite backups, duplicate source bundles or eternal
  tested binaries. Explicit archives/migrations keep their promised copy counts,
  survivor verification and real restores. Never fake independent copies.
- Keep one owned task checkout on constrained machines unless concurrency has a
  budget. Remove reviewed temporary output at closeout. Retire clean inactive
  integrated checkouts after preserving unique files and discharging consumers;
  reconstructible committed source needs no extra archived checkout/restore.
- Prepare candidates outside integration admission; acquire the lease only to
  recheck the exact base/candidate and clean checkouts, then promote and release.
  Never develop, validate, back up or wait for a decision under admission.

## Machine-specific instructions

Before starting work, read `AGENTS.local.md` beside this file if it exists and
apply it in addition to these shared instructions. For a linked Git worktree
without its own supplement, read `AGENTS.local.md` from the main checkout if
present; locate that checkout using `git worktree list --porcelain`.
The supplement contains local storage limits, paths and build preferences.
Keep it untracked and do not copy it to other machines. If absent, continue
with these shared instructions.

## Worktree ownership and integration readiness

- Always create or reuse a dedicated, exclusively owned Git worktree and task
  branch before editing repository files or running task-producing commands.
  This includes documentation and tooling tasks. Inspect the starting checkout
  read-only, then work in the owned checkout; do not develop in the main/master
  checkout or another task's worktree. Reuse the same owned worktree for the
  bounded task instead of creating a checkout per step. Apply the local storage
  policy and share immutable inputs without copying installations or evidence.
- Finish implementation, focused tests, source/coverage accounting, applicable
  feature-branch checks and required evidence retention before requesting master
  integration admission. Commit the owned batch through the normal finish/hook
  workflow and pin its ready commit. Review known history gaps, dependencies and
  likely conflicts outside the integration slot. All required inputs, review
  reasons and local evidence must be available; no unresolved task blocker or
  user decision may remain. Readiness permits explicitly recorded validation
  limitations; it does not permit missing required checks or evidence.
- Never acquire or queue for the integration lock to reserve a place while still
  developing, testing or preparing evidence. Admit only ready tips. Batch already
  ready, compatible tips when useful; never hold admission waiting for another
  task to become ready. A blocked task must not hold up independent ready work.
- Before admission, prepare the combined candidate against the observed master,
  perform required combined-tree reviews/checks and the normal staged
  hook, and pin its exact commit/tree/base. A ready feature pass alone does not
  validate a different combined tree. Resolve conflicts outside admission.
- Under admission, recheck master, the pinned candidate and clean checkouts;
  perform only checked promotion. A changed base or candidate refuses promotion
  and preserves the work for review outside the slot. Release promptly.
- Follow [the integration workflow](docs/integration-workflow.md). The lease
  enforces exclusive master updates, not semantic readiness; agents must enforce
  this checklist even when using a custom integration wrapper.

## Goal

Build a documented, reproducible reverse-engineering and modding toolkit for
Magic & Mayhem (1998), alongside an incremental native modernization of selected
engine systems.

Current work covers build-specific pathfinding and audio reconstruction,
isolated runtime observation and adapters, native Qt/OpenGL presentation,
input and media integration, and native asset loading and audio foundations.
Validate recovered behavior and native implementations in bounded steps before
replacing original work during live execution. Offline reconstruction,
synthetic tests, observation hooks, and live replacement are separate milestones;
record their evidence and remaining boundaries in
`research/runtime/coverage-ledger.md`.

The longer-term gameplay objectives remain:

1. Commander orders for summoned creatures.
2. Per-creature veterancy.
3. A more active mana tempo/economy.

These gameplay features remain outstanding. Engine modernization is current
scope, while behavior validation must remain separate from gameplay balance
changes.

## Rules

- Never modify files in `original/`.
- Treat `original/` as read-only input, including the scripts stored there.
- Run `./tools/original-manifest.sh verify` before and after experiments that
  consume original artifacts.
- `working/` may be modified and may contain generated or installed game files.
- Every binary modification must be performed by a script.
- Before patching a binary, verify the expected original bytes and/or executable
  hash.
- Record discovered file structures under `research/formats/`.
- Record runtime structures, functions, addresses, signatures, and hooks under
  `research/runtime/`.
- Prefer data/config modification over executable patching where possible.
- Prefer isolated runtime hooks over broad executable rewrites.
- Keep experiments small, reversible, and independently testable.
- Separate confirmed findings from hypotheses.
- For reverse-engineering findings, record the evidence and confidence level.
- Do not assume pointer addresses or offsets remain stable across launches unless
  verified.
- Avoid introducing gameplay balance changes while still validating engine
  behavior.
- Store new generated evidence and bookkeeping in SQLite. Follow
  [SQLite evidence storage](#sqlite-evidence-storage); do not introduce expanded
  JSON report files as the default recording or export workflow.

## Working storage budget

Apply any machine-specific storage budget and staging preferences from
`AGENTS.local.md`. Keep generated output bounded, protect evidence inside its review window,
unique inputs and active consumers; review pins and dependencies
before cleanup. Follow the SQLite evidence preservation rules below.

### Acquisition and closeout

Acquire the smallest evidence that answers the question. Reuse applicable retained
inputs before recollecting. Classify reproducible compiler/build/staging output as
temporary and remove it after its consumers finish. Unknown output remains held.
Keep source until integration finishes; hand off evidence still inside its review
window before retiring a checkout. Cleanup is part of completion, not an optional
low-space action. Record remaining holds with allocation, owner, concrete purpose
and release/review date. Follow [finite retention](docs/evidence-retention.md) for
expiry and [storage consolidation](docs/storage-consolidation.md) for migrations
that promise continued recovery. Do not apply migration restore requirements to
intentional evidence expiry or reproducible committed source-checkout removal.

## Reverse-engineering coverage upkeep

Use [binary-to-behavior accounting](research/runtime/coverage/README.md) alongside
the coverage ledger. Each contributor owns incremental updates for the behavior
they investigate or change; the audit checks those links and claims but does not
infer semantic mappings or update the register automatically.

- Before work on an engine contract, inspect its entries in
  `research/runtime/coverage/register.json` and the relevant research. When a
  subsystem is not registered yet, seed the behaviors touched by the work;
  do not defer all mapping until the subsystem is complete.
- In the same change as new findings, implementations or validation, add/update
  stable behavior IDs and original build/address links, scope/confidence,
  implementation/test paths, evidence IDs, dispatch entries and scenario links
  as applicable. Include newly discovered branches, failures and remaining gaps.
- Keep understanding, implementation, original comparison, integration and live
  replacement statuses independent. Record intentional native policies separately
  from recovered baseline behavior. A function link covers only its stated scope.
- Treat source drift as an impact review, not a reset of completed behavior.
  Follow [targeted revalidation](docs/evidence-applicability.md).
  Before launching another original-game run for an existing claim, inspect
  `python3 tools/evidence-applicability.py plan --evidence <id> --behavior <id>`.
  Use `--require-applicable` when consuming an unchanged accepted parent proof.
  A preserved scope can supply retained inputs to a new analysis; an old
  collector's whole-file freshness refusal alone does not justify recollection.
  Documentation/accounting edits without a contract change require accounting.
  Control changes require current controls; analyzer changes require retained-data
  analysis; oracle changes require retained-capture comparison. Recollect only
  affected runtime/input/scenario evidence, or evidence genuinely missing from
  retained captures. Do not rerun a full matrix merely to make every hash current.
- Preserve unchanged existing milestones with reviewed source-applicability
  boundaries where appropriate. These reports pin original source bytes, existing
  behavior/scenario contracts and accepted statuses; audits freshly compare the
  named Python callables, conservative local dependencies and module initialization.
  They preserve applicability, not exact byte freshness, new CLI/test-discovery
  coverage, widened input domains or status promotions. Native/runtime sources
  remain full-hash dependencies. Preserve historical staleness/results; expire supporting bytes through the lifecycle;
  do not replace old hashes or use an applicability report as fresh execution.
- Preserve historical evidence identities/results and their recorded source hashes; payload custody follows finite retention. Register a new
  result under a new evidence ID; never refresh old hashes just to clear staleness.
  Source-only changes may leave evidence stale: report the affected limitation,
  and rerun the relevant comparison before claiming current-code validation.
- Run `python3 tools/check-re-coverage.py --base HEAD` after ordinary changes and
  inspect affected behavior/evidence findings. Run `python3 tools/audit-re-coverage.py`
  for an intentional full audit or a discovery change needing corpus accounting. Run checks appropriate to the
  change; use `--require-fresh` when requiring all registered evidence to match
  current source. Existing unrelated historical staleness is not a reason to
  rerun every subsystem or stop authorized work.
- Run `python3 tools/check-re-coverage.py` for the reviewed local baseline, or
  pass `--base <reviewed-commit>` for a Git comparison. Full evidence CI is
  manually requested; ordinary CI runs isolated tooling guards. The change gate
  compares the requested Git base; changing a census/baseline cannot
  reset that comparison. Initial adoption retains the existing backlog;
  `--bootstrap` is rejected once the base contains the gate baseline.
- Install the repository-local pre-commit gate with
  `python3 tools/install-coverage-hook.py`. It checks the staged tree against
  `HEAD`, using initial-adoption bootstrap only when needed. Stage the matching
  census, register and exact reviews alongside changes; unstaged fixes do not
  satisfy the hook. Local hooks supplement the required CI check.
- Before committing affected work, include the source changes, updated census,
  central/focused registers, evidence or explicit pending limitations, ledger
  updates and exact review receipts in the same commit. Coverage tooling,
  `.githooks/` and `.github/workflows/re-coverage.yml` must also be committed
  when introduced or changed; a local installation alone is not reproducible.
  Check the actual staged tree with `python3 tools/check-re-coverage.py --staged
  --base HEAD` (add `--bootstrap` only when HEAD predates initial adoption).
  Resolve failures and restage before retrying; do not bypass accounting with
  `--no-verify`. Preserve unrelated work already staged by other contributors.
- If commits landed since the last accounting review, inspect that Git range
  before completing the next coverage update. Compare each committed file's
  parent/current SHA-256 with its exact receipt, check affected behavior and
  evidence links, and append retrospective receipts for missing intermediate
  versions. Retain a report naming the reviewed commit range and unresolved
  gaps. Keep committed-history findings separate from uncommitted changes;
  retrospective accounting does not assert a past gate pass or new validation.
- For affected code, tests, research or coverage metadata, append an exact
  hash-bound review to `research/runtime/coverage/change-reviews.json`, naming
  all affected behaviors and new pending gaps. Prefer the local SQLite workflow:
  `python3 tools/coverage-bookkeeping.py prepare --base HEAD`, then `record`
  with concrete `--reason` and `--validation-reason` values. Recording rechecks
  the draft and exports automatically; inspect `gate_errors`, since a scoped
  receipt can leave unrelated work pending. After reviewing source links,
  `prepare --index-review-reason <reason>` generates and pins the source census.
  Keep the database under ignored `working/`; text exports remain Git/CI authority.
  The manual `--draft-review` workflow also prepares exact hashes. Drafts with
  empty reasons do not pass. Preserve prior receipts.
  Validation is `pending`, `recorded` or `not_required`; engine/protocol changes
  cannot use `not_required`, and status promotions require fresh execution
  evidence of the appropriate kind. Pending validation retains historical
  status/freshness without asserting current equivalence.
- Review source-census findings for new, changed, removed or unlinked files.
  After reviewing affected code and updating explicit implementation/test links,
  generate a new snapshot with `tools/index-coverage-sources.py` and update the
  register's `code_index` path/hash. This records an indexing review, not a new
  behavior validation result; preserve existing evidence fingerprints.
  Reconcile focused subsystem registers into the central register when changed;
  update their `register_imports` hashes only after reviewing their entries.
- Update `research/runtime/coverage-ledger.md` when scope or milestones change.
  Refresh the generated coverage summary when register/status changes warrant
  it. Retain durable machine-readable results through the SQLite report writer;
  use physical JSON under `working/` only for the explicit interchange/retention
  exceptions below. Follow the coverage README's evidence preservation workflow.
- Retain unknown functions, unassigned ranges, unresolved indirect flows and
  incomplete dispatch tables. Add documented classifications or recovered-range
  exceptions where justified. Re-export/review the binary inventory when new
  discovery changes its boundaries; ordinary native edits need only the audit.

## Repository layout

See [project architecture](docs/architecture.md) for current boundaries,
the proposed target structure, and the incremental migration plan. Proposed
directories do not imply implemented or validated subsystems.

- `original/`: immutable source media and supplied binaries.
- `working/`: disposable generated installations and experiment outputs.
- `tools/`: reproducible preparation, inspection, launch, and patch scripts.
- `reconstruction/`: build-specific models of recovered engine behavior,
  currently pathfinding and audio; keep these separate from native policies
  and application code.
- `runtime/`: injected PE32 observation hooks and adapters, including neighbor
  shadow capture and rendering, input, and media bridges.
- `apps/qt-shell/`: native Qt application hosting, viewport presentation,
  input forwarding, and media playback.
- `renderer/`: native OpenGL surface storage, drawing operations, and capture
  replay, separate from runtime hooks and the Qt shell.
- `assets/`: native asset path resolution, read-only file access, and byte
  comparison tools.
- `audio/`: native PCM loading, sample storage, and voice state foundations,
  separate from recovered engine contracts in `reconstruction/audio/`.
- `research/formats/`: documented on-disk structures and configuration formats.
- `research/runtime/`: documented runtime structures, functions, and hooks.
- `research/`: installation evidence and the immutable-input manifest, alongside
  the format and runtime research directories.
- `patches/`: patch specifications and release metadata, not patched binaries.
- `tests/`: automated checks and repeatable manual test protocols.
- `docs/`: architecture and consequential design decisions, separate from
  reverse-engineering evidence and coverage status.

## Architecture boundaries

- Keep QWidget presentation separate from session lifecycle and orchestration.
- Widgets express semantic actions; keep game addresses, wire offsets,
  Wine/X11 details, and executable staging behind application/adaptor boundaries.
- Native services must not depend on application widgets or build-specific
  reconstruction. Recovered contracts may use native storage/backend interfaces.
- Keep future native simulation/rules independent of Qt presentation and
  legacy hook mechanics; isolate intentional gameplay changes from recovered
  baseline behavior.
- Treat shared channels as versioned wire contracts, not shared host pointers
  or C++ object layouts. Record integration and validation in the coverage ledger.

## Game launch

Once the working installation and launcher have been created, use:

```bash
./tools/run-game.sh
```

## SQLite evidence storage

SQLite is the default for generated evidence, experiment results, inventories,
source censuses, coverage snapshots and change-review bookkeeping. Choose the
storage path before running a producer; do not generate a durable JSON file and
leave its migration to a later cleanup.

- New tools and agent scripts must use `coverage_evidence.write_report` for
  immutable generated reports under `research/runtime/` or `research/formats/`.
  Pass serialized bytes directly to the writer. Do not use `write_text`,
  `json.dump`, shell redirection or file copies to create expanded JSON reports
  there, including dated `*-current.json`, `*-final.json` and review snapshots.
  If a producer requires a physical output file, use a bounded temporary output
  under ignored `working/`, then import the durable result through the writer.
- The writer stores payloads in ignored
  `working/coverage/evidence-pending.sqlite3`. A small `.json`-named SQLite
  reference is allowed where the existing evidence/register/census interface
  requires a path. It is a compatibility reference, not an editable JSON report.
  Do not add redundant report copies or exports alongside it.
- Record reviews with `tools/coverage-bookkeeping.py prepare` and `record`.
  SQLite recording automatically exports accepted receipts to the existing
  active journal. Do not create separate draft/review/closeout JSON files for
  routine bookkeeping; use the SQLite draft and receipt records. Required
  machine-readable retention plans and dual durable archive receipts remain
  exceptions to this rule.
- Keep editable central/focused registers, the active review journal, reviewed
  gate baseline, source locks, bootstrap manifests, UI policy, authored
  configuration, wire schemas and test fixtures in their existing JSON format.
  A new authored JSON file needs an actual configuration/schema/fixture consumer;
  generated observations and evidence do not become exceptions by being called
  configuration or fixtures.
- Physical JSON under `working/` is an explicit interchange/debug export or a
  required producer/tool input, not a second durable evidence store. Keep it
  bounded and identify its consumer. Persist citable results, including failures
  and negative evidence, in SQLite before closeout. Preserve required retention
  reports, raw evidence and active consumers; this rule does not authorize their
  deletion or replace the reviewed cold-archive maintenance workflow.
- Before closeout, inspect newly added or copied generated research reports and
  confirm they are SQLite references rather than expanded JSON. Verify the
  original payload through the SQLite reader. Read routine review/audit results
  from SQLite or stdout where supported instead of creating another JSON file.

Install the coverage hook. Use targeted locally verified payloads for routine
checks; use `python3 tools/evidence-store.py fetch --verify` when explicitly
acquiring archives for a full audit. Source clones need not fetch the corpus.
Use `coverage_evidence.read_json`, `read_text`, `read_bytes` or `Reader` for report
inputs; a reference's identity is its original verified payload hash. Generated
reports remain SQLite-backed during custody. Keep editable registers, schemas,
configuration, fixtures and source locks as JSON. The accepted retirement control
journal is also authored JSON; plans and observations remain SQLite records.
Local commits/merges require available evidence for the exact accepted scope,
passing coverage checks and the normal staged hook. Use `tools/finish-work.py`;
it records compact acceptance in SQLite without automatic full-database backups.
Optional independent stores use `mm.evidencePrimary`/`mm.evidenceReplica`; explicit
two-copy verification must never silently fall back. Archive migration promises
retain complete verification/restores, while intentional expiry follows the
finite lifecycle. Keep in-window evidence and current operational inputs outside
retiring worktrees. Publication/private transfer is never needed just to finish.

Private backups use the explicit `tools/backup-evidence.py` command after local
completion. Its SSH host and project directory are repository-local Git settings
(`mm.evidenceBackupHost` and `mm.evidenceBackupDirectory`); never attach a transfer
to commit hooks. Preserve its retained SQLite catalog and pending sets, use
`--retry` after interruptions and verify a downloaded snapshot before claiming
restoration. Private backups do not update public publication/CI availability.
Source pushes and hosted merges do not require evidence publication. The pre-push
guard checks evidence pointer format and rejects raw evidence and pre-migration
history without opening evidence databases or fetching archives. Ordinary CI runs
isolated tooling tests; the full evidence-backed audit is an explicit manual
workflow run. A source clone alone may lack unpublished payloads and cannot claim
a complete evidence audit until its verified SQLite inputs are supplied. Local
commit/merge accounting and exact scope-bound evidence remain required during acceptance.
GitHub Releases is an optional distribution backend; publication is a separate
explicit operation. `tools/sync-evidence.py` collects missing immutable
payloads across outgoing history, verifies publication and commits the pinned
availability manifest; `--push` explicitly also pushes the current branch.
Preserve failed publication receipts and immutable assets. Publishing or omitting
publication never changes engine validation claims.
Apply the worktree ownership and readiness checklist above. Keep later concurrent
edits out of that batch's source snapshot, evidence claims and commit.
Only after the exact combined candidate has passed its required reviews, checks,
normal staged hook and required in-window evidence, acquire promotion admission through
`tools/integrate-work.py`. It prepares and validates outside the lease, then
rechecks the actual master base and exact clean candidate before promotion.
Custom operators use `tools/with-integration-lock.py --execute --expected-master
<base>` around promotion only. Follow `docs/integration-workflow.md`. Current
hooks and the inherited active lease remain mandatory for master updates. Do not
re-run validation, acquire evidence or investigate failures inside admission.
Preserve refused candidates and repair outside the slot.
The review workspace remains local and exact text exports remain Git/CI authority.
Archive relocation does not assert engine validation.
