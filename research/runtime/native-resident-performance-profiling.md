# Resident World bottleneck measurement

This work measures the 16.667 ms budget needed for 60 distinct World frames per
second. It changes diagnostics, not game speed, draw skipping or pacing policy.
`NR.resident-stage-profiling` is an intentional native diagnostic policy; original
comparison, live replacement and hardware-general 60 FPS remain separate claims.

## Measurement boundaries

`GlBlitter::profileBatch` brackets renderer-context work with two `GL_TIMESTAMP`
queries. `pollGpuTimings` first checks `GL_QUERY_RESULT_AVAILABLE`; it never waits
for an unavailable result. The ring holds at most 64 pairs. Overflow executes the
batch without timing and reports the omitted diagnostic. Invalid zero/duplicate
identities and nested profiling refuse; an operation exception deletes its pair.
Renderer teardown also deletes pending queries. Ordinary rendering uses no queries.

These timestamps measure elapsed GPU timeline intervals, including GPU idle gaps
while the CPU prepares/submits work. They do not measure hardware busy utilization
or public command composition, Qt paint, compositor, monitor or input latency.
Host `native_submit_cpu_ms` remains a CPU wall interval. Producer, preparation and
submission observations are nested inside native consume and must not be added to
it again. Decode plus handoff plus native consume is the measured host World path.

With `MNM_RESIDENT_STAGE_PROFILE`, the public resident service retains at most
8192 generation-keyed rows. Its existing original channel ticks provide capture,
ACK wait, return, preceding pacer and screen-copy intervals. The original guarded
clock call sites are described in `world-tick-loop.md` and
`runtime/scene/world_continuous_clock.c`; this task does not change them. Unsigned
32-bit subtraction preserves tick wrap. Clock resolution is milliseconds. Missing
pacer masks are reported as unobserved, not as proof of zero wait. The preceding
between-World interval less its observed pacer and screen-copy time is an
**unclassified residual**, which can contain simulation, UI, dispatch and other
waits; it is not a measurement of useful simulation CPU time. Negative residuals
are retained rather than clamped. Menu pauses and long tails stay in summaries.

## Repeatable probes

Run through the main checkout's storage guard (allow for the complete bounded
capture and private game installation):

```sh
python3 tools/profile-native-resident.py --capture --cpu-sample
python3 tools/benchmark-native-resident.py --campaign <campaign-evidence-directory>
```

The first probe uses a private accelerated TITAN display and the existing physical
native campaign casting/combat, Fireball/refusal, movement, portrait, Mini Cancel
and normal Quit journey. It preserves the 19 FPS gates. Optional 99 Hz user-space
CPU stacks sample 30 seconds after 40 seconds of physical gameplay. Capture enqueue/compression work,
timestamps, growing report writes and sampling perturb this run; it cannot replace
an uninstrumented performance proof. Prospective behavior/scenario source claims
precede launch. Preserve failed probes without relaxing their admission gates.

The offline probe freezes source inputs and builds an optimized resident binary
and NVIDIA EGL Qt platform plugin. It refuses desktop use or a non-TITAN fallback.
It requires complete, independently validated casting/combat/movement/normal-Quit
inputs and verifies their recorded hashes. A cadence-failed diagnostic capture may
be replayed only if those independent gameplay and capture checks passed; its
original failure is retained. Every captured World is replayed in order through
source decode, ownedV5 handoff and resident consumption. There is no synthetic
continuation or original simulation/pacing. Timestamp mode times each resident
batch; throughput mode disables frame timestamps. Both use a terminal empty
marker and bounded polling to include GPU completion in total replay throughput.
All-frame throughput includes disk reads and cold setup. One native-word readback
and SHA-256 comparison occur after the completion timer. Repeated/mode output
hashes must agree; ordinary World delivery must retain zero readbacks.

The offline consumer omits public command import, UI and Qt paint. Its throughput
is a render-path measurement, not public gameplay FPS. Actual timings and CPU
hotspots are pending fresh execution; no 60 FPS achievement is claimed here.

## Provenance and remaining boundaries

The incoming merged-master first-parent range is reviewed separately in
`coverage/resident-profile-base-history-20261011.json`; all 360 affected exact file
versions have retained receipts, with no unresolved gap. Historical renderer and
public resident evidence retain their hashes and may become stale for shared
source edits. New tests do not refresh those older execution records.

Pending: query/context controls on real hardware, fresh public pacing and CPU
samples after the minimap fix, a complete ordered gameplay corpus, fresh offline
hardware throughput, and attribution of presentation/unobserved original work.

## Instrumentation milestone, 2026-10-11

`resident-profile-instrumentation-20261011.json` records a fresh NVIDIA TITAN RTX
EGL fixture: 64 ordered timestamp pairs, one overflow batch still executed, four
identity/exception/nesting refusals, deletion of pending queries on destruction,
1024 unchanged native words and no ordinary readback. Six malformed corpus cases
(wrong magic, short header, out-of-order generation, source/request extent and
short payload) refuse without publishing a success report. Profiling tools and
the public native shell build successfully. These are synthetic controls; full
public shared-context observation and representative throughput remain pending.

## Diagnostic failure and bounded capture repair

The first raw synchronous capture probe is preserved as
`resident-profile-synchronous-capture-negative-20261011.json`. Its smoke reported
native fallback after multi-second host poll gaps; a retained individual capture
write took946.324452 ms while the corresponding native consume took21.931336 ms.
Other command/poll stalls remain unattributed. Sampling was terminated and the
wrapper interrupted campaign cleanup; the owned display/Wine session was then
independently stopped and original media reverified. This is a failed diagnostic,
not a rendering throughput result or completed gameplay corpus.

The replacement capture service copies immutable source/request bytes into a
256MiB raw-byte queue. A worker owns compression and file writes, with no widget,
GL context, original pointer or channel mapping access. Queue/file/generation
failures make the capture incomplete while gameplay continues. Retirement drains
the worker after native gameplay has ended. The compressed V2 corpus is normalized
exactly once into bounded V1 outside all benchmark timing. This keeps compression
and expansion work separate from measured render-path throughput. CPU sampling
runs in its own signal group; sampler failure is recorded before awaiting normal
campaign cleanup, preserving its report.

Public clock ownership previously left all pacing masks unobserved. The guarded
render IAT owner now optionally calls `ScenePacerObservationV1(uint32 caller,
uint32 tick)` with stdcall32 at return PCs0x4e3f59/0x4e3f97 only. Admission checks
the existing import/load/wait loop plus both original call-site byte anchors and
the loaded versioned scene export. The callback only updates process-owned
integers; it does not call a clock, API, host or renderer. Its caller preserves
LastError. The scene diagnostic clock uses the same configured real WinMM uptime
when appropriate, without installing a second IAT hook. Missing/partial masks
remain unobserved; no universal20Hz simulation policy is inferred. Fresh source
acquire timings also separate channel copying/hash validation from decode/handoff.
New capture, callback and complete public performance execution remain pending.

`resident-profile-worker-fixtures-20261011.json` proves four byte-exact Qt/Python
compressed records, five queue/generation/extent/NewOnly controls and11 malformed
compressed inputs (including hostile expansion lengths and trailing streams).
Clock forwarding passes33 mutated byte guards and exact original clock/LastError
values with two paired callbacks and no extra clock calls. Current shared import
passes65,536 words, two HUD/copy comparisons and zero ordinary readbacks; native
Quit acknowledges/retirement/refusal/idempotence cases pass. Full original public
callback activation, fresh gameplay CPU stacks and throughput remain pending.
