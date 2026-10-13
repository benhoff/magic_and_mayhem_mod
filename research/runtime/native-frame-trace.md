# Public native frame timeline

Opt-in `MNM_NATIVE_FRAME_TRACE=1` (`test-native-campaign.py --frame-trace`,
independent resident World required) measures the public dedicated render owner.
It retains at most 16384 meaningful worker turns and 8192 GUI events in tail
rings. Aggregate phase timings cover every turn. No source pixels are captured.
Serialization happens at joined completion/cancellation, outside the hot loop.
The regular GUI status report does not copy the trace rings.

One native steady-clock origin covers World polling, commands, extra World/UI,
immutable export, status publication, actual condition-variable waiting and
inter-turn gaps. Export serials join GUI pickup, GPU widget paint begin/end and Qt frameSwapped
events tagged with the last completed widget paint;
coalescing can skip exports, and repeated swaps are not distinct World frames.
Qt frameSwapped is not physical scanout. Active visible rates count native World
work separately from UI-only exports and GUI swaps.

The capture owner records acquisition and readiness timestamps only when enabled.
Readiness follows source decode, handoff, ACK, verified original return and slot
release. Queue delay starts there and ends at native dequeue. Existing original
uint32 millisecond capture/ACK/return durations stay in their own clock domain;
wrap subtraction is defined, and no host/original timestamp subtraction or
original CPU-time attribution is made. Native producer/prepare/submission costs
are copied from the source's completed profile. CPU wall time includes driver
waits; this trace does not measure GPU busy time.

The default trace-off path preserves source FIFO/two credits, original pacing,
World/HUD rendering and immutable two-buffer handoff. Ring controls independently
exercise wrap, retained order, overwrite counts, phase monotonicity refusal,
status/sleep accounting, active frame counts and disabled admission. Existing
worker pixel/lease/refusal/movie/teardown controls also capture the GUI lifecycle.
Current gameplay, tracing overhead, exact pixels and sustained60FPS remain
separate milestones; all failed or interrupted observations are retained.

## Initial TITAN observation

The full functional journey balances4343World returns with zero ordinary readbacks, original/source integrity and cleanup. Engine18.739FPS fails19; overall native48.913FPS and visible native49.438FPS fail60. Visible owner time230.551s includes54.749sWorld,29.969scommands,39.611s extraWorld/UI,2.076sexport,19.873sstatus publication,84.078swait and0.196sloop gaps. Status construction costs0.250ms per owner turn, repeated79345times; it is an observed avoidable cost, not yet a proven cause of low FPS. Native frame interval median17.804ms,95th36.780ms. GUI paintmean0.225ms, source queue delaymean9.754ms. Wait time includes intentional polling sleep; original coarse durations and GPU execution remain separate. Current tracing overhead and a causal scheduling fix remain pending.

## Targeted status cadence policy

HOST.native-worker-status-cadence bounds regular heavy status snapshots to16ms while preserving immediate initial/lifecycle/recovery/result-availability/media-pause/normal-Quit/stop/error publication. It does not wait or throttle source/command processing or immutable frame delivery. Busy rendering may delay a nonterminal report beyond16ms; joined terminal report is unconditional. `MNM_WORKER_STATUS_EVERY_TURN=1` and `--worker-status-every-turn` retain the previous policy for controlled comparisons. The gate independently tests63publications across1000authored1ms turns, urgent7ms publication/restarted cadence, clock regression refusal and every-turn baseline. Paired full native framebuffer hashes and current gameplay/performance remain pending.

The16ms policy remains experimental: select `MNM_WORKER_STATUS_CADENCE=1` or `--worker-status-cadence`. Every-turn construction remains default because current full regressions include startup timeout, missing regression fingerprints, drawing-history GAP, genuine covered-displacement bounds refusal and player-wizard death. A full functional traced run shows4498balanced returns,19.3018source/50.6438nativeFPS and82%less status wall work, but its overall asset-fingerprint validation fails. These observations do not justify default adoption or60FPS. Regression runs now explicitly enable full asset fingerprints without changing normal launcher staging.
