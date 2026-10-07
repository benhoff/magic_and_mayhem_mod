# Input after native command refusal

2026-10-07. Native application policy `HOST.command-input-fallback`; no original
rendering, input polling, wire schema, retry budget or game balance change.

## Finding

The [retained diagnostic](native-command-input-refusal-run-20261007.json) for
`working/experiments/opengl-render/run-f619ca_i` used continuous
commands, movie playback and an 8 ms tracker wait. Read-only inspection of its
manifest, lifecycle log and mapped-channel headers confirms four failed v2
sessions (initial plus three retries), each with reason 2, GAP. Tracker timeout
records at lifecycle lines 62, 80 and 88 precede the subsequent refusals. The
requester/holder tags identify copy admission/commit and pre-Unlock capture when
mapped against this run's bridge source fingerprints. The sampled fields do not
prove exact ownership or separate capture CPU cost from scheduler pauses.
The user reports input lag only after the refusal. This is a timing report, not
an instrumented input-latency measurement. Mesa/Quartz warnings alone do not
establish the cause of the GAP. See [refusal diagnosis](native-command-refusal.md).

Before this change, the shell cleared the GPU frame, hid the viewport and
suspended its forwarded input after terminal refusal, but left the original
window outside the shell. Window discovery continued through the OpenGL input
target path, which could not reconnect a hidden, empty native viewport. This
explains the loss of the shell's game controls after refusal; the producer's
missing history remains a separate failure.

## Policy

A failed `LiveCommandSession::poll()` now stops native frame and input timers,
releases forwarded keys/buttons, publishes an inactive input lease and clears
the forwarding target. The shell then uses its existing foreign-window hosting
path to attach the single newly discovered original game desktop and focus its
container. Physical input goes directly to the original window. Native command
resources remain refused; no missing commands are ignored or fabricated.

The fallback enables Attach/Detach controls. Existing prelaunch window exclusion,
ambiguous-candidate refusal and missing-window retry/timeout checks remain in
effect. The presentation choice resets on the next launch; fallback does not
change the requested launch backend. Exhausted recovery retains the validated
last renderer error, including producer session and GAP reason, in its message.
Smooth/integer native texture scaling does not apply to the embedded original
window. Native media interception after fallback, unavailable/ambiguous X11
windows and original DirectInput behavior on the user's graphics driver remain
validation gaps.

## Execution evidence

`tools/test-native-command-fallback.py` runs the actual Qt shell against a
synthetic producer and an independent X11 client in a temporary fake repository.
It presents a complete native frame in each of four sessions, then marks each
session FAILED/GAP. Both windowed and fullscreen cases exhaust three retries,
attach the client beneath the shell, clear the forwarded input lease and held
keys, and deliver physical XTest mouse/key events directly to that client.
Observed receipt delays in this isolated run are approximately 1.6 and 2.2 ms;
these are synthetic delivery checks, not original-game performance results.

Reproduce on an isolated display:

```sh
xvfb-run -a -s '-screen 0 1920x1440x24' \
  python3 tools/test-native-command-fallback.py working/build/qt-shell
```

[Pinned synthetic evidence](native-command-input-fallback-20261007.json)
retains the executable and source hashes. Existing launch/scaling/input and
hosting contracts keep their independent historical statuses and hashes. No
original execution, driver equivalence, sustained gameplay or live replacement
is claimed by this test.
