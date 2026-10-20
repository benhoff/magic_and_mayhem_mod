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
