# Native source admission and completed-image delivery

## Confirmed defects and bounded fixes

The independent owned GPU path applied the supplemental 16.667 ms display
deadline before dequeuing an already verified engine snapshot. This could add
almost a display period to an otherwise ready source. Remove that admission
check; retain FIFO ordering, the pending WORLD import barrier, nonce/generation
validation, two snapshot credits, and the existing supplemental replay deadline.
The original 20 Hz pacer, simulation, animation, AI and combat are unchanged.

The render worker also cleared its completed-image dirty flag when both immutable
presentation leases were busy. Returning a GUI lease alone could consequently
leave the latest completed image undisplayed until another PRESENT. Clear the
flag only after a successful export. The worker retries the latest completed
presentation on later turns, using the same two buffers and existing cross-context
fences. Intermediate display outputs can still coalesce; source snapshots and
WORLD imports cannot. `presentation_exports_skipped` counts blocked attempts,
including retries, rather than lost engine updates.

These are intentional native scheduling policies, not recovered original engine
behavior. No runtime caller allowlist, executable, wire contract or gameplay
balance changes are included.

## Independent controls

`resident-source-admission-test` seeds an authored fixture at the verified FIFO
boundary and sets the display deadline one hour ahead. Two sources must consume
and import in order immediately; a pending import still blocks the second,
incorrect identities fail, credits balance, and supplemental replay still waits.
Wire verification itself remains the existing queue's separate responsibility.
The baseline fails because the first source waits for the deadline.

`native-frame-export-retry-test` holds the red GUI frame and a queued green
frame, then completes a blue presentation while both leases are occupied. It
applies a white surface update without PRESENT, releases the first lease, and
requires delivery of the completed blue image without another command or engine
update. Two allocations remain the bound and abort retires the GUI lease. The
baseline fails waiting for the third output. Both candidate controls pass on
software OpenGL and the M1 native backend.

## Live observations and boundaries

The accepted short pre-change battle window had 36–41 living creatures, 31
moving actors and 25 cross-owner melee hits: engine 19.57 Hz, distinct Qt World
swaps 19.57 Hz, worst distinct World-tagged swap gap 100.85 ms. These tag intervals are
not a pixel-based character-motion measurement or a matched scene benchmark.

With source admission corrected, a separate 15 second window had 29–34 living
creatures, 22 moving actors and 39 melee hits: source 19.99 Hz, Qt World swaps
20.04 Hz, source queue mean 0.642 ms / p95 1.219 ms / maximum 2.075 ms. Worst
distinct World-tagged Qt swap gap remained 84.28 ms. The scene and trajectories differ, so this is
scoped observation, not a causal live FPS speedup measurement.

The combined-fix `final01` run accepted all 12 summons and completed six battle
phases with 206 observed cross-owner damage events. Its final scripted 15 second
battle window had 28–30 living creatures, 23 moving actors and 53 hits, with
19.48 Hz engine production. The diagnostic stop helper refused a duplicate
native role; the existing campaign later restored menus and both original
process and launcher exited zero. The bounded timing ring no longer covered
that early battle, so this run alone supplies no battle Qt swap-gap proof.

A second combined-fix run (`final02`) accepted 12 summons and completed all six
battle phases with 124 observed damage events. Its final 15 second scripted
battle window had 21–22 living creatures, 12 moving actors and 10 melee hits:
engine 20.00 Hz, distinct World-tagged Qt swaps 19.89 Hz, source queue mean
1.052 ms / p95 2.721 ms / maximum 5.900 ms, native submit mean 5.942 ms.
The first 15 seconds of battle had 24–28 living creatures, 15 movers and 28 hits
at 19.96 Hz; the GUI tail did not retain that earlier window. Neither window
is a matched maximum-population benchmark. A controlled, nonce-verified write
to the existing cancellation word retained terminal timing after measurement;
original status 95 and runner interruption 143 are intentional diagnostic stop
outcomes here, not normal Quit or the earlier unsupported-raster refusal.

The largest final-window distinct World-tagged swap gap was 100.964 ms
(generation 2690 to 2692). Qt continued swapping every approximately 16.7 ms
through it, with intervening exports tagged as UI output (generation zero).
Thus this tag gap does not prove a 101 ms visible freeze, a dropped original
source, or stalled native drawing. Pixel-based motion continuity or stronger
completed-source attribution remains separate. Source generation sampling
missed zero completed sources in the window. Baseline retained-trace analysis
finds source queue mean 8.340 ms / p95 15.826 ms / maximum 35.257 ms; differing
scenes prevent interpreting the live figures as an isolated causal speedup.
The deterministic before/after controls establish the scheduling defects.

Exact raw observations, source revisions and tested executables are retained in
SQLite under `working/native-frame-pacing-20261113/`, with verified local backups.
The prior pre-change observations remain under `working/live-heavy-fps-20261113/`.
Generated registered reports under `native-frame-pacing-results-20261113/` are SQLite
references, not expanded reports.

Remaining limits: original source intervals can themselves exceed 50 ms;
interpolation and original-engine optimization are outside this patch. Qt
`frameSwapped` measures completed application swaps, not physical scanout.
Earlier trials stopped at the existing indexed-raster guard for caller
`0x54a563` (reason 43, tag 14, callee `0x59521a`). Its input/ABI recovery and
original comparison are still required before widening admission. This patch
does not establish universal smoothness, long-session stability, original pixel
equivalence, or full live replacement.

The current coverage count/warning snapshot is retained through the SQLite
[audit summary](native-frame-pacing-results-20261113/audit01.json). Historical
dated global summaries and unrelated proof staleness remain unchanged.
