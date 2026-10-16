# Owner creature knowledge (NS76)

Original No-CD build
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
`4181d0`, modes 1 and 2. Static findings are grounded in read-only exports
`working/tests/owner-knowledge-ns76-export-v1` through `v3`, including exact
disassembly. The decompiler misrepresents the dynamic `59c000` stack probe;
stack-relative values and control flow are checked against disassembly.
Execution evidence below supports high confidence within the declared corpus;
static discovery alone does not establish equivalence.

## Recovered scope

Owner trees at brain `11c02 + 48*owner` hold anonymous observation keys, **not
creature slot keys**. Node `+12` is the key, `+16` last observation, `+20` the
mode-1 marker, and `+24` the copied creature. Brain `11d82 + 4*owner` is the
next key. `11da2/11da6` track sweep epochs; `11fed`, `11fee`, `11ff2`, and
`11ff6..12072` hold reset, owner, memory iterator and 125 matched-slot flags.

Mode 1 marks records whose last observation is at least the previous sweep
epoch. Mode 2 performs eight work iterations, with at most eight owner advances.
It matches one old record per iteration, then scans new sightings when the
owner tree reaches its end. Disabled owners are skipped. Passing owner 7 moves
current sweep to previous and sets current to brain tick. Starting an owner
clears matched flags. Already matched slots cannot satisfy another old record.

Old records match active visible pool actors with the same owner and type;
health and present alliance do not filter this match. Minimum signed squared
distance wins, and pool order resolves ties. Fine XY coordinates wrap at
`dimension*32`; Z does not wrap. The maximum distance is the truncated x87
product `(wrappedSigned((tick-seen)*globalCFG589*3))^2 * float[5c53f0]`.
The pinned float is `0.00019290123600512743`. A match refreshes observation time
and snapshot but preserves the mode-1 marker. Unmatched records expire only
when signed wrapped age exceeds 1024. Remaining active, living, visible,
nonallied actors create new sequential-key records. This can retain allied
old records and can associate a different same-type creature with a memory;
the NS75 slot-based native policy does neither.

## Boundaries

Visibility fields `creature+24+4*owner` remain explicit inputs. This milestone
does not recover geometric visibility generation, the strategic task graph,
notification payloads or the two-brain World driver. The original entry clears
two notification vectors per owner in either mode, and new observations can
adjust strategic task valuations. The admitted domain keeps those vectors and
per-owner task pointers empty; those consequences remain pending. Only the
snapshot projection consumed by regional danger is modeled. Native save format
and frame admission are separate policies. No live replacement is claimed.

The scoped call graph includes active pool iterator `52a670`, health-filtered
iterator `52a710` with predicate `412aa0`, fine squared distance `40a710`,
wrapped-axis helpers `40f7a0/40fb00`, snapshot construction/copy
`418b60/418d50`, tree insertion/erasure `42a5d0/42a6e0` and reset `4156b0`.
Those helpers retain their other callers and untested branches. Snapshot
construction clears `+708` and `+8f9`; these fields are outside danger's
logical snapshot projection. Original task/notification consequences and
snapshot fields unrelated to danger remain unmodeled.

The lifecycle corpus does not directly compare nonzero inventory/resource
snapshots, arbitrary status/form combinations, extreme/future tick arithmetic,
allocator exhaustion or counter overflow. Earlier direct danger-helper tests
do not establish those inputs through this new producer. The native record/key
admission guards are implementation policies; the refusal suite is bounded,
not an exhaustive check of every guard combination.

## Native use

`native-encounter run-knowledge INSTALL X Y WIDTH HEIGHT SEED TICKS SNAPSHOT`
selects recovered creature knowledge on the generated regional graph. The
existing `run-danger` entry retains the NS75 native observation policy. The
new adapter marks memories, advances danger/deduction from the previous
observation state, then advances creature knowledge before creature maintenance.
This preserves the recovered ordering for the declared empty task domain;
actual World task-budget admission and dual-brain scheduling remain pending.

CMD1/v10 stores the updater's complete logical continuation and validates its
danger-memory projection. Old versions decode absent knowledge; older writes
refuse state loss. Activation stages a private copy and requires an unadvanced
danger producer. Normal runtime operations publish only after success. Native
admission limits records to125 per owner and fixed retained actor identities,
refuses key exhaustion, and retains the existing1MiB checkpoint limit; these
are separate from the original allocator's capacity. Embedded inputs support
`resume` and `edit-terrain` without installed assets or an original process.

The helper's insertion/expiry counters are net per-tick changes in record
cardinality, not a complete event log: an erase and insertion in the same tick
can cancel. Hidden/dead/allied/anonymous counters count record-tick witnesses.
Journey arrivals count goal occupancy frames, not distinct arrivals.

## Execution evidence

`owner-knowledge-bound-ns76-20261015.json` records 57,369,512 lifecycle checks
over18,432 synthetic fixture ticks, with original32/native64/sanitizer agreement.
Eight independently advanced original32/native32 installed journeys pass18,432
ticks. Native64 and ASan/UBSan/LSan traces and final snapshots match exactly.
Hidden movement lasts1344 frames; remembered records survive1031 of those frames,
then expire before the new sighting. Both travel seeds reach their goal; combat
seeds produce36/19 hits and1/0 deaths. Diplomacy retains original old-record
semantics. The state suite passes4873 checks,288 round trips and8 refusals;
journeys add2304 in-process and80 fresh-process restores. Cold checkpoints use
seed17 for each scenario; both seeds receive complete native64/sanitizer replays.

The completed record composes two prospectively declared stages. The immutable
`owner-knowledge-engine-stage-ns76-20261015.json` preserves v1's successful engine
helpers, paired journeys, replays and checkpoints, followed by failure of an
independent legacy assertion expecting wire version9. Its exact superseded
driver and assertion source are retained in
`owner-knowledge-retained-sources-ns76-20261015.json`. Only that assertion
(`9` to `10`) and the recovery driver changed; engine and paired-test bytes did
not change. V2 checks identical claims/source closure/installed inputs and every
retained artifact before reuse, then runs both corrected five-test core suites,
both installed encounter regressions, legacy observation-policy state tests,
and CLI run-knowledge/edit/resume/refused-output checks. Upstream hashes are
rechecked at completion. V2 does **not** claim repeated original execution.
Five separate negative guard checks reject changed scope, engine source,
source closure, installed inputs and cached artifacts; their raw report is in
`working/tests/knowledge-ns76-accounting/reuse-negative-guards-v2.json`.

The existing danger regression passes5,203,818 checks. All272 declared sources,
264 compiler/configure inputs, five installed input hashes and2927 immutable
originals remain stable within their respective stages. Original text is
guarded. The native legacy policy passes399 checks,96 round trips and10 refusals
on both native64 and sanitizers. Only `AI.owner-creature-knowledge` and
`NP.owner-knowledge-continuation` receive new scope-bound current validation;
older evidence hashes/statuses remain historical when shared sources change.

Run `tools/test-owner-knowledge.py` under the main checkout's storage guard,
with `--install`, `--build-root`, a prospective `--claims` declaration and a
new `--output` directory. Its normal path executes the complete corpus.
`--reuse-engine-run` admits only the exact documented assertion failure and
rejects changed engine/paired-test sources or claims; restore archived raw
inputs before using it. This composition is source-bound evidence, not proof
of full World scheduling, visibility, strategic AI, campaign or live replacement.


## NS77 continuation

[Owner visibility production](owner-visibility.md) now supplies the visibility
inputs in a separate opt-in encounter mode. NS76's report, input policy and
historical source hashes remain unchanged. The NS77 contract owns a bounded
staggered creature-perception projection; observation-point fog, UI reveal,
strategic consequences and full World scheduling retain their separate gaps.
