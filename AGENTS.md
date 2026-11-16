# Magic & Mayhem Reverse Engineering

## Start here

Read `AGENTS.local.md` beside this file when present. In a linked worktree
without one, locate the main checkout with `git worktree list --porcelain` and
read its supplement. Apply its machine capabilities, storage limits, paths and
build preferences; keep it untracked and do not copy it between machines. If
absent, continue with these shared instructions.

Follow these detailed procedures when applicable:

- [Evidence lifecycle](docs/evidence-retention.md): user approved 2026-11-15;
  supersedes older perpetual/hybrid retention and conflicting local exceptions.
- [Integration workflow](docs/integration-workflow.md): owned worktrees,
  exact-candidate acceptance and promotion admission.
- [Binary-to-behavior accounting](research/runtime/coverage/README.md) and
  [targeted revalidation](docs/evidence-applicability.md): coverage and proof scope.
- [Storage consolidation](docs/storage-consolidation.md): closeout and migrations
  that explicitly promise continued recovery.
- [Project architecture](docs/architecture.md): current boundaries and proposed
  structure; proposed components do not imply implementation or validation.

## Goal and architecture

Build a documented, reproducible reverse-engineering and modding toolkit for
Magic & Mayhem (1998), incrementally modernizing selected engine systems.
Current work includes build-specific pathfinding/audio reconstruction, isolated
runtime observation/adapters, Qt/OpenGL presentation, input/media integration,
and native asset/audio foundations. Validate bounded steps before live replacement.
Offline reconstruction, synthetic tests, observation and live replacement are
separate milestones; record evidence and remaining boundaries in
`research/runtime/coverage-ledger.md`.

Commander orders for summoned creatures, per-creature veterancy and a more active
mana economy remain outstanding gameplay objectives. Keep engine behavior
validation separate from balance changes.

- Keep QWidget presentation separate from session lifecycle/orchestration.
  Widgets express semantic actions; adapters/application code own game addresses,
  wire offsets, Wine/X11 details and executable staging.
- Native services must not depend on widgets or build-specific reconstruction;
  recovered contracts may use native storage/backend interfaces.
- Keep native simulation/rules independent of Qt presentation and legacy hooks.
  Isolate intentional native/gameplay policies from recovered baseline behavior.
- Shared channels are versioned wire contracts, never host pointers or C++ object
  layouts. Record integration and validation in the coverage ledger.

## Repository map and experiment rules

| Location | Responsibility |
| --- | --- |
| `original/` | Immutable source media and supplied binaries/scripts |
| `working/` | Generated installations, builds and experiment output |
| `tools/`, `patches/` | Reproducible preparation/inspection/launch/patch tools; patch specifications and release metadata, not patched binaries |
| `reconstruction/` | Build-specific recovered behavior, separate from native policy/application code |
| `runtime/` | Injected PE32 observation hooks and adapters |
| `apps/qt-shell/` | Qt hosting, viewport, input and media |
| `renderer/`, `assets/`, `audio/` | Native rendering/capture replay, read-only asset access/comparison, PCM/sample/voice foundations |
| `research/formats/`, `research/runtime/` | Discovered file structures; runtime structures/functions/addresses/signatures/hooks and coverage |
| `research/`, `tests/`, `docs/` | Installation evidence/immutable-input manifest; automated/manual checks; architecture/design decisions |

- Never modify `original/`, including its scripts. Run
  `./tools/original-manifest.sh verify` before and after experiments consuming it.
- Perform every binary modification by script, first verifying expected original
  bytes and/or executable hash. Prefer data/config changes and isolated hooks
  over executable rewrites.
- Keep experiments small, reversible and independently testable. Record evidence
  and confidence; distinguish confirmed findings from hypotheses. Do not assume
  addresses/offsets survive relaunch without verification.
- Once the working installation/launcher exists, launch with `./tools/run-game.sh`.

## Worktree, acceptance and integration

1. Inspect the starting checkout read-only, then create or reuse an exclusively
   owned task worktree and branch **before editing or producing task output**,
   including documentation/tooling work. Never develop in main/master or another
   task's checkout. Reuse one checkout for the bounded task; on constrained
   machines, additional concurrency needs an explicit budget. Share immutable
   inputs without copying installations/evidence.
2. Finish implementation, focused checks, exact source/contract accounting,
   feature-branch checks and required evidence retention outside admission.
   Prose needs proportionate review; engine validation promotions need fresh,
   appropriate scope-bound execution proof. Use `tools/finish-work.py` and the
   normal staged hook to commit the owned batch; pin its ready tip. Keep later
   concurrent edits outside the batch's snapshot, claims and commit.
3. Review committed-history gaps, dependencies and conflicts. Prepare the combined
   candidate against observed master in the owned checkout; resolve conflicts,
   perform combined-tree reviews/checks and the normal staged hook, then pin its
   exact commit/tree/base from the invocation-bound finish result. A feature pass
   cannot validate a different combined tree. Required inputs, reasons and local
   evidence must be available, with no unresolved blocker or user decision.
   Explicit validation limitations are permitted; missing required checks are not.
4. Only ready candidates may acquire or queue for admission. Use
   `tools/integrate-work.py`; custom operators use
   `tools/with-integration-lock.py --execute --expected-master <base>` around
   checked promotion only. Recheck master, pinned candidate and clean checkouts;
   promote and release promptly. Changed bases/candidates refuse promotion and
   remain preserved for repair outside admission. Current hooks and the inherited
   lease are mandatory for master updates, including custom wrappers.
5. Never develop, validate, acquire evidence, back up, debug or wait for decisions
   under admission. Batch already-ready compatible tips when useful; never
   reserve the lock or delay independent work for an unready task. The lease
   enforces exclusivity, not semantic readiness.
6. After release, observe integration to start evidence custody. The standard
   wrapper records clocks; custom promotion uses
   `tools/evidence-lifecycle.py integrated --commit <exact-commit>`. Complete the
   storage closeout below before reporting completion.

## Reverse-engineering coverage upkeep

Each contributor owns incremental coverage updates; audits check links/claims,
not inferred semantic mappings.

- Before changing an engine contract, inspect
  `research/runtime/coverage/register.json` and relevant research. Seed touched
  behaviors when a subsystem is unregistered. In the same change, update stable
  behavior IDs, original build/address links, scope/confidence, implementation/test
  paths, evidence IDs, dispatch/scenario links, branches, failures and gaps.
- Keep understanding, implementation, original comparison, integration and live
  replacement statuses independent. A function link covers only its stated scope.
  Retain unknown functions/ranges, unresolved indirect flows and incomplete
  dispatch tables; document justified classifications/range exceptions.
- Before another original-game run for an existing claim, inspect
  `python3 tools/evidence-applicability.py plan --evidence <id> --behavior <id>`;
  use `--require-applicable` when consuming unchanged accepted parent proof.
  Reuse retained inputs. Documentation/accounting changes need accounting;
  control changes need affected controls; analyzer changes need retained-data
  analysis; oracle changes need retained-capture comparison. Recollect only
  affected runtime/input/scenario evidence or genuinely missing captures, never
  a full matrix merely to refresh hashes or satisfy an old whole-file guard.
- Reviewed applicability boundaries preserve existing contracts/statuses using
  pinned source bytes and fresh comparison of named Python callables, conservative
  local dependencies and module initialization. They do not grant byte freshness,
  new CLI/test-discovery coverage, wider domains, fresh execution or promotions.
  Native/runtime dependencies remain full-hash. Keep historical identities,
  results, source hashes and staleness; new results need new evidence IDs.
  Report source-drift limitations and rerun relevant comparisons before claiming
  current-code validation. Payload custody follows the lifecycle below.

For each affected change, follow the coverage README's exact review workflow:

1. Review new/changed/removed/unlinked census findings and implementation/test
   links. Refresh the SQLite-backed census with `tools/index-coverage-sources.py`
   and the register's `code_index` path/hash; the bookkeeping option below can
   automate this. Reconcile changed focused registers into the central register,
   reviewing entries before updating `register_imports` hashes. Index review is
   not behavior validation; preserve evidence fingerprints.
2. If commits landed since the last accounting review, inspect that range and
   compare every committed file's parent/current SHA-256 with exact receipts.
   Check behavior/evidence links, append missing retrospective receipts and retain
   a SQLite report of the range/gaps. This asserts no past gate pass or execution;
   keep history findings separate from uncommitted work.
3. Use `python3 tools/coverage-bookkeeping.py prepare --base HEAD`, adding
   `--index-review-reason <reason>` after source-link review, then `record` with
   concrete `--reason` and `--validation-reason`. Name all affected behaviors and
   pending gaps. Recording rechecks hashes and exports to the append-only
   `research/runtime/coverage/change-reviews.json`; inspect `gate_errors` because
   partial receipts can leave work pending. Keep the database under ignored
   `working/`; exact text exports are Git/CI authority. The manual `--draft-review`
   workflow remains supported; empty reasons never pass.
4. Validation is `pending`, `recorded` or `not_required`. Engine/protocol changes
   cannot use `not_required`; promotions require fresh execution of the right
   scope/kind. Pending validation preserves historical results without claiming
   current equivalence. Update the ledger when scope/milestones change and refresh
   the generated summary when register/status changes warrant it.
5. Run `python3 tools/check-re-coverage.py --base HEAD` for ordinary changes and
   inspect affected findings. No-base checks use the reviewed local baseline;
   `--base <reviewed-commit>` selects a Git comparison that census/baseline edits
   cannot reset. Use `python3 tools/audit-re-coverage.py` for intentional full
   audits or discovery needing corpus accounting; re-export/review inventories
   when discovery changes boundaries. Ordinary native edits need no inventory
   re-export. Use `--require-fresh` only when requiring all evidence current;
   unrelated historical staleness must not block authorized work.
6. Install current hooks with `python3 tools/install-coverage-hook.py` from a
   durable checkout. Stage source, census, central/focused registers, evidence or
   pending limitations, ledger and exact receipts together as applicable, including
   changed coverage tooling, `.githooks/` and `.github/workflows/re-coverage.yml`.
   Run `python3 tools/check-re-coverage.py --staged --base HEAD` against the actual
   index. Fix and restage failures; never use `--no-verify`. Preserve others'
   staged work. Use `--bootstrap` only when the base predates gate adoption;
   initial adoption retains the backlog. Local hooks supplement required CI.

## SQLite evidence storage

Choose storage before running producers. Generated evidence, results, inventories,
censuses, coverage snapshots and review bookkeeping default to SQLite.

- New tools/scripts must pass serialized report bytes to
  `coverage_evidence.write_report` for immutable reports under `research/runtime/`
  or `research/formats/`. Never create expanded JSON there with `write_text`,
  `json.dump`, redirection or copies, including dated current/final/review reports.
  Required physical producer output goes temporarily under ignored `working/`,
  then into the writer, whose default is
  `working/coverage/evidence-pending.sqlite3`.
- A small `.json`-named SQLite reference is allowed for existing path interfaces;
  it is not editable report JSON. Do not add redundant copies/exports. Read reports
  with `coverage_evidence.read_json`, `read_text`, `read_bytes` or `Reader`; identity
  is the original verified payload hash. Reports stay SQLite-backed during custody.
- Use SQLite drafts/receipts and automatic journal exports for routine reviews,
  never separate draft/review/closeout JSON. Required machine-readable retention
  plans and dual durable archive receipts remain exceptions.
- Keep existing editable central/focused registers, active review journal, gate
  baseline, source locks, bootstrap manifests, UI policy, configuration, wire
  schemas and test fixtures as JSON. The authored retirement control journal is
  also JSON; its plans/observations remain SQLite. New authored JSON needs an actual
  configuration/schema/fixture consumer; relabeling observations is no exception.
- Physical JSON under `working/` is bounded interchange/debug output or required
  tool input with an identified consumer, never a second durable store. Persist
  citable failures/negative results in SQLite. Before closeout, verify new/copied
  research reports are references and read their original payloads through SQLite.
  Read routine results from SQLite/stdout instead of making extra reports.

Use targeted locally verified payloads for routine checks. Source clones need not
fetch the corpus; `python3 tools/evidence-store.py fetch --verify` explicitly
acquires archives for a full audit. Ordinary CI runs isolated tooling guards;
full evidence CI is manually requested and requires verified original payloads.
A source clone with missing unpublished proof cannot claim a full audit. Local
commits/merges still require exact-scope evidence, coverage checks and staged hooks.

## Retention and storage closeout

- Keep supporting evidence through **observed integration plus 14 days**; pending
  integration never expires by age. Preserve originals, unique source/history,
  saves/configuration and current operational accounting inputs while consumed.
  Accepted review exports survive; not every historical database/schema/draft/cache.
- Acquire the smallest useful evidence, reuse retained inputs and apply local
  storage budgets/reservations. Bound generated output. After consumers finish,
  remove reviewed reproducible build/staging output and stop owned processes.
  Unknown output and unresolved ownership remain held; review pins/dependencies
  before cleanup. Cleanup is required regardless of free space.
- After expiry, review consumers/pins and commit/integrate an exact retirement
  declaration while reports remain available. Preserve identity/hash, definition,
  result, reproduction command/inputs, source/integration state, owner, reason and
  lost capability. Then reviewed bytes may be deleted without replacement/restore.
  References/pins do not imply perpetual custody. Longer holds need owner, purpose,
  allocation/budget and review/release date.
- Retired proof supplies no fresh validation/promotion credit. Scoped gates keep
  historical results and explicit unavailability; full audits need original bytes.
  Unrelated expired evidence cannot block work. Routine finish verifies consumed
  evidence and keeps compact SQLite acceptance, without automatic full-database
  backups, duplicate source bundles or perpetual tested binaries.
- Keep source until integration finishes. Before retiring a clean, inactive,
  integrated owned checkout, preserve unique files/settings, export accepted
  receipts, hand off in-window evidence/current inputs outside it and discharge
  consumers. Follow the storage procedure's exact SQLite inventory/plan and
  rechecks; remove via Git without force. Reconstructible committed source needs
  no extra checkout archive/restore. Record reclaimed allocation and remaining
  holds; do not clean another task's output.
- Explicit archives/migrations promising recovery retain their promised copy
  counts, survivor verification and real restores. Never fake independent copies
  or silently weaken selected two-copy operations. Optional independent stores
  use `mm.evidencePrimary`/`mm.evidenceReplica`. These recovery contracts do not
  apply to intentional expiry or reconstructible source-checkout removal.

## Optional backup and publication

Publication/private transfer is separate from local completion and never required
merely to finish, push source or merge remotely. It changes no validation claims.

- Private backup uses `tools/backup-evidence.py` explicitly after local completion,
  with repository-local `mm.evidenceBackupHost`/`mm.evidenceBackupDirectory`.
  Never attach transfers to hooks. Preserve its SQLite catalog/pending sets, use
  `--retry` after interruptions and verify a downloaded snapshot before claiming
  restoration. Private backups do not establish public/CI availability.
- The pre-push guard checks pointer format and rejects raw evidence/pre-migration
  history without opening databases or fetching archives. Optional GitHub Releases
  publication uses `tools/sync-evidence.py`: collect missing outgoing-history
  payloads, verify publication and commit the pinned availability manifest;
  `--push` also explicitly pushes the branch. Preserve failed publication receipts
  and immutable assets. Archive relocation grants no engine-validation credit.
