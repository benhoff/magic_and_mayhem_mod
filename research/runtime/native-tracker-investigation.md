# Native capture tracker contention in run-tvc6w3zj

2026-10-07. Investigation of retained user startup and recovery, alongside
bounded fresh synthetic and original observations. Original drawing remains
active. No scheduling default, wait budget or executable code is changed here.

## Confirmed within the retained trace

[User evidence](native-tracker-user-investigation-20261007.json) matches all 61
captured runtime C/header source fingerprints to the current files. Initial
session 1634732869 and recovery session 1634732870 both end FAILED/GAP. The
initial reader acknowledged 12,423,056 bytes; recovery acknowledged zero bytes.
This distinguishes an initially progressing native stream from failed restart.

The first wait-origin entry at lifecycle line 62 maps request `0xc73d012a`
to `lock_surfaces.h:298`, post-original Blt/BltFast commit admission, and sampled
holder `0x916700bf` to `lock_lifecycle.h:191`, pre-original Unlock capture. The
8 ms timeout is followed by pixel-tracker contention, session invalidation and
wire reason 2. Internal `session_gap` detail 3 means invalidated session pixels;
it is not wire reason 3 (CANCELLED).

After fresh-session recovery, line 72 maps request `0x91670090` to
`lock_lifecycle.h:144`, successful original Lock descriptor admission, against
sampled holder `0xc73d012a`, post-blit commit. Its measured wait/hold age is
10 ms against the configured 8 ms budget. The following original Unlock is
unmatched, its target invalidates, and the recovered command stream refuses.
Recovery restores transport/checkpoints, not the competing callback schedule.

Four retained hold-origin entries measure post-blit commit at 13, 9 and 11 ms
and post-Unlock commit at 16 ms. This confirms that guarded work can outlast
the admission budget. Remote thread/site/time fields are independent atomic
samples, not a linearizable ownership certificate. Durations include scheduler
pauses; no CPU, disk or graphics-driver bottleneck is established.

Both refusal records report GAP, not queue overflow or stalled-reader timeout.
Queued peaks of about 1.9 and 2.0 MiB remain below the 32 MiB local queue limit.
Ring backpressure occurred but does not establish the cause of the lost pixels.
No original `DDERR_SURFACEBUSY` log was produced by this run. Mesa EGL probe and
Quartz warnings remain separate; the manifest did not force software rendering
and does not identify the Qt GL renderer or retain ordered-copies selection.

## Critical-section work and mitigation boundary

Post-blit commit includes generation checks, owned pixel updates, native command
serialization, and primary mirror publication. `lock_palette.h:38` publishes
RGBA regions/full frames through `render_pixels` while the tracker remains held.
This mirror still runs in continuous native-command mode, even though Qt's
native consumer presents a GPU texture. Pre-Unlock capture allocates and copies
application-owned rows under the same tracker. Neither path waits across an
original COM call while holding the tracker.

Reducing or deferring unused RGBA mirror work is a performance hypothesis;
changing it must separately preserve frame-readback/observation consumers,
ownership, generations and failure handling. This investigation does not
attribute a measured share of the long holds to conversion. Diagnostic I/O
and process scheduling are other possible contributors.

The existing `MNM_RENDER_ORDERED_COPIES=1` lease covers the implicated Blt,
Lock and Unlock callback transactions. It serializes callback admission outside
the tracker, never application execution between Lock and Unlock. Its 50 ms
timeout still forwards once and invalidates uncertain history; same-thread
reentry does not wait. It is an intentional scheduling policy and remains
opt-in. Increasing the tracker timeout or suppressing GAP is not a validated fix.

## Fresh bounded execution

[Four synthetic fixtures](native-tracker-ordering-synthetic-20261007.json) pass
cross-thread copy and Lock ordering plus copy/Lock timeout refusal, with six
independent complete-frame comparisons, two valid and two refused sessions.
The first attempt used an older consumer and failed pixel hashes; rebuilding
the consumer and rerunning all four passed. This does not establish driver
equivalence or reproduce the original timing failure.

[Paired original observations](native-tracker-paired-observation-20261007.json)
use the same source/build and isolated Xvfb/Mesa environment, with movies enabled
in configuration. Both reach an active Main menu, recover once from the harness's
deliberately injected reader cancellation and continue publication. Unordered
mode ends at 212 native frames/55,035 commands; ordered mode ends at 215 frames
and 55,561 commands. Neither records an incidental tracker timeout or GAP.
Both independently compare a stable Main-menu region and verify all 2,927
original files before and after. Neither detects calls from the sampled movie
routine, so successful movie playback is not validated by these observations.

Reproduce the existing option on the user's desktop for further evidence:

```sh
MNM_RENDER_ORDERED_COPIES=1 ./tools/run-qt-shell.sh
```

This is a mitigation trial, not a proven fix. NVIDIA desktop reproduction,
movie callback interleaving, repeated runs, active gameplay, per-stage tracker
timing and complete original/native pixel equivalence remain pending.
