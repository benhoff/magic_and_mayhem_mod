# Bounded background route image encoding

The native campaign smoke test retains route screenshots and independent native/
reference captures. GUI-owned reads produce QImage copies; one background job
saves their PNGs. GUI completion polls do not wait. Steps and scripted actions
advance after artifact results; identity overlap, failed writes and a five-second
encoding deadline refuse. No widgets, QPixmaps, GL contexts or GPU frame leases
cross into encoding. Normal Quit occurs after the last route capture completes.

`--synchronous-route-capture` retains the explicit comparison.
`--no-route-screenshots` remains diagnostic omission with no route pixel claim.
Combat/portrait diagnostic captures and source/19 FPS checks remain unchanged.
This addresses measured test-harness PNG stalls, not engine or raster work.
Current controls/full TITAN casting journey and defensive failure-path coverage
remain pending; no original equivalence, scanout or sustained 60 FPS claim.

## Persistent encoder and current validation

The encoder starts once before gameplay, runs one image task at a time, and
refuses more than two queued tasks. Route and diagnostic futures come from
packaged tasks, so releasing a future on the GUI does not join an async thread.
Valid diagnostic requests are removed on the encoder before its acknowledgement
is published. Images and paths are owned values; GPU/widget objects stay on GUI.

The first transient-thread version `run-ek1nbb6y` passes the full casting journey
but retains a 632.919 ms diagnostic capture gap after its approximately 8 ms
readback, plus one 55 ms presentation callback. Enqueue/thread reclamation and
request deletion were not individually timed, so the exact wait cause is not
proven. Its source-bound report is retained as historical intermediate evidence.

Final `run-cwgxy8t9` controls pass queue refusal, GUI progress with a blocked
encoder, exact PNG pixels, exception propagation/continued work, 128 actual World
word comparisons and native worker/lease/retirement/hitch checks.
`run-98mfpjv9` passes casting, combat, spell cases, portrait recentering/comparison
(maximum channel error 1), three menu cycles, fresh-source Quit-No and normal
Quit. All 5265 Worlds/imports/verified returns and GPU counters balance; no
ordinary readbacks, stable sources/assets and verified original manifests.

All 14 route captures retain their artifacts. Maximum GUI route capture is
20.248 ms, diagnostic capture 18.801 ms, GUI PNG encoding samples zero; worker
encoding reaches 621.652 ms while GUI remains available. Global native World rate
is 59.28694 FPS, visible native World rate 60.01656 and engine rate 19.37173 Hz.
Gameplay GUI paints average 76.92861 FPS (includes UI) with a worst sampled second
of 46.16896. One clipped 144.164 ms eligible paint gap remains unclassified, with
only 1.121 ms of diagnostic activity overlap. Native presentation callback max
32.409 ms, paint max 4.319 ms and compose-to-swap max 6.201 ms are observations.
Cold all-GUI heartbeat delays do not imply warm gameplay stalls.

This removes measured screenshot/diagnostic enqueue-cleanup stalls; it does not
claim the cause of every wait, engine acceleration or sustained 60 FPS. Defensive
route overlap/write/deadline branches retain incomplete live coverage.
