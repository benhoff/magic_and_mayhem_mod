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

## Remaining gap attribution — 2026-10-14

Schema 4 covers whole campaign callback, progress publication, request intake,
window discovery, input heartbeat and render status pickup. Aggregate every
facade/status poll, but retain completed spans >=1 ms or one fast sample per
second/activity (kind 9); begin is end minus duration. Original ring/window
capacities remain unchanged. Authored 1 ms polling with a 144 ms slow progress
span now retains unclipped boundaries; aggregate counts remain complete.

TITAN `run-yowhbyub` passes these controls and 128 native World comparisons.
`run-t764kwik` stops on source producer cancellation after 1454 imported/returned
Worlds and has one outstanding source; preserve the negative and unknown cause.
It retains no eligible hitch and does not establish complete journey success.

Unchanged-source repeat `run-acopfvr_` passes complete casting/combat/spells/
portraits/menu/Quit at 19.54621 engine Hz / 58.75104 global native World FPS /
59.56581 visible native World FPS. A 230.309 ms warm paint gap is fully captured,
unclipped, including its post window. Request intake spans 224.411 ms, overlaps
216.818 ms of the gap, and ends after the native readback. Native diagnostic
capture overlaps only 0.988 ms of the gap; original reference read 0.682 ms and
native readback 7.609 ms. Renderer continues 15 Worlds with 17 export refusals.
Progress publication max 0.605 ms, status pickup 0.481 ms, discovery 5.372 ms and
input heartbeat 0.071 ms do not explain this stall.

This confirms request intake (existence/open/read/parse/future inspection) as the
blocking boundary for the reproduced gap. The exact filesystem operation and
the original 144 ms event are not individually proven. Move request intake off
GUI; keep owned native reads, strict artifact checks and performance gates.
Sources/assets/original manifests remain stable. No engine-rate, original
equivalence, scanout or sustained 60 FPS claim.
