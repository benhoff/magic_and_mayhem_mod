# Magic & Mayhem Reverse Engineering

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

- Retain at least 90 GiB of filesystem free space on the main checkout's drive
  (explicitly approved by the user on 2026-10-19 for tranche 2 completion and
  tranche 3 issue work).
  There is no fixed size cap on `working/`, including nested worktree output.
- Before a storage-producing experiment or large build, use the main checkout's
  `tools/working-retention.py run --reserve-gib <estimated-output> -- <command>`.
  If the preflight refuses, review storage or obtain an explicit budget change
  before proceeding. The guard monitors free space every two seconds and stops
  its launched process group below the 90 GiB reserve. Reservations are estimates;
  this is not a filesystem quota and cannot constrain other writers.
- Use `tools/working-retention.py inventory --output <new-report-path>` to prepare
  cleanup. Preserve registered worktrees, referenced evidence and `.retain` pins.
  A review candidate is not permission to delete; inspect dependencies, preserve
  unique sources and raw evidence, and verify any archive before cleanup.
- Keep installations and Wine prefixes isolated. Prefer bounded capture counts
  and completed-run retention over accumulating full disposable copies. Record
  storage reports under `working/retention/`; do not refresh historical evidence
  hashes or remove originals to meet the budget.
- Check free space at the start of a work session, at least daily during active
  work, and after each build/experiment batch. Review storage when main-drive
  free space is below 150 GiB, or a completed batch has produced at least 10 GiB.
  Aim to restore 180 GiB free through eligible cleanup; 90 GiB remains the hard
  reserve, not the normal operating target. Do not rescan the whole tree before
  every command: retain one inventory per maintenance pass and refresh affected
  activity, dependencies and file identities immediately before removal.
- Periodic maintenance is authorized by the user on 2026-10-20. Within this
  standing authorization, archive and remove only reviewed, ignored payloads in
  completed/inactive runs, including ignored output inside registered worktrees.
  Preserve the source checkouts, unique sources, reports, configuration, saves,
  current fixture/tool inputs, all `.retain` pins, active/recent runs and the
  newest two runs per group. Keep files modified within 24 hours and directories
  active within the past hour. Check host-visible processes and Wine prefixes;
  old timestamps and a manifest alone do not establish inactivity.
- Before maintenance removes a local payload, retain an exact file-level plan,
  verify its complete cold copy and both durable catalog receipts, and recheck
  the source identity, pins, current consumers and activity. Record reclaimed
  allocated bytes separately from filesystem free-space changes; verify a real
  restore and preserve failed attempts. Historical source drift is not permission
  to discard original captures or negative evidence. If no safe candidates remain,
  report the shortfall rather than weakening these exclusions.
- At batch closeout, identify durable evidence and temporary execution outputs.
  Prefer bounded captures and reusable, marked, idle, exclusively locked fixture
  prefixes over fresh full installations for every attempt. Avoid full source
  snapshots when an exact Git revision plus preserved unique changes supplies the
  same reproducibility; never remove existing source snapshots without review.
- The verified cold archive is `/var/local/magic-and-mayhem-archive` on the second
  NVMe; keep at least 100 GiB free there. Automatic archiving inside experiment
  launch remains disabled; the reviewed periodic maintenance above is separate.
  Use `working-retention.py restore <working-path>` before
  reading archived inputs. Include ignored worktree output only after inspecting
  its activity; preserve tracked sources, pins and active/recent runs. An archive
  is storage provenance, not fresh engine comparison or replacement evidence.

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
- Preserve historical evidence and its recorded source hashes. Register a new
  result under a new evidence ID; never refresh old hashes just to clear staleness.
  Source-only changes may leave evidence stale: report the affected limitation,
  and rerun the relevant comparison before claiming current-code validation.
- Run `python3 tools/audit-re-coverage.py` after affected changes and inspect
  warnings for the touched behavior/evidence IDs. Run checks appropriate to the
  change; use `--require-fresh` when requiring all registered evidence to match
  current source. Existing unrelated historical staleness is not a reason to
  rerun every subsystem or stop authorized work.
- Run `python3 tools/check-re-coverage.py` for the reviewed local baseline, or
  pass `--base <reviewed-commit>` for a Git comparison. CI compares the actual
  PR/push/merge-group base; changing a census/baseline in the same change cannot
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

Run `python3 tools/evidence-store.py fetch --verify` in a fresh checkout and install
the coverage hook. Audit, gate and UI readers resolve reports directly from
SQLite without hydrating the working tree. Use `coverage_evidence.read_json`,
`read_text`, `read_bytes` or an invocation-scoped `Reader` for generated research
inputs; hashing a reference must mean the verified original payload hash.
The source-census workflow already writes generated research reports straight
to SQLite.
Use `export` or targeted `hydrate <path>...` only for readers needing physical
JSON. Use `inventory-json` and a verified, cold-backed `compact-json` plan for
existing expanded reports. Preserve editable registers, schemas, configuration,
fixtures and source locks as JSON. Evidence pointers in Git preserve exact original report bytes in
SHA-256-pinned SQLite release archives; never refresh historical hashes to
satisfy current checks. Local commits and local merges, including merges into
local master, require locally available, hash-verified evidence, retained verified
SQLite backups and passing coverage checks. Remote publication is not a
prerequisite for either operation; never upload evidence merely to finish a
local commit or merge. Keep the local databases and their backup catalogs pinned
against working-storage cleanup; a Git commit contains references, not payloads.
Use `tools/finish-work.py` to record the exact review in SQLite, retain verified
SQLite backups, stage the owned batch and commit with the normal hook enabled.
Private backups use the explicit `tools/backup-evidence.py` command after local
completion. Its SSH host and project directory are repository-local Git settings
(`mm.evidenceBackupHost` and `mm.evidenceBackupDirectory`); never attach a transfer
to commit hooks. Preserve its retained SQLite catalog and pending sets, use
`--retry` after interruptions and verify a downloaded snapshot before claiming
restoration. Private backups do not update public publication/CI availability.
Before an explicitly requested remote push or merge in a hosted repository, all
referenced evidence must be available from the configured shared store. This
sharing requirement does not apply to local Git merges. GitHub Releases is the
current distribution backend, not a local completion requirement; publication
is a separate explicit operation. `tools/sync-evidence.py` collects missing immutable
payloads across outgoing history, verifies publication and commits the pinned
availability manifest; `--push` explicitly also pushes the current branch.
The normal pre-push guard and CI continue to reject unavailable or raw evidence
and old graph commits. Preserve failed publication receipts and immutable assets.
Use one owned worktree per bounded task batch. Keep later concurrent edits out
of that batch's source snapshot, evidence claims and commit.
The review workspace remains local and its exact text exports remain Git/CI
authority; archive relocation does not assert engine validation.
