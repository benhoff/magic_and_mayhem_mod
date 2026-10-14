# Display imported native World while the next waits for command import

Opt-in `MNM_DISPLAY_WHILE_PENDING=1` / campaign `--display-while-pending`,
requires asynchronous native production. The completed/new source remains
pending until its ordered World command imports it. Display retains the prior
imported immutable World and uses only matching sealed UI generation; source
completion and display generation remain separate. Primary imports and redraws
advance the native display clock/counter; undisplayed source completion does not
count as a displayed World or cancel an old-generation redraw.

Source FIFO, verified returns, two credits/two immutable slots and the engine
clock remain unchanged. No import or simulation advances early. Missing imported
frames, stale/mismatched generations and ordinary non-async modes refuse; default
comparison behavior remains available. Consumer command leases and the retained
imported frame return before producer join.

The capture milestone found a 1008.957 ms paint gap with pending-import
suppression, short owner CPU phases, and an old sealed World/UI. This policy
removes that display gate; the cause of original World-command latency remains
separate. Current controls and a complete TITAN journey validate the scoped
native policy. Original equivalence, interpolation, scanout, a native simulation
engine, and sustained 60 FPS remain outstanding.

## Controls and first live failure

TITAN `run-7e4h0u1d` passes180ms authored delayed-import old red-frame
presentation, no early/new/stale/missing frame admission, unchanged source/import
credits and blue switch only on authored import.128 actual captured Worlds still
match all native words; GPU/worker/retirement/presentation/hitch controls pass.

First public `run-666f2v5_` exercises961 pending redraws, but fails later with
`Menu adapter retired` and engine17.10Hz/native55.36FPS at4864 balanced periodic
World counters. This is a preserved failure, not a complete successful journey.
The menu lease is unchanged at2000ms; the exact retirement cause is unconfirmed.

Two reviewed corrections follow: completed source admission no longer waits for
a display deadline, and primary import restarts its independent display interval.
Resident Quit-No automation requires three newly imported Worlds beyond the
post-answer base (past the two-credit backlog), plus display progress, instead
of treating cached-frame redraws as simulation resumption. Nonresident behavior
is retained. Final TITAN `run-q3vumvu1` repeats all controls with stable sources/
inputs/original manifests, including cached/backlogged resume refusal. The public outcomes below preserve both failure and success. Causal
distinct-input FPS, original command-delay attribution, original equivalence,
and sustained 60 FPS remain excluded.

Second public `run-23a79onx` completes the scripted casting/combat/menu/normal-
Quit flow with5012 balanced World/import/verified-return/GPU counts, zero ordinary
readbacks and no native error. It fails the unchanged19FPS gate at18.61690
engine Hz; do not claim a full suite pass. Native World57.81722FPS, visible
58.31342, mean gameplaypaint74.20617 and worst20.87114FPS are not stable60.
246 pending redraws exercise the branch. Largest retained350.149ms GUI gap
contains22 World frames,27 export refusals and short owner phases;342.439ms
gap has23 World frames and32 refusals. The renderer continues while GUI/output
leases stall, unlike the baseline1008.957ms pending-import suppression. Original
source throughput and exact GUI stall cause remain unresolved. An unchanged-
source full rerun assesses variability without relaxing the floor.

## Final unchanged-source public repeat

`run-s_dutjcz` passes the complete campaign runner, including casting/combat,
spell cases, portrait checks, three menu cycles, Quit-No fresh-source resume,
and normal Quit. Sources and asset inputs remain stable; original manifests
verify before and after. Hardware is zink on the NVIDIA TITAN RTX. All 5200
Worlds, imports, verified returns, and GPU submitted/produced/taken counts balance
at retirement; ordinary native readbacks remain zero. The branch performs 60
pending redraws. Engine rate is 19.16361 Hz and native World rate 58.77168 FPS.
Gameplay GUI paints average 75.957 FPS with a worst sampled second of 42.57246
FPS; these include UI presentation and are separate from the World rate.

The same final code previously failed the 19 Hz floor at 18.61690 Hz. This repeat
establishes one successful full journey, not stable engine performance or a
causal throughput improvement. Both outcomes remain registered.

Four retained warm paint gaps range from 233.075 to 352.023 ms. The largest
contains 22 native World frames, 27 export refusals, and one GUI pickup, with
short owner phases and no pending-import suppression. Each retained gap falls
outside the recorded gameplay rate windows. GUI pickup/output-lease stalls are
the next measured boundary; their exact cause is unconfirmed. Capture excludes
non-world, hidden, movie, and cold continuity, so it does not cover every UI
stutter. The static analysis embeds its reproduction script and raw report hash.
