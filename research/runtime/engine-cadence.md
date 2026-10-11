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

