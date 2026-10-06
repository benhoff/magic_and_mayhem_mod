# Coverage atlas

Launch from the repository root:

```sh
python3 tools/serve-coverage-ui.py
```

The launcher prints `http://127.0.0.1:8786` and opens the default browser.
Use `--no-open` to print the URL only, or `--port <number>` for another port.
Ctrl+C stops the server. Python's standard library is sufficient; there is no
frontend build, package installation, hosted service or game execution.

The default **Major tracks** view groups movement, native scene controls,
rendering/animation, effects/lighting, audio, assets/persistence, menus/campaign,
and core gameplay/spells. Planned commander orders, veterancy and mana changes
appear separately. Each card shows demonstrated capabilities, partial work,
a named milestone percentage and a concrete next task. Open a track to inspect
its full milestone checklist and required behavior/evidence links, then browse
its behavior gaps. Track drilldowns include both recovered and native policies.

The **Detailed coverage** view separates binary mapping, scoped implementation,
original comparison and scoped live replacement. Select a subsystem to inspect its
remaining work. **Work remaining** filters missing code, partial implementation,
understanding, comparisons, stale evidence, tests, integration and replacement.
Open a behavior for its verbatim scope, research, code, tests and evidence.
**Binary explorer** searches function names, behavior IDs and hexadecimal entry
or interior addresses; function details link behaviors and discovered direct
entry-to-entry callers/callees. Unknown functions and recovered-range exceptions
remain visible. Filters are retained in the URL for bookmarks.

## Reading the percentages

- **Track milestones demonstrated**: achieved named goals / all named goals in
  that track. `tracks.json` curates the outcomes and explicit required contracts;
  all required statuses and applicable non-static execution evidence must exist.
  Partial code is shown as in progress, with no estimated fraction. Missing IDs
  or empty required groups remain visible as mapping gaps and never count ready.
  The spell dispatch register contributes one spell goal. Goals have equal weight;
  their scope and size differ, so these are roadmap percentages, not estimated
  effort, whole-game completion, or full native feature parity. A live observation
  goal retains original engine ownership. Replacement requires replacement proof.
  Newly registered contracts in an assigned subsystem appear in the track's
  behavior drilldown; adding a new product outcome requires curating a new goal.
  Supporting tooling outside these tracks remains visible in detailed coverage.
- **Current execution fingerprints** on a track: each requirement has at least
  one current execution record of every needed kind (comparison/integration/
  replacement). Older records remain historical; this does not claim all linked
  evidence is fresh or expand any experiment's scope. Historical demonstrations
  continue to count as recorded achievements, visibly separate from this count.
- **Functions with behavior links**: distinct discovered functions containing a
  registered code anchor / all discovered functions, globally across builds.
  Data anchors, classifications and recovered ranges outside discovery do not
  inflate this number. A link may describe only a branch within the function.
- **Associated function footprint**: the union of linked discovered function
  bodies / file-backed executable section bytes. Whole bodies contribute, so
  this is an association footprint, not the percentage of code recovered,
  implemented or validated. Overlaps are counted once. Padding, embedded data
  and undiscovered code remain in the executable denominator; virtual zero-fill
  and external DLL bodies are outside it.
- **Scoped implementation**: behaviors marked `scoped` / selected behaviors.
  `partial` and `none` are shown separately, without estimated effort weights.
  Scoped means only the documented input/behavior boundary.
- **Comparison and replacement**: applicable recovered behaviors only. Native
  policy and tooling behavior is separate; an empty denominator is `N/A`.
  Original comparisons remain historical if linked source fingerprints changed.
  Current comparison counts require all linked original comparison evidence to
  have current fingerprints; mixed current/historical records stay separate.
- **Any integration** includes headless tests, preview and observation; it is
  not equivalent to replacing original engine work during live execution.

In the detailed coverage views, each registered behavior counts equally. The spell dispatch
checklist has many entries; counts do not represent effort, engine importance,
runtime cost or whole-game completeness. Gap categories overlap. Generic next
milestones derive from the register statuses; precise branch omissions are in
the verbatim recorded scope and research, not inferred by the UI.

## Data and verification

The server reads the current central register and pinned discovery JSON, then
runs the existing offline audit. **Refresh audit** rebuilds the in-memory snapshot
after repository changes. No records or historical hashes are rewritten, and no
original artifact is read. A failed refresh retains the previous snapshot.
Audit errors are shown above the dashboard; source-census drift and historical
staleness remain findings, not silently refreshed validation.

Only explicitly registered document/source/test/evidence links are served as
plain text. The server binds to loopback, rejects foreign Host/Origin headers,
and blocks traversal, original/working inputs and symlinks outside the repository.

Run the semantic and HTTP checks with:

```sh
python3 -B tests/test-coverage-ui.py
node --check apps/coverage-ui/app.js
python3 tools/audit-re-coverage.py
python3 tools/check-re-coverage.py --base HEAD
```

Source files and the curated milestone JSON belong to `TOOL.coverage-ui`. Frontend HTML/CSS/JS is included in
the source census alongside Python so changes remain subject to accounting.

Optional browser checks are reproducible with an existing Chromium CDP endpoint
and Python `websockets` (only browser QA uses this extra dependency):

```sh
python3 tests/test-coverage-ui-browser.py \
  --url http://127.0.0.1:8786 --devtools-url http://127.0.0.1:9331 \
  --output working/tests/coverage-ui-browser-new
```

The script refuses to overwrite its output directory, exercises the real
controls, compares displayed percentages to the current register, and retains
desktop/mobile screenshots and a machine-readable result. No original game
execution or source-status promotion is implied by UI verification.
