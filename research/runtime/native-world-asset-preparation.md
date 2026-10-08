# Native World asset preparation

2026-10-08. This is an intentional native scheduling/ownership policy. It adds
no original addresses, hooks, drawing bypass or gameplay behavior changes.

## Implemented boundary

`ResourceManager::request` snapshots an immutable recipe, store and loader limits
with a private binding token. `prepareResource` opens worker-owned streams, reads
in at most 64 KiB requests, optionally verifies the complete image SHA-256 before
decoding, checks complete SPR/ANI/TTD pairing and computes charged residency.
The move-only result owns encoded image bytes and decoded output. A checkpoint
can throw before/between reads and between decoding stages to cancel preparation.
Individual decoder calls remain bounded by their existing limits, without new
intra-decoder interruption.

`adopt` runs on the manager's owner thread. It checks the private binding token,
residency/count/byte budgets and revision availability before publishing the
complete decoded result. `unload` and `unloadAll` invalidate outstanding tokens,
including requests for resources that were not yet resident. Another manager
cannot adopt the result, even with the same root and recipe. Failed adoption does
not consume the prepared resource or change counters/revisions. The synchronous
`load` path uses this same preparation/adoption implementation.

`WorldPreparation` owns one CPU worker, one outstanding job and one completion.
The worker first constructs the immutable v4 SPR `WorldCatalogue`. An owned
packet then produces a decoded World frame and the needed pinned file recipes.
The GUI binds recipes and submits requests only for absent decoded resources.
The worker verifies each source, decodes it once, indexes its encoded/visual
identities, resolves all draws, and releases encoded scratch before completion.
Identity indexes persist for warm frames; decoded pixels move to the GUI manager.
Catalogue scanning and lazy preparation therefore each read a requested SPR
once; GUI adoption never reopens or redecodes it. Unrequested files are indexed
without full pixel decoding. The offline `WorldResources`/`SnapshotResources`
paths retain their synchronous behavior and separate validation boundaries.

The live scene and resource cache use `residentOnly` access. Missing CPU data
refuses admission/drawing rather than falling back to synchronous file loading.
The ordinary synchronous scene/cache defaults remain available to offline callers.
Scene geometry/composite preflight remains complete before any scene execution;
the previous completed GPU lease remains available during successful preparation.
Normal capability refusals 7/8 still clear unsupported presentation and wait;
verification refusals, malformed data, pin mismatches and budget errors remain
fatal. Native output/oracles remain separate, with no original pixel seeding.

Only one pending packet is consumed until its prepared scene finishes, preserving
the v1 channel's `presented == last consumed` acknowledgement rule. This does not
recover dropped queues or establish retained-canvas correctness. No wire change
is made. An ended producer remains pending until its consumed packet completes;
the Qt loop cannot exit merely because the producer ended during CPU preparation.
The file-free adoption fixture also checks this final-packet drain.
`catalogue_ms`, aggregate `asset_prepare_ms`, `resources_prepared` and
`max_poll_ms` distinguish worker work from GUI polling. Packet planning, driver
latency and full event-loop percentiles are not individually timed yet.

## Cancellation and memory

Close sets the stop flag and channel cancellation without joining the worker.
The detached worker owns shared state, store, indexes, jobs and callback captures;
it borrows no manager, widget, renderer or channel mapping. Stopped state cannot
publish a completion and cancelled state cannot deliver an already queued result.
The worker releases its state after the current bounded decoder/file operation
reaches a checkpoint. Callback captures must own their lifetimes. Slow filesystem
calls cannot be forcibly interrupted; cancellation does not promise immediate
worker retirement or process-wide drain before application shutdown.

Missing-resource count is checked before dispatching individual preparations;
completed decoded bytes cannot exceed the GUI manager's available residency.
Only one asset is decoded at a time; a resource that exceeds the remaining byte
allowance is rejected before its result enters the completion collection. Existing
SPR input/decoded/scanned limits, 256 catalogue files, catalogue byte/frame limits
and 65,536 indexed prepared frames remain enforced. Encoded scratch is released
after identity indexing. These are component bounds, not a total RSS budget:
the in-progress decoder, encoded/Qt copies, metadata, recipes, owned packets,
GUI residents and GPU storage have additional allocations. No automatic CPU
retirement, speculative prefetch or process-wide reservation scheduler exists.

## Evidence and remaining work

`tools/test-world-preparation.py` runs resource preparation, worker ownership,
Qt lifecycle and dependency regressions using synthetic assets only. Tests cover
SPR/ANI and bitmap preparation on workers, adoption after file deletion, foreign
and stale tokens, cancelled reads, immutable source pin failure, completion
budgets, cold/warm identity reuse, blocked catalogue/resource cancellation, GUI
heartbeat during a held cold load, resident-only cache/scene refusal, complete
pixels, resize, strict mismatch, capability refusal recovery and cleanup.

Recorded results and their source hashes are separate from historical live-game
and original comparisons. No current installed-game latency reduction, sustained
resource churn, driver/context-loss or original rendering equivalence is claimed.

The [first retained synthetic execution](native-world-asset-preparation-20261008.json)
passes all 68 CTests with stable source/claim fingerprints. The held-load Qt case
records 46 timer callbacks during its 50 ms observation and a 1 ms close. These
measurements describe the injected blocked worker fixture, not installed-game
or physical-driver performance. The initial run's invalid dotted bitmap resource
name failed canonical-ID validation; correcting that test fixture and rerunning
the complete suite produced this new result. Its failed report remains under
`working/tests/world-preparation/run-9mah8p90/`.
That result precedes the final producer-end drain fix and retains its original
source fingerprints; it cannot establish validation of the changed drain code.
The [final drain regression execution](native-world-asset-preparation-final-20261008.json)
reruns all 68 CTests successfully with stable sources and prospective claims,
including the ended-producer case. This is the current synthetic validation;
the first execution remains historical evidence of its exact version.

The next [upload preparation milestone](native-world-upload-preparation.md) now
moves sprite and projected-shadow plane construction onto the worker and limits
GUI colour/coverage row transfers. Its execution evidence is separate; the
68-test reports above retain their historical hashes after these source changes.

Remaining work: move or budget large scene preflight, canvas allocation and packet
copies; add CPU retirement and total preparation reservations; measure installed
cold/warm World sessions and individual driver calls. GPU allocation/drawing and
cooperative elapsed checks remain on the GUI thread.
