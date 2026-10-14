# Native GUI delivery attribution

Measure presentation callback, route/diagnostic capture, reference/screen/native
readback and encoding activity on the same native steady host clock. Paint and
compose-to-Qt-swap spans and an independent 16 ms GUI heartbeat distinguish
callback work from event-loop scheduling. Opt-in trace only, bounded records,
no added pixel reads or live JSON serialization. These are host/driver wall
timings, not GPU execution or physical scanout.

`--no-route-screenshots` is an explicit diagnostic comparison: route steps remain
validated, their screenshot/pixel artifacts are skipped and marked. Casting,
combat, dialogue and portrait diagnostic captures retain their existing checks.
No source clock, engine floor, source-credit/import order, or native frame
ownership rule changes. Current controls, full public attribution and a paired
comparison remain pending. Original equivalence and sustained 60 FPS are excluded.

## TITAN attribution and paired route comparison

Current `run-wl2mlqc1` controls pass nested GUI activity spans, independent paint/
compose/heartbeat timings, unmatched-marker accounting, bounded hitch/tail
retention, 128 actual captured World word comparisons and worker/lease/retirement
checks. Public casting attempt `run-52xntuxp` ended in wizard death before combat
completion; forced runner cleanup did not retain a final frame trace. The failure
and periodic source rate are preserved, with no successful journey credit.

Movement/menu `run-rxh86c6c` completes three menu cycles and normal Quit, with
2972 balanced sources/imports/returns, unchanged sources/inputs, verified original
manifests, 19.55089 engine Hz and 61.19480 native World FPS. Largest retained
375.322 ms paint gap contains 164.434 + 202.028 ms of synchronous image encoding
and about 7 ms native readback. Owner rendering continues 23 Worlds, with 31
export refusals while GUI delivery stalls. Presentation callback max 0.644 ms,
paint max 7.323 ms and compose-to-swap max 10.061 ms do not explain that gap.
Five retained gaps fall outside gameplay rate windows. Total route capture max
643.492 ms; all-GUI heartbeat max includes cold launch and must not be treated as
a warm gameplay stall. No physical scanout inference.

The unchanged-source `run-d4chsl24` uses the same movement/menu workload with
route screenshot artifacts explicitly skipped. See its immutable report and
embedded-script analysis for rates and remaining hitches. This is a diagnostic
comparison, not equivalent route pixel coverage or a statistically established
FPS gain. Both runs preserve the 19 Hz engine floor. The next change will retain
route artifacts while moving their PNG encoding off the GUI thread.

## Retained screenshots after the fix

[Persistent background capture](native-route-capture.md) now validates retained
route/diagnostic artifacts with current controls and a full casting journey.
The route-screenshot-free baseline had zero eligible gaps at 61.56323 native
World FPS / 19.67746 engine Hz; its omission remains explicit. Final retained-
artifact casting run observes 60.01656 visible native World FPS, with one clipped
144.164 ms unclassified gap. Capture GUI maxima are below 21 ms, with no GUI PNG
encoding. The exact diagnostic wait after the earlier readback was not separately
attributed; persistent task reuse and off-GUI cleanup address that boundary.
Current statuses remain scoped live observation, not original equivalence,
engine-rate improvement, scanout or sustained 60 FPS.
