# Opt-in copy contention and recovery timing

The reported first-mission failure names sessions 1516015097/1516015098: a 65 ms
copy acquisition wait exceeds the existing 50 ms budget, forwarding an original
draw while invalidating history; a 300 by 128 surface has no complete saved
pixels, checkpoint recovery refuses, and fresh-session history becomes invalid.
These are user-supplied findings. Their exact launch artifacts have not yet been
identified in this checkout. The buffer-padding fix and contention are separate.

The existing timeout's owner age is a sampled current owner, not the duration of
one continuous hold across the entire wait. No scheduler or reacquisition cause
has been established for that real launch. Timeout, conservative invalidation,
fresh/checkpoint policy and game behavior are unchanged by this increment.

`MNM_RENDER_COPY_TIMING=1` enables bounded diagnostic observation when a lock
capture directory is configured. Drawing callbacks append immutable fixed
records to 512 native slots without allocation, file I/O or waiting for a
consumer. The existing publication worker drains ready slots to
`lock-capture/copy-timing.bin`. An unpublished slot pauses the worker's next
finite drain; it never blocks the producer. Capture may end before draining.

The timeout records include GetTickCount budget measurements, raw high-resolution
clock points, wait calls/successful returns, and published acquisition counters.
An owner sample uses a bounded sequence/readiness/thread check, with an explicit
unavailable flag rather than mixing partially published labels. Counters cover
ordinary/bind acquisitions; separate shutdown admission is not instrumented.
Sampling and clocks perturb execution, so this remains opt-in observation.

Timing is captured before the existing timeout log performs I/O. Same-thread
reentry remains nonwaiting and does not masquerade as a timeout. The normal
acquisition limit remains 50 ms. A successful WaitOnAddress return does not prove
one wake notification or explain why the thread was scheduled late.

Checkpoint refusal, fresh recovery selection and tracked pixel discards add
records to the same timeline, carrying old/new session, surface generation,
epochs, known layout, dimensions, pixel-presence and source-bound call-site tags.
These tags/opaque addresses identify this build/run only. Ordinary complete
pixel replacements are excluded. Discard tracing arms at a timeout/checkpoint/
recovery boundary and uses only the first 256 slots; the remaining capacity is
reserved for fault timing. Records can be dropped, so this is bounded evidence,
not a complete drawing history. Missing pixels are never fabricated.

The controlled native probe includes actual production copy admission headers
with synthetic Win32 clocks/waits. It distinguishes a 65 ms wait spanning three
intervening published acquisitions and a final coherent 16 ms owner publication
age. Admission/reentry/timeout invalidation and LastError are checked with tracing
off/on; concurrent slot publication, bounds and unavailable owner samples are
checked independently. This is headless preparation, not original-driver
comparison, real scheduling proof, victory reproduction or a crash fix.

Read a retained log without modifying it:

```sh
python3 tools/copy_timing.py <experiment>/lock-capture/copy-timing.bin
```

The next live run must use the same buffer-padding fix and launch profile as the
reported failure, record actual declared sources/builds and preserve the two
session transitions. A pre-victory save avoids replaying unrelated gameplay.
