# Native command presentation and input responsiveness

2026-10-07. `HOST.native-command-responsiveness` is a native application policy,
separate from original input polling, drawing scheduling and simulation.

The reader formerly called `GlViewport::repaint()` inside every PRESENT callback.
This synchronous path can wait for painting/composition while processing a burst
of historical frames on the Qt GUI thread. The shell also waited for its next
16 ms frame timer even when decoded commands or acquired ring bytes remained.
These are confirmed source-level opportunities for delay, not a measurement of
the user's end-to-end latency or proof that all observed lag had this cause.

The reader now queues viewport updates; Qt can coalesce them and display the
latest complete GPU lease. Every command, PRESENT callback, resource transition
and sequence check still executes in order. Explicit framebuffer grabs in
diagnostic callbacks remain possible. Existing byte and weighted work bounds
remain: at most32 full-cost commands or256 small-update work units per poll.
Known pending work schedules a zero-delay single-shot continuation in the shell,
yielding to Qt events between batches. An empty queue waits for the normal16 ms
frame timer; recovery, fallback, movie playback and terminal cleanup do not spin
on stale pending work. Fallback and launcher exit stop queued continuations.

The [fresh synthetic execution](native-command-responsiveness-20261007.json)
records70 changing PRESENTs in three bounded polls, zero synchronous Paint events
inside the reader and delivery of a posted event between polls. All final
framebuffer pixels match the independently specified last frame after DELETE/END;
commands and PRESENTs are not skipped, terminal storage is zero and ordinary
readbacks/viewport uploads remain zero. The existing fragmented, checkpoint,
failure and sustained suite also passes:5,077 commands and74,631,284 wire bytes.
The [control execution](native-command-responsiveness-control-20261007.json)
uses the previous repaint implementation with the new test and fails on its
first synchronous Paint event, demonstrating regression sensitivity.

Actual Qt shell Launch-button fixtures publish4,202 records, with every frame
black until the final colored update. The final composed image is retained in
normal and fullscreen/high-DPI windows. This validates complete backlog drain,
rather than just an early healthy frame or acquired-byte acknowledgement.
Targeted XCB input tests pass. Separate normal/fullscreen fixtures exhaust three
native retries, reconnect an independent original-window stand-in, clear the
forwarded lease and deliver direct mouse/key input. Their recorded input timings
describe this isolated Xvfb fixture, not the user's driver or game latency.

No original game runs and no immutable artifacts are consumed. Producer GAPs,
tracker/ordered-copy waits, synchronous XCB input round trips, large indivisible
GPU operations, driver swap scheduling and actual-game end-to-end latency remain
separate boundaries. This change does not repair incomplete drawing history or
claim original-frame equivalence or live rendering replacement. Historical
execution records retain their source hashes; changed shared adapters can leave
those earlier results stale.

2026-10-07 performance observation: two 25-second original-game startup/Main-menu
runs used the real continuous v2 reader and 800x600 viewport in an isolated
Xvfb/Mesa llvmpipe session. Default and ordered copies delivered 390 and 374
PRESENT callbacks, approximately 19.5 and 19.6 intervals/sec after the first
frame. Busy polls measured 3.06/3.07 ms median, 8.19/8.48 ms p95 and
16.97/17.12 ms maximum. Neither measured interval refused its channel; ordinary
native/RGBA readbacks and viewport uploads remained zero. About 94% of sampled
consumer-process CPU time was in Mesa graphics/JIT code; Mesa worker threads
accounted for about 95%. Wine/producer CPU was not sampled. Each frame involved
approximately 259 native surface uploads, whose current implementation enters
and restores an OpenGL context separately. Context batching remains a candidate,
without a measured optimization result.

The host has an NVIDIA TITAN RTX and a loaded driver, but this experiment had no
authenticated desktop display and did not measure hardware acceleration.
From a desktop terminal, omit `--software-rendering`, unset
`LIBGL_ALWAYS_SOFTWARE`, `__GLX_VENDOR_LIBRARY_NAME` and
`__EGL_VENDOR_LIBRARY_FILENAMES`, then verify `glxinfo -B` reports the hardware
renderer. Xvfb software timing does not establish the cause of a user's desktop
slowdown. Battle throughput, producer CPU, original-frame equivalence and
end-to-end input latency remain pending. The independently bounded archive was
enabled for these observations; the normal continuous launch defaults it off.
The [immutable profile](native-render-profile-20261007.json) includes source and
artifact hashes, full poll/frame timings, CPU summaries and reproduction source.
The experiment cancels at its time limit; later cancellation diagnostics are
cleanup. This adds bounded observation evidence without promoting statuses.
