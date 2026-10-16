# Native campaign session

This is an authored native policy, separate from original campaign progression
and rewards. It coordinates the existing native skirmish with typed campaign
checkpoints. It does not reconstruct the original campaign, apply wizard
attributes to combat, award experience/items/spells, or change arena balance.

Content declares one ordered mission per realm/region. The player is skirmish
owner zero and the declared opponent owner one. Victory completes the current
region, transfers its campaign ownership to the player and selects the next
declared mission; the final victory completes the campaign. Defeat and draw
retain the selected mission and its initial owner so it can be retried.
Only a completed prefix is admitted on restoration.

`CampaignSession` owns progress and the optional active `SkirmishSession`.
Start, orders, ticks and restoration stage both before no-throw publication.
Resource factories must be deterministic/read-only apart from their returned
owned resources; this transaction cannot undo external I/O. Recursive mutations
are rejected. Failed creation, resource loading, resource identity checks,
altered restored bytes, invalid state or a throwing backend retain both owners.
The service is single-threaded; moves/calls from other threads are unsupported.

Start issues an odd attempt token using the next transition counter. A terminal
tick atomically records that token's result, increments transition again and
releases the battle. The result is emitted on that tick only. Idle ticks/reopen
cannot replay it; a retry receives a new token. Counter admission reserves room
for both start and settlement and refuses exhaustion without mutation.
These counters order continuation, not wall-clock time or original engine ticks.
Settlement is once per adopted checkpoint lineage. Deliberately loading an
older active save rewinds that lineage; there is no cross-process merge/lock,
external award transaction or promise to deduplicate events across save forks.

Version-2 campaign records add the active attempt token and optional last result;
see [the format](../formats/native-campaign-state.md). Version-1 blocks retain
their bytes and decoding. An old active battle with token zero lacks the session
identity needed here and is refused rather than assigned an invented attempt.
The general persistence API still accepts it as legacy campaign data.

An active restore first validates campaign identity, content roster, selected
mission, completion/ownership prefix and attempt/result ordering. The application
resource adapter then rebuilds a real skirmish through its normal navigation,
animation and health backend validation. The resulting world must encode to the
exact saved bytes. A terminal active battle is unreachable at a published
boundary and refuses. Idle/completed checkpoints contain an empty world and the
retained result, so completed battle resources need not be reopened.

The initial contract test uses an owned synthetic navigation adapter and actual
skirmish stepping. Explicit test-only low-health/projectile fixtures reach
victory, defeat and draw without adding production tuning. It checks pending
casts/projectiles, fresh session resource restoration, exact continuation,
wrong/missing/altering resources, throwing damage, invalid orders/content/results,
repeated start/settlement/reopen, retry and counter exhaustion. Actual frozen
arena binding and independent-process journeys are a separate integration step.
Original map/asset restoration, campaign GUI, arbitrary branching content,
original victory/defeat rules, power loss and injected file-system failures
remain outside this milestone.

Targets: `mnm-campaign-session`, `campaign-session-test`. Reproduce using the
main checkout's storage guard around this worktree's
`tools/test-native-campaign-session.py`; the optional `--arena` route is for
the separate native arena integration. No original artifacts are needed.
