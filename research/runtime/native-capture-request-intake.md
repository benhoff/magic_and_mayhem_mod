# Off-GUI bounded diagnostic capture request intake

## Camera publication deadline repair — 2026-10-19

The camera helper waits ten seconds for two diagnostic PNGs and their metadata.
The former publisher used `QSaveFile` for each PNG and the acknowledgement,
placing durable disk synchronization before each file became visible. The
prospectively declared `test-camera-capture-publication.py` control runs the
actual unchanged `CameraDialogueGuard`: an injected eleven-second sync produces
the exact `Camera dialogue capture deadline` error with the former publisher
(ACK at 11.148 seconds). The current publisher completes at 0.073 seconds without
entering durable sync. Both 800x600 images decode to their independently expected
pixels before the observer accepts the complete metadata. Confidence: confirmed
for this delayed-writeback failure mechanism; no original pixel equivalence or
general filesystem/scheduling guarantee.

`native_capture_request.hpp` now flushes and closes a private same-directory
temporary, checks errors, and atomically renames it into place. Lossless PNG
compression remains zero. The worker publishes metadata only after both image
attempts; false save flags still reject readiness, and image or acknowledgement
publication failures now propagate through the existing future to the campaign
controller. The ten-second helper deadline, two quiet captures, physical input
witnesses, bounded worker queue and GUI/GL ownership are unchanged.

The retained failure in `campaign-4sewy0kh` reports only 11.209 ms on the capture
owner, but native and original PNG write-to-rename metadata gaps of 4.717 and
1.312 seconds. Those timestamps support publication latency as a contributor;
they do not measure each historical syscall or prove every older deadline had
the same cause. Existing captures, execution records and source hashes remain
unchanged. The new fault control and subsequent game journeys are separate
results; ordinary World completion and renderer performance remain separate.

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

New [execution record](camera-helper-publication-fix-20261019.json): 647 balanced Worlds, 617 CPU restorations and 14 accepted camera captures over two pinned normal journeys. Complete lossless native/reference artifacts and actual helper validation are retained per capture.
