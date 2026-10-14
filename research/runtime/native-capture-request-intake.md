# Off-GUI bounded diagnostic capture request intake

A fully retained 230.309 ms paint gap overlapped 216.818 ms of request intake
before native readback. Existence/open/read/parse now run on the persistent image
worker. GUI polls a single future, keeps the validated owned name across busy
reference reads, and captures owned native/reference images on GUI. The worker
then removes the valid request before encoding acknowledgement. No new read
starts while that image job is pending.

The native policy bounds JSON to 4096 bytes and names to 1..79 UTF-16 units,
rejects slash/backslash/dotdot names, removes invalid requests on worker, and
retries missing/unopenable files. Opportunistic reader submissions skip a full
queue; required image tasks still refuse overflow. No widgets or GL objects
cross into intake, no source/engine clock/19 FPS/balance change. Current controls
and full TITAN validation are recorded below; the exact historical syscall and
original 144 ms cause, original equivalence, scanout and sustained 60 FPS remain
unproved.

## Current validation — 2026-10-14

Controls `run-hj3chstj` verify missing/valid/malformed/non-string/oversized and
traversal/long-name requests, valid-file retention, invalid removal on worker,
opportunistic queue refusal, blocked-worker GUI heartbeat, exact PNG roundtrip,
error propagation/continued work, 128 actual World words and ownership controls.
Request path is prepared before polling; GUI does not perform request-file
existence, open, read, parse or cleanup calls. A queued validated name persists
across busy reference reads, and the encoder still removes valid request before
acknowledgement. Encoding errors are now consumed from completed futures.

Final `run-rnwjsprp` passes casting, combat, spell cases, portrait recentering/
comparison (max channel error 1), three menu cycles, fresh-source Quit-No and
normal Quit. All 5407 World/import/verified-return/GPU counts balance. All 14
route artifacts remain; sources/assets stable, original manifests verify before
and after, ordinary readbacks remain zero. Hardware is NVIDIA TITAN RTX via zink.

Worst request handler GUI span is 17.594 ms, including diagnostic capture
(readback max 16.584 ms), versus baseline 224.411 ms. Whole campaign callback max
29.150 ms is route readback/capture; source/import/clock/engine gates remain
unchanged. There are zero eligible warm completed-paint gaps >=50 ms in the final
run. Global native World 59.05049 FPS, visible native World 59.78562 FPS and engine
19.36550 Hz are independent metrics; no causal mean-rate gain or sustained 60 FPS
claim. The exact original144ms event is not retrospectively attributed; the
reproduced230ms intake boundary is structurally removed from GUI. Cold/movie/
hidden/non-world continuity and physical scanout remain outside that gap proof.
