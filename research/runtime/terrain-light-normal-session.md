# Normal-session terrain lighting

The existing finite file-capture experiment is independent of this opt-in
normal launch path. `MNM_TERRAIN_LIGHTING=observe` or `native` adds a checked
lighting DLL and one persistent native worker to the normal menu preparation
and game runner. The default remains `original`. The original world scheduler,
clock, loader and renderer keep their existing owners. Native graphics selection
and lighting selection are separate. This milestone does not enable default
lighting rollout or establish long-duration/arbitrary-map acceptance.

The fixed little-endian `MNMLS001` file has a128-byte header and three8MiB lanes
for source, reply and optional observed result. There are no shared pointers or
C++ object layouts. A fresh64-bit nonce, monotonic sequence, input checksum,
explicit lengths and complete cycle state identify each transaction. One engine
thread owns requests; the host owns replies. Acquire/release phases transfer
ownership. Refusal, timeout, sequence exhaustion or invalid reply closes the
channel permanently for that session. A late reply cannot revive it. The
original updater handles fallback. Unavailable or unsupported source snapshots
remain original and do not consume an admitted sequence.

Normal mode keeps fixed storage and no per-update trace. Optional diagnostics
retain at most32 complete source/reply/result records and64MiB, while the worker
continues past that recording budget. `first32` begins immediately; `reload32`
starts at caller00472201 only after the host selects the named original saved
row and arms capture. Original-active observation precedes current-source native
replacement comparisons. No selection flag or successful worker startup counts
as a replacement or original-equivalence result.

The normal supervisor verifies staged binary and compiler-source identities,
starts the worker before Wine, and stops/reaps it after the admitted original
process wait returns. Lighting requires that DirectPlay-profile process-wait
boundary so early Wine desktop return cannot end the worker while the game is
still active. The child authorizer passes only the checked channel, including
inside the existing private graphics boundary. Outcomes and failures are stored
in SQLite; a selected backend with zero admitted updates makes no live claim.
All binary staging checks the original executable hash and expected updater
entry bytes. Source installations and original artifacts remain read-only.

Recorded-input controls, installed live equivalence, launcher integration and
production rollout remain distinct evidence dimensions. Current execution and
remaining limitations belong in the central register and coverage ledger.

A diagnostic mailbox is available only after CLOSED. It requests one original
fallback input/result witness, without reviving the worker or applying a reply.
This permits independent original-code comparison after deliberate worker loss.
The host records worker PID/start-tick identity in the session SQLite lifecycle;
this is operational bookkeeping rather than a shared pointer contract.
