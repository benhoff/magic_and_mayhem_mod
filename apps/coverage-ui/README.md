# Coverage atlas

Launch from the repository root:

```sh
python3 tools/serve-coverage-ui.py
```

The launcher prints `http://127.0.0.1:8786` and opens the default browser.
Use `--no-open` to print the URL only, or `--port <number>` for another port.
To serve on all IPv4 interfaces:

```sh
python3 tools/serve-coverage-ui.py --host 0.0.0.0 --port 8786 --no-open
```

Open `http://<server-ip>:8786` from another machine, using this server's LAN IP.
`0.0.0.0` is the bind address. You can also bind a single interface with
`--host <server-ip>`. The server remains read-only and has no authentication;
network visitors can view its registered research/source/evidence links.

Ctrl+C stops the server. Python's standard library is sufficient; there is no
frontend build, package installation, hosted service or game execution.

The default **Engine overview** view summarizes each active engine track:
implemented/partially implemented/not implemented behaviors, current and historical original comparison,
headless/preview/observation integration, scoped live replacement and the next
open milestone. Summary behavior counts deduplicate contracts shared by tracks;
track rows retain their own denominators. Planned gameplay changes remain separate.
Bookmark `#view=engine`.

The **Simulation roadmap** follows owned state/checkpoints, command execution,
ordinary owner skirmishes, original decision-pipeline comparisons, creature combat,
Cure/Sphere effects, campaign continuation and playable native campaign slices.
Wizard strategy, original tick/lifetime, full original gameplay composition and
live simulation replacement remain independent open gates. Open each milestone
for its exact required stages and behavior evidence; **Browse gaps** includes
both recovered contracts and native policies. `tracks.json` defines the checklist;
**Refresh audit** recomputes states from the current register without editing it.
Bookmark `#view=simulation`. A native preview does not imply original
engine equivalence or live replacement. UI checks validate presentation only.

The **Native rendering** page separates **Implementation** from **Validation**.
Implementation counts all registered rendering/animation behaviors, provides
expandable lists of partially implemented and not implemented contracts, and
names a separate implementation follow-up. Validation counts the curated
milestones by current, historical, pending or unknown execution support, with
its own next task and evidence-gap action. These denominators are different.
Every rendering milestone has independent implementation and validation panels.
The **Open contracts, reconciled** panel reviews all 203 previously partial/absent
entries by role: 111 diagnostic/research tools, 67 bounded implementations located,
13 unresolved original contracts, five aggregate contracts with narrower supplied
implementations, and seven acceptance/corpus targets. Open a group or behavior
for its concrete finding, source witness and remaining work. These static findings
do not add to the registered implemented count. Changed source/scope/status makes
a finding historical. The linked register/review preserves the full denominator,
original unknowns and independent execution statuses. `AN.world-facing` now links
existing native provider arithmetic and is partial; its synthetic controls do not
prove the entire original targeting contract.

Validation checks required comparison/integration/replacement stages and their
execution evidence independently of code or understanding completion; a code-only
milestone still needs implementation execution evidence. Current validation also
requires current source and scope bindings. Missing mappings remain unknown.
The combined milestone checklist retains all original requirements and statuses.
Selected native-only capabilities do not establish completion of the broader
rendering track. Research, release gates and existing `#view=rendering` bookmarks
remain available. **Refresh audit** updates counts; a running server must be
restarted after changes to the Python snapshot producer.
If a running server returns a plan without its computed review metadata, that
plan shows a restart notice while the rest of Native rendering stays available.
Restart the server and select **Refresh audit** to load the complete plan.

For API/producer denominators rather than equally counted dashboard milestones,
use `python3 tools/report-rendering-coverage.py`. The
[rendering crosswalk](../../research/runtime/rendering-coverage.md) separates
scoped code, independent original/driver comparisons and actual bypass across
the twelve required surface categories and nine binary drawing families. It
retains the newer queues1..16 World takeover executions separately. Those support
percentages are not full-category completion or effort estimates; audit errors
suppress current readiness while preserving historical scoped results.

The **Major tracks** view groups movement, native scene controls,
rendering/animation, effects/lighting, audio, assets/persistence, menus/campaign,
and core gameplay/spells. Planned commander orders, veterancy and mana changes
appear separately. Each card shows implemented and partially implemented capabilities,
a behavior implementation percentage and a concrete next task. Milestone
requirements and current validation are reported separately. Open a track to inspect
its full milestone checklist and required behavior/evidence links, then browse
its behavior gaps. Track drilldowns include both recovered and native policies.

The **Detailed coverage** view separates binary mapping, implementation,
original comparison and live replacement. Select a subsystem to inspect its
remaining work. **Work remaining** filters not implemented and partially implemented code,
understanding, comparisons, stale evidence, tests, integration and replacement.
Open a behavior for its verbatim scope, research, code, tests and evidence.
**Binary explorer** searches function names, behavior IDs and hexadecimal entry
or interior addresses; function details link behaviors and discovered direct
entry-to-entry callers/callees. Unknown functions and recovered-range exceptions
remain visible. Filters are retained in the URL for bookmarks.

## Reading the percentages

The Native rendering page also ranks **planning importance** from 1/5
(Supporting) to 5/5 (Critical). All 341 current rendering contracts have an
explicit weight and reason in [importance.json](importance.json). The default
queue includes all 199 partially implemented contracts, ordered by descending
importance with stable behavior-ID ties: 33 Critical, 50 High, 16 Medium, nine
Low and 91 Supporting. Filters select status, kind of work and importance.
Weights also appear in behavior details, the behavior register, work remaining
and reconciled rendering lists. Other tracks remain visibly unrated.

The **Critical work: 3 tranches** panel plans all 33 Critical, partially
implemented behaviors once, with 11 behaviors in each ordered tranche:

1. **Storage, ownership and callback safety**: tail reads, CPU continuation,
   output containment/failure reporting, binding/publication/pressure controls,
   read-only continuity, checkpoint and palette residuals, displacement edges
   and source-region epochs.
2. **Original canvas history and native producers**: create/Lock branches,
   padded binding, clear completion, pre-World reuse and initial state, producer
   attribution, palette/wave domains, owned replay, startup composition and
   producer caller effects.
3. **Integrated journeys and acceptance evidence**: finite history and handoff,
   targeted DirectDraw compatibility, stable-source launch/restart, zero-forward
   controls, original publication, action-bound storage sessions, hardware replay
   and physical-frame identity.

Each behavior has a concrete next action, implementation boundary, validation
check, completion condition and registered test/research links. Open a task's
details or the behavior dialog to read them; the importance list also shows its
tranche action. Tranche order is a proposed execution sequence, not a recovered
dependency graph. A reproduced contained graphics failure in tranche 1 supplies a minimal
compatibility fixture for the targeted tranche-3 repair. Unknown original producer/history inputs
must be resolved or explicitly refused before dependent comparisons run.

Tranche 1 has a reviewed execution brief for each behavior: required inputs,
actual entrypoint templates and their current coverage, missing fixture cases,
strict pass criteria and explicit handoffs. Its order is tail allocation, CPU
continuation, region permissions, bind admission, publication isolation, pressure,
read-only continuity, palette residuals, checkpoint residuals, displacement and
finally output ownership. The last task owns one shared 20-second stress pilot,
after the ten bounded controls pass; other tasks do not each require their own
public game run. The runner's existing flow deadline and bounded attempt policy
remain visible in the tranche execution rules.

Several planned assertions need fixture work before they can be run: complete
continuation state, additional bind schedules, publication thread/ownership
records, producer-level region epochs, a cross-thread palette lifetime collision
and checkpoint opaque-capacity/abandoned-owner cases. Existing helper-only,
same-thread or fake-clock results cannot stand in for those controls. Command
templates use real current CLI flags; legacy runners without prospective claim
support require a new declaration-bound wrapper before their output can support
current validation. Required retained inputs are preflighted rather than silently
substituted.

Tranche 2 also has eleven reviewed execution briefs. Its
[source and planning review](../../research/runtime/rendering-tranche2-planning-review-20261019.md)
places selected producer recovery before first-World history and separates
source-only producer replay from the existing native word-zero World comparison.
Startup/World and producer capture use separate incompatible CLI pipelines;
equal pointers in separate runs cannot establish one canvas lifetime.
The startup comparison's `success` field means measurement completed; task
acceptance additionally checks every required pixel-match field and source hash.
The [tranche-2 state review](../../research/runtime/rendering-tranche2-state-review-20261019.md)
now records eight implemented scopes and three unresolved aggregates (pre-World
reuse, producer history and initial state), with no identified concrete missing-code
branch. The retained initial-state mismatch is a separate acceptance failure;
historical matching 16-queue completed outputs do not reconstruct the initial canvas.
Creation, bind/rebind, clear, palette-selector, wave and generic-caller reference
fixtures are supplied. Padded dimensions, a native wave initializer and the finite
generic caller signature/tag guard with zero-EAX return domain are implemented.
Their fresh execution remains pending. A supplied same-run history checker still
needs valid matching inputs and does not prove all writers or physical ownership.

Tranche 3 uses the [implementation-state review](../../research/runtime/rendering-tranche3-state-review-20261019.md)
to distinguish **Implemented within scope**, **Partially implemented** (a named
missing branch or mechanism), and **Unresolved aggregate** (reconcile delivered
services before sizing code). **Acceptance failure** is a separate label and can
coexist with implemented code. Its eleven tasks now show current state, validation,
acceptance and next steps by kind of work. Ten scoped mechanisms are implemented;
DirectDraw retirement needs aggregate reconciliation. No concrete missing code is
identified in the ten scoped tasks. Storage parity and the hardware suite retain
the same baseline negative as acceptance failures, without implying two native
defects. The obsolete request to add the already delivered default-offscreen
descriptor policy has been replaced with pilot diagnosis and scope reconciliation.

The subsequent [tranche-3 issue execution](../../research/runtime/rendering-tranche3-issues-review-20261019.md)
records a fresh 1,061-checkpoint / 16-World finite handoff pass and advances only
its integration stage to **headless**. Factory, surface, DC/clipper and resource
comparisons pass after a stale mode-enumeration fixture is repaired; full
DirectDraw retirement remains an unresolved aggregate. The complete unchanged
238-frame storage reference still fails at ordinal 170 (252 combined pixels).
The separate 222-frame corrected-driver and 323-frame Quick Battle profiles pass.
Task cards retain these different outcomes, prior source assessments and the
diagnostic limits; an expected-negative harness pass never clears pixel parity.

These source assessments do not renew historical engine execution or change the
registered stage counts; task details show that older accounting separately.
Source fingerprint drift flags the assessment for review. Pending validation,
excluded domains and a deliberately false complete-replacement flag do not by
themselves mean partial implementation. All three tranches now use these definitions.

Tranche 1 now has a [state review](../../research/runtime/rendering-tranche1-state-review-20261019.md):
nine implemented scoped mechanisms, two unresolved aggregates (palette resources
and checkpoint attachment), and one overlapping output-ownership acceptance
failure. Ten bounded PASS results remain historical. Already delivered continuation,
producer-epoch, bind/publication, palette-race and opaque-borrow controls are listed
as supplied, rather than proposed as missing fixtures. Next steps separate
current-source validation, aggregate reconciliation and retained-pilot diagnosis.

The shared status guide defines all five work labels. Reviewed behavior rows,
work summaries, importance entries and dialogs use their source assessments;
unreviewed partial entries say **Registered partial** / **Scope review pending**.
The importance filter also offers **Partially implemented (reviewed missing code)**,
**Implemented within scope**, **Unresolved aggregate**, **Not implemented (reviewed)**
and **Acceptance failure**.
No missing-code conclusion is inferred from stale evidence or an excluded domain.

The **High work: 3 tranches** panel initially selected 51 unfinished High contracts
(50 then registered partial and one then not implemented), in three groups of 17:

1. Terrain generation, palette and lighting.
2. Animation, sprites and native drawing.
3. Ownership diagnostics and continuous journeys.

The [High source review](../../research/runtime/rendering-high-tranches-review-20261019.md)
initially recorded 48 implementations within their bounded scopes needing validation, two
unresolved aggregates (`NR.surfaces` and `NR.public-resident-world`) needing a
scope crosswalk, and one original model without a linked implementation at that review
(`RS.displacement-final-word-read`). These initial findings remain historical;
subsequent task reviews and execution results can supersede them. The 47 High
contracts already scoped at selection retained their ranking and were excluded
from that unfinished-work plan.

Each task separates its implementation assessment, historical or pending
validation, concrete next action, completion condition and wider boundary.
The **Not implemented (reviewed)** filter identifies the missing model separately
from aggregate uncertainty and implemented code awaiting validation. Source assessments
remain separate from the bounded execution results and registered stages.
Source, scope, status or priority drift flags the task for another review.

[High tranche 1 execution](../../research/runtime/rendering-high-terrain-execution-20261019.md)
now records bounded PASS results for all 17 terrain/palette/light contracts.
The current-source matrix includes exact original/native state and WORD-plane
comparisons, distinct existing native presentation policies and retained
refusals. Each task shows its PASS and concrete broader next work. Two builds
each pass 20 terrain/palette CTests, with six additional harness/display checks.
Only the four-view traversal gains its missing original-comparison dimension;
implementation, integration and replacement stages remain unchanged.

The newer bounded tranche-1 normal acceptance and tranche-2 task passes supersede
the earlier blanket output-ownership blocker for the finite handoff. Earlier
stage-13/camera failures remain historical risks; a pinned pass does not establish
arbitrary-scheduling, native-only/restart, hardware or complete replacement
acceptance. New journeys retain their own scope and budget requirements.

Task acceptance now requires its positive and expected-refusal matrix to pass
with stable sources and inputs. A report listing mismatches or an unresolved
diagnosis is not a pass. The old contained BltFast failure is a historical
hypothesis: bounded non-reproduction is reported honestly, and a new failure
stays failed even if it produces a useful fixture. A bound failure can be handed
to the targeted compatibility repair while broader session acceptance remains
held. Region query success is also distinct from permission-cache admission:
an inadmissible descriptor can be returned successfully while remaining uncached.

The dashboard presents **planned work and retained execution results**; it does
not start experiments. Review each Critical tranche's 11 outcomes or High
tranche's 17 outcomes and unresolved findings before moving on;
completing its bounded tasks does not automatically mark all contracts
implemented or validated. Scope, status or importance changes flag the task for
review, and newly Critical partial contracts are shown as unassigned. Execution
uses prospective claims, new evidence IDs, unchanged historical hashes and the
repository's existing storage, immutable-input and coverage-accounting rules.

These are initial engineering judgments for reliable native rendering and
eventual complete replacement. Session safety, ownership and unresolved original
producer boundaries carry high importance; visible capabilities, optimization
and narrow diagnostics receive different weights. Some focused ownership
investigations are High despite being diagnostic. Every entry retains its reason
and explicit work type: capability, original recovery, broad aggregate,
acceptance/assurance or diagnostic/research. A Critical acceptance target is not
automatically a missing implementation task. Broad aggregates overlap narrower
contracts; weights are not additive estimates of independent work.

Importance describes the documented contract as a whole, not the measured size,
severity, execution frequency or effort of its remaining gap. It is independent
of implementation and validation: partial code receives no completion fraction,
and completed Critical contracts stay important while leaving the unfinished
queue. No weighted completion percentage is calculated. Scope changes flag the
importance as needing review; new rendering contracts stay unrated until an
explicit judgment is added. Weights can be revised as priorities and documented
boundaries change; they are planning metadata, not reverse-engineering evidence.

- **Behaviors implemented**: behaviors with registered `implementation: scoped` /
  all registered behaviors in that track or selected scope. Registered stage
  badges retain **Implemented**, **Registered partial** or **Not implemented**.
  Reviewed source assessments use the four work labels separately.
  Implemented means code covers the documented scope; it does not imply complete
  original parity, current validation, integration or live replacement. Partial
  code receives no completion weight. Shared behaviors count once in overview
  totals; per-track denominators remain local. Spell dispatch entries count
  individually in behavior percentages, so these are checklist proportions, not
  effort or whole-game completion estimates.
- **Milestone requirements met/pending**: all explicitly required stages and
  applicable execution evidence for a named milestone. These are separate from
  implementation badges: code can be implemented while comparison, integration
  or replacement requirements are pending. Each milestone also shows the
  implementation status of its required behaviors; missing mappings show
  **Implementation unknown**. Milestone counts weight goals equally and group
  the spell dispatch checklist as one outcome. `tracks.json` defines this
  checklist; no milestones or evidence statuses are promoted by display labels.
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
- **Comparison and replacement**: applicable recovered behaviors only. Native
  policy and tooling behavior is separate; an empty denominator is `N/A`.
  Original comparisons remain historical if linked source fingerprints changed.
  Current comparison counts require an exact scope-bound execution result with
  complete implementation, test and declared dependency fingerprints. A new valid
  result can supersede stale history; fresh unbound records remain historical.
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
plain text. The server binds to loopback by default. Explicit IPv4 URL hosts are admitted
for a wildcard bind; a single-interface bind also admits its selected IP.
Unconfigured DNS hosts and nonmatching Origin headers are rejected. The server
blocks traversal, original/working inputs and symlinks outside the repository.

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
