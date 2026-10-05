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
  all affected behaviors and new pending gaps. Use `--draft-review` to prepare
  the hashes, then supply concrete review and validation reasons. Drafts with
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
  it, retaining new machine-readable reports under `working/`. Follow the
  coverage README's report and evidence preservation workflow.
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
