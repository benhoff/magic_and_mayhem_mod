# Bounded command backpressure and stalled-reader refusal

2026-10-06. Native policy `NR.command-backpressure` is the fourth continuous
producer chunk. Wire v2 and its existing failure reasons remain unchanged.

## Policy

Ring FULL writes nothing. The producer keeps the complete ordered byte stream
in its owned 32MiB queue and retries; it never overwrites unacknowledged ring
bytes, skips drawing records, coalesces updates or waits for the peer in append.
The 1MiB mapped ring, 64KiB fragments, at most 1MiB per pump and finite monotonic
byte lifetime remain. Whole append admission refuses OVERFLOW before copying
when private storage is insufficient. The first queue failure remains sticky.

Every guarded pump now validates cancellation, identity and ACK monotonicity,
even when the private queue has drained. A small outstanding ring publication
can no longer leave cancellation or invalid ACK unchecked during idle. The
normal retry worker sleeps 1ms when queued data has room to progress and 10ms
when ring FULL or idle; each attempt remains bounded and uses the existing drain
guard. Queue peak and saturating FULL retry counters describe pressure.

`MNM_RENDER_CONTINUOUS=1` additionally enables a no-ACK deadline, default 5000ms.
`MNM_RENDER_STALL_TIMEOUT_MS` accepts decimal values from 10 through 5000; absent,
malformed or out-of-range values retain 5000. Default bounded/v1 publication does
not acquire this deadline. A pending private queue or unacknowledged ring bytes
arms the timer. Advancing validated ACK resets it; new producer traffic or merely
publishing more bytes does not. No pending work disarms it, so long idle periods
with fully acknowledged output are harmless. Unsigned elapsed time handles the
GetTickCount wrap. Checks occur in the guarded pump, never through a peer wait.
The idle worker observes expiration without further drawing callbacks.

Expiration refuses the open stream with existing INTERRUPTED reason 4. The worker
logs `command_queue_stalled` with reason, conservative queue peak and FULL count;
other observed queue failures log `command_queue_refused`. Cancellation and
malformed ACK retain their own sticky reasons. Already clean ENDED publication
is not subject to this timer: END/publication and private-copy ACK remain distinct
from decoder/GPU completion. Explicit shutdown retains its independent deadline
and join bounds and now also checks the private queue failure before reporting
clean completion. The native consumer's existing failed-channel cleanup and
original-rendering fallback remain the integration boundary; this does not
install new game lifecycle calls or restart a failed session.

## Evidence and boundaries

```sh
python3 tools/test-render-command-queue.py
python3 tools/test-render-backpressure.py
python3 tools/test-render-command-idle.py
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-continuous-producer.py working/build/render-ring
```

Native C queue/channel tests use deterministic clocks and independent byte
checks, including deadline equality/wrap, reset only on ACK, drained-idle state,
quiet cancellation/invalid ACK, configuration bounds and retained overload.
ASan/UBSan includes leak checks. Actual PE32 fixtures poison accepted source bytes,
then remain idle while an independent mapped reader pauses, progresses slowly,
fully drains or refuses. They check complete raw bytes, original API error
preservation, sticky reasons, timeout before explicit shutdown and worker logs.
A healthy actual producer-to-GPU continuity run separately checks full frames.
Fresh final-source records are registered under new IDs below.

No original assets are consumed. This timer measures validated transport ACK,
not GPU progress or process heartbeat, and is observed at the next pump rather
than a real-time scheduling guarantee. A permanently blocked or unscheduled OS
worker is not recovered. Hard overload still refuses; game-side pacing, safe
semantic coalescing, archive I/O stalls, queue resizing, automatic restart,
full counter exhaustion, lifecycle orchestration and sustained original-driver/
gameplay equivalence remain separate boundaries.

## Recorded result

[Queue execution](opengl-command-backpressure-queue.json) passes native and
ASan/UBSan runs with leak checks: eleven v1 writer cases and twenty-one v2 queue
cases, including deadline equality, ACK-only reset, drained-idle disarming,
clock wrap and configuration bounds.

[Actual PE32 backpressure](opengl-command-backpressure-pe32.json) passes all
eight cases. Pause/resume and slow progressing readers each verify 2,097,275
complete owned bytes after the caller poisons its source; slow ACK progress spans
more than the configured deadline without refusal. A fully drained 64-byte
publication stays open across idle. Stalls in both a full ring/queued backlog
and an empty private queue refuse before explicit shutdown, with timeout logs.
Cancellation, invalid ACK and whole-append overflow retain reasons 3, 5 and 1.
The final [default idle regression](opengl-command-backpressure-idle-regression.json)
passes all five cases. The [healthy GPU regression](opengl-command-backpressure-gpu-regression.json)
passes eight cases and 352 independent complete frames, including 73.7MB streams.

An initial native fake-environment edit hit the strict compiler's indentation
warning and was corrected; earlier execution records remain in `working/tests/`.
Final source fingerprints are retained separately from historical evidence.
The prior producer checkpoint 422be2a was reviewed across 47 committed files and 35
affected behavior receipts before this update; its journal remains append-only.
