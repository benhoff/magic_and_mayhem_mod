# Automatic recovery from owned checkpoints

2026-10-06. Administrative RECOVER=1 now prefers a complete independently owned
checkpoint. The direct PE32 RenderRecover export retains its fresh-pixel policy;
explicit CHECKPOINT=2 retains strict refusal when complete state is unavailable.
No wire fields, versions, resource limits or timeout budgets changed.

After exclusive callback admission and old-worker shutdown/join, the producer
checks the candidate file and holds the tracker. It chooses the recovery policy
once, before claiming the candidate. Complete state uses the same predicate and
serialization as strict CHECKPOINT: current owned pixels/metadata/palettes, exactly
one primary, no unresolved misses, finite surface/pixel/byte budgets, resource
creation/binding before initial PRESENT and READY. Initialized source surfaces
remain available to subsequent drawing under fresh session-qualified wire IDs.

If completeness is unavailable, administrative RECOVER uses fresh observations
with current verified lifetime metadata. Borrowed resources, lifecycle uncertainty,
malformed candidates and gate collisions still refuse. Failure after claim or
checkpoint publication/worker startup does not retry the same candidate as a fresh
stream. Claimed file/session identities remain spent. The host waits for matching
READY and a native frame before resuming native input/presentation ownership.

The bounded lifecycle diagnostic `command_recovery_policy` records selected
policy in its argument field (0 fresh, 1 checkpoint) and expected session ID in
its flags field. These are opaque diagnostic values, not a new wire acknowledgement.

## Validation boundaries

The full administrative fixture adds static drawing, incomplete-state fallback
and claimed-checkpoint worker failure. Static drawing makes only two original
Lock/Unlock calls, then performs no further drawing: recovery must reconstruct the
latest independent original pixels from owned state. A duplicate displayed pixel
value is allowed only for the first frame of a new session; subsequent incremental
frames must remain strictly ordered. The incomplete fixture includes a descriptor
without pixels and verifies selection of fresh observations. Worker fault verifies
checkpoint selection, refusal, cleanup and no second candidate retry.

The startup fixture now waits for its explicit initialization marker before
consumer polling. A prior exploratory run raced initial original drawing and
safely refused recovery; its output is retained under working/ rather than counted
as a passing matrix. Collision, held ownership, timeout and cancellation tests
retain their adversarial schedules. A separate exploratory run was rejected by
the source-freshness check after an unused fixture branch changed during execution.
Fresh records below replace those runs without refreshing historical evidence.

The strict 20-case checkpoint matrix also checks unchanged incomplete/borrowed
refusal, mixed formats, palette recreation, overflow/stall/stale ACK recovery and
mapping/allocation/worker faults. Production PE32/Qt builds and selected control,
palette and live-channel CTests passed. Original-game experiments verify immutable
originals before and after and retain source/artifact hashes.

In the bounded game startup observations, early strict attachment still refuses
when primary pixels are missing. Administrative RECOVER after an observed complete
primary selects checkpoints for sessions 124, 125 and 126 and presents three native
frames, preserving initialized source baselines across the retries. Explicit
CHECKPOINT followed by automatic retries also presents three frames. Sustained
traffic still overflows the finite queue and exhausts the three-request budget;
consumer cleanup and original-window fallback remain intact. These are first-frame
and repeated recovery observations, not sustained gameplay or original pixel
equivalence. Throughput, pacing, gameplay/movie interaction and real-driver
equivalence remain pending.


Fresh evidence: [administrative matrix](opengl-auto-checkpoint-host-20261006.json),
[strict matrix](opengl-auto-checkpoint-strict-20261006.json),
[direct export regressions](opengl-auto-checkpoint-explicit-20261006.json) and
[bounded game observations](opengl-auto-checkpoint-game-20261006.json).
The administrative matrix passed 16 cases/189 independent GPU frames; strict
attachment passed 20 cases/294 frames; direct export passed 4 cases/20 frames.
Native/sanitized control-client checks and five protocol tests passed.
