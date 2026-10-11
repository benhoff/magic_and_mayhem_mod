# Original engine pacing and World production experiment

Build: No-CD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

This bounded experiment observes whether accepted gameplay updates, the world
counter, eight global ANI controllers, creature maintenance/scheduling and World
queues share the original pacing cadence. It does not alter pacing, game speed,
simulation, draw skipping or rendering policy. No 60 FPS claim is made.

## Observation contract

An opt-in separately imported PE32 DLL patches fifteen pinned instruction sites
only after complete source-PE staging hash and all 76 anchor bytes match. Marker
thunks execute the original sequence/tick stores; no-argument call thunks preserve
registers, flags, x87/SSE and LastError around observations and execute original
routines once. Queue entry tail-forwards the existing scene hook with the original
return PC. Primary message-loop pacing at 00557403 is observed; other pacing callers
are excluded. The shared import clock and its guarded internal PCs remain untouched.

The sequence store is 0046b45f, world-counter store 0046b496, maintenance call
0046b735, eight ANI calls 0046b74f..0046b795, scheduler call 0046b79f, and queue
calls 004fd4ab/004fd554. ANI storage is the eight original globals
006deffc..006df14c (48-byte stride). These are not all creature/effect controllers.

At most 262144 96-byte events (24 MiB) remain in DLL-owned memory. QPC frequency,
thread identity, original counters, interval/companion/suppression fields and
selected controller/scheduler words are recorded. File writes occur only during
process detach after gameplay; no per-event logging, queue publication, capture or
GPU timestamp is used. Overflow, another engine thread, missing returns, malformed
headers or incomplete exit files refuse a complete trace claim. Millisecond target
and sampled wall time remain distinct. Trace timings include observer overhead.

The final accepted update window lacks a following boundary and is excluded.
Nested call durations must not be summed as independent stages. Alternate-screen
accepted updates are visible through the counter marker, but pause semantics and
unobserved branches remain unrecovered. Frame/UI/driver equivalence is excluded.

## Reproduction and controls

Run tools/test-engine-cadence.py and tools/trace-engine-cadence.py with fresh
prospective coverage claims through the main working-retention guard. The live
probe uses a private TITAN native resident Apprentice campaign, actual casting
and combat, portraits, menu resume and normal Quit, retaining existing 19 FPS
gates. Source declarations precede execution and original manifests verify before
and after original-consuming runs. Earlier evidence stays historical.

[Final-source PE32 controls](engine-cadence-controls-20261011.json) pass all seven
thunk paths, ten independent original forwards, flags/registers/512-byte x87/SSE
and LastError, both original marker stores, volatile-ECX ANI snapshots,76 mutated
guard bytes, bounded overflow and14 malformed decoder refusals. The earlier two
synthetic address-reservation failures occurred before hook execution and remain
separate negative records. Live coupling measurements remain pending. Lowering
the interval stays outside this experiment.

Exact prior committed range27ed7f8..e692a4f was reviewed against parent/current
hashes and receipts: nine files, no unresolved gaps. Retained report:
working/tests/engine-cadence-history-20261011.json; no retrospective gate/validation
assertion.


## TITAN live result and scheduling decision

[Fresh trace](engine-cadence-titan-20261011.json) and the independently retained
[full campaign](engine-cadence-titan-campaign-20261011.json) pass all unchanged
19 FPS journey gates. Actual Zombie creation, Fireball damage, melee/lethal combat,
invalid-target/insufficient-mana refusal, movement,five portraits,Mini resume and
normal Quit complete with4545 balanced GPU Worlds at19.29295 aggregate engine FPS.
NVIDIA TITAN RTX/NVIDIA Zink is accelerated, without software fallback. Sources,
private Wine shutdown and immutable originals verify. No delivery readback,
pixel reply or pending World remains; normal Quit retires the resident producer.

The final trace has154158 events (14799232 bytes including header),4664 complete
primary pacer calls, zero overflow/foreign-thread records, and4544 complete
accepted-update windows. The final accepted update lacks its next boundary and
is excluded. The same c4dc08519c4030903d0b34d9a7fd08a09d111c59d086dea9519816a2f2e0c993
observer DLL was used by the final PE32 controls and this live campaign.

| Observed original mode byte | Complete windows | World counter increments per window | Global ANI calls | Budget scheduler calls | World queues |
| --- | ---: | ---: | ---: | ---: | ---: |
| 0 |4355|1|8|1|1|
| 1 |189|0|0|0|1|

Every advancing window selects50 ms and companion20, produces its queue at the
new counter value, and observes suppression0. All34848 monitored ANI calls are
active and change controller state; these are eight globals, not all creature
or effect animations. Every non-advancing window also omits the selected creature
maintenance call and keeps its queue at the unchanged counter. Field+fd5a is
recorded as its low byte; the complete DWORD mode/transition policy is not inferred.
The accepted-update sequence still advances: all189 mode-byte1 windows are
followed by sequence+1. Thus the non-advancing branch does not leave all engine
state unchanged. This is confirmed bounded live call/counter correlation with high confidence,
not a complete simulation-time, pause or original-frame equivalence claim.

Accepted-update interval median is50.0002 ms,p9556.910 ms,mean51.8209 ms and
maximum1823.0709 ms including retained interaction/menu/capture tails. Complete
pacer wall time averages9.9933 ms. Selected scheduler average0.00707 ms and
maintenance0.00258 ms per creature call are small in this scene; remaining
simulation/effect/map/UI work is unclassified. These nested/instrumented
observations are not summed into an exclusive CPU budget or60 FPS prediction.

**Decision:** do not lower the shared pacing target as a rendering-only change.
Counter-driven creature deadlines, staggered scheduling and global ANI advances
are tied to accepted updates. More accepted updates would run that work more often.
A separate existing non-advancing branch does produce Worlds, so native simulation
is not a prerequisite to investigating higher rendering frequency. It is not yet
a safe render-only API: static0046b711 branches to0046b7c2, where another subsystem
can execute ten calls to005404a0, and later
world/effect/UI work continues. The ten-call branch must remain explicitly unresolved.
The next bounded experiment should isolate the original scene-production path
and compare gameplay counters, ANI event/controller state, scheduler phases,
entity state and other writes before/after repeated draws. Reusing the mode flag
or changing pacing remains outside the validated scope.

The prior exact e692a4f..7c0e46a committed range covers19 files with zero missing
receipts (journal self-excluded), retained in
working/tests/engine-cadence-live-history-20261011.json. No retrospective gate or
new execution assertion is made by that review.
