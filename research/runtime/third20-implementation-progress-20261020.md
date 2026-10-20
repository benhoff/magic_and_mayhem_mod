# Third twenty partial implementation contracts — 2026-10-20

The [frozen selection](coverage/third20-selection-20261020.json) chooses the next
twenty raw partial contracts by descending importance and behavior ID, excluding
the first forty already reviewed. All twenty are High priority. Contract review,
bounded implementation, original comparison, integration and live replacement
remain separate; a partial entry does not by itself prove a missing branch.

## Group 1: capture failure and COPY source identity

Reproduced a capture exit-path bug: late decoder errors could set report success
false but still print success and exit zero. The final exit now refuses that
report, preserving any earlier primary exception and the saved failed report.
A regression executes the actual production report/exit tail without a game.

Reproduced COPY normalization ignoring conflicting explicit read-worker IDs
and mutating earlier source prefixes before a later rejection. Supplied read IDs
now match their API. Snapshot prefix edits are staged until every run/read
validates, preserving original inputs on failure. Two negative regressions fail
before the changes. Retained valid multi-worker calls preserve the established
first-observed-command policy when no explicit read ID is supplied; it is not
a proof of global multi-worker causality. A stricter multi-worker experiment
changed the historical corrected-storage plan and was not adopted.

The first group also reviews existing backing classification, storage journal
versions/caller/epoch binding and unique same-thread Lock association. Their
numerical identities and observed ordering do not establish physical allocation,
all aliases/writers, original timing or native/live replacement. Fresh bounded
execution results will be linked after the controls complete. The remaining fifteen contracts are reviewed below; no broad milestone is promoted here.
execution results will be linked after the controls complete. Remaining fifteen
contracts are queued for this batch; no broad milestone is promoted here.

The [fresh result](third20-offline-controls-20261020.json) passes59controls and
replays the corrected222frame storage metadata through1,228bound COPYs. Its
normalized journal hash remains `fd7b1b4c4dce28f4a12c2289ad13972d7b7bc62b18bc63ac3847313f090dba2f`,
exactly matching the earlier immutable result. This is a current offline policy
check, not a new original/GPU/physical or complete journey validation. Original
manifests verify before/after; historical evidence and discarded diagnosis are
retained under their original fingerprints.

The first controls preceded the concurrent native-scene merge87105990. A
[fresh rerun](third20-offline-merged-controls-20261020.json) repeats all59tests
and the identical222frame/1,228COPY normalization after adopting the merged
code and accounting. The prior execution and its fingerprints remain retained.


## Group 2: scene generation safety and owned native controls

Scene metadata retained decoded body/animation references after explicit unload.
A negative-first fixture demonstrates acceptance of a retired generation. Scenes
now bind body and attachment resource revisions and validate resident generations
before selection, ticking and drawing. Unload/reload requires a new scene. Last
accepted GPU output remains readable after refusal, and owned GPU resources clean
up. The resource manager/renderer must outlive scenes; this is native policy.

The [fresh combined result](third20-native-controls-20261020.json) passes11native
CTest cases and2ASan/UBSan cases. Qt/Mesa process leak detection is disabled;
explicit scene GPU-resource cleanup is asserted. The [original comparison](third20-original-controls-20261020.json)
executes unchanged selected ANI/SPR helpers:91SPR frames,195scene frames and390actor
states match. Five retained original RGB565 captures match complete CPU/OpenGL
destinations. Native transform controls cover150cases and17,550pixels, architectural
continuation and original-row trapping, strict facade admission and Qt restart.

Six serial task-owned PE32 suites pass:13version,19reservation,9mutator-entry,
55resolver,4real software graphics,16physical-owner,2source-lock and23update-route
cases, with additional private-handoff/parser controls. Deliberate unhooked-write
and distinct-resource alias escapes remain evidence of guard limitations.

A forced Wine prefix update timed out. The failed result and diagnosis remain
immutable; shared suite runners now initialize their verified task-owned prefix
without forcing repeated updates. Fresh successful results use this path. A
concurrent merge changed shared Git merge state during the first commit:ce0cd336
recorded both parents while retaining its first-parent/offline content. The next
change restores incoming Medium source/evidence versions, retains Low/offline
history and records exact retrospective receipts. No past combined gate pass is
asserted for that incomplete merge.

The [twenty findings](coverage/third20-findings-20261020.json) name each fix/control
and its remaining boundary. The native scene lifetime, capture-exit and COPY
identity/atomicity fixes are concrete. The other selected contracts already have
bounded implementations; new execution does not establish all original writers,
physical backing/aliases, full native/public game journeys, NVIDIA hardware,
sustained gameplay/FPS or live replacement. Their broad raw partial statuses are
retained. No original artifacts or balance values were modified.

The concurrent recoveryfc8a3493 restored Medium content before this final commit.
Its engine source versions match every affected fresh native fingerprint, so the
final change adopts that recovered history/metadata and adds the scene fix and
new evidence. Earlier intermediate static/census snapshots are retained without
claiming a past combined gate pass.
