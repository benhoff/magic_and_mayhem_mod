# Resident command queue pressure

The retained [Initiate failure](mini-dispatch-initiate-queue-witness-20261015.json)
confirms a full 1 MiB mapped ring and a private queue peak of 33,542,596 of
33,554,432 bytes, with 469 FULL retries before OVERFLOW during rebuilt Main Load.
The exact refused packet was not captured. This is private backlog exhaustion,
not the separate cumulative 32-bit stream limit. Existing append refuses an
entire append before copying when space is insufficient.

The native policy `NR.resident-command-pressure` checks backlog after acquiring
ordered copy admission, before opening its trace/status deferral or taking the
surface tracker. At 8 MiB it releases and wakes copy admission, then waits for
the independent publication worker to reduce backlog to 4 MiB. It reacquires
admission and rechecks pressure before admitting the original callback. The
50 ms copy contention clock is separate from the shared 5,000 ms pressure budget
for that admission. Sleep occurs without either copy or tracker ownership.

The wait reads private counters only. The worker retains mapped-ring publication,
peer identity, cancellation, invalid-ACK and no-progress checks. Callback release
still avoids inline mapped publication. LastError survives admission and release;
existing wrappers retain original-once forwarding on refused observation. Failed
streams remain terminal. No records are dropped, overwritten or coalesced; queue
capacity and wire contracts are unchanged.

This applies only to resident sessions with ordered copies and an active worker.
Finite/nonordered paths retain their existing behavior. Same-thread reentry never
waits. A single or nested callback can still exceed available 32 MiB storage;
that remains a whole-append refusal, now with exact append lengths and private
cursors recorded by `command_queue_overflow`. Sustained reader stalls still
terminate, rather than extending no-ACK deadlines. This is intentional native
scheduling, not recovered original-driver ordering or whole-engine equivalence.

The actual C control compares the old unpaced schedule, which refuses after 31
packets, with paced callbacks transferring 83,895,920 bytes through an independent
byte oracle after immediate caller-buffer poisoning. It checks released owners,
LastError, cancellation, invalid ACK, budget expiry and tick wrap, same-thread
ordinary/bind reentry, and nonresident/nonordered/no-worker bypass. It also runs
the existing 11 writer and 25 queue controls under native and ASan/UBSan builds.
Synthetic PE32 controls use a 10 MiB + 123 byte owned enqueue before actual copy
admission: a 150 ms paused independent reader must resume losslessly, while
stalled, cancelled and invalid-ACK readers retain their sticky terminal reasons.

Live reproduction and final source-bound results are recorded below when
completed. The old failed Initiate evidence remains immutable. Earlier
unclassified Preferences observations are separate and are not repaired by this
queue policy. No original comparison or replacement milestone is claimed here.

The [final controls](resident-pressure-final-controls-20261016.json) pass native
and ASan/UBSan runs, four PE32 pressure cases, eight existing transport guards,
and 17 storage plus 21 campaign portable tests. The live validator checks joined
wait/resume totals, since the existing diagnostic logger retains only four
distinct rows per reason; bounded or deduplicated rows are witnesses, not totals.
The raw overflow control now accepts an already FAILED producer before its first
publication, while still requiring reason 1 and the expected child result.

The [bind/publication regression](resident-pressure-bind-final-regression-20261016.json)
passes short-owner admission, original Lock/Unlock results/LastError, poisoned
borrow ownership, exact V5 FIFO/archive, zero callback pumps and clean shutdown.
Its old fixture lacked the now-required native-history callback and expected an
obsolete V4 header. The fixture now supplies a test-only callback which verifies
the pointer, operation, generation and nonce; production exports and native
history semantics are unchanged. The [retained failed probes](resident-pressure-fixture-findings-20261016.json)
record those two separate fixture mismatches without claiming tracker contention
as the initial cause. The [copy timing regression](resident-pressure-copy-timing-final-regression-20261016.json)
also passes timing/reacquisition/deferred-release checks, four decoder refusals
and both PE32 builds; its zero transfer-wait stub restores the existing default
fixture contract.

Two initial cold Initiate runs pass the formerly failing complete journey:
[ordinary](resident-pressure-initiate-live-20261016.json) has 916 balanced Worlds,
three waits/resumes, 8,785,318 peak private bytes and a 403 ms maximum wait;
[500 ms delayed capture](resident-pressure-initiate-delayed-live-20261016.json)
has 832 balanced Worlds, four waits/resumes, 9,013,388 peak bytes and a 365 ms
maximum wait. Both replace the original World canvas, resume fresh native Worlds
and complete normal Quit, with zero recovery, fallback or other-pixel forwarding.
They meet the declared 5 FPS functional floor at 12.929 and 11.974 aggregate
engine FPS respectively under llvmpipe. These records retain their original source
hashes; subsequent fixture-only corrections are not retroactively applied.

The [frozen final-source repeat](resident-pressure-final-initiate-live-20261016.json)
passes the same delayed cold Initiate journey after the fixture corrections:
962 balanced Worlds, 3 waits/resumes, 9,035,414 peak private
bytes and a 273 ms maximum wait. Normal Quit joins the publisher and
drains the stream cleanly. Inputs/sources remain stable; original manifests
verify before/after and the private Wine session stops. This establishes the
previously failing bounded route, with the single-callback, no-progress and
concurrent lifecycle limits above still explicit.

The final raw driver fingerprints a broad repository source superset, including
three concurrent uncommitted Cure files. They occur in neither prospective
behavior declarations nor any compiler receipt. The final evidence's explicit
identity source bindings exclude only these unused superset entries; the raw
report and all its source hashes remain intact. All declared and compiled inputs
remain bound. This keeps the renderer result scoped independently of that work.
