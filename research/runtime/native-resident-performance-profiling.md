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
python3 tools/profile-native-resident.py --cpu-sample
python3 tools/profile-native-resident.py --capture
python3 tools/benchmark-native-resident.py --campaign <campaign-evidence-directory>
```

The first probe uses a private accelerated TITAN display and the existing physical
native campaign casting/combat, Fireball/refusal, movement, portrait, Mini Cancel
and normal Quit journey. It preserves the 19 FPS gates. Optional 99 Hz user-space
CPU stacks sample 30 seconds after 40 seconds of physical gameplay. Run CPU sampling and corpus capture separately; serialize hardware probes and keep coverage builds/symbolization outside gameplay. Symbolization is now deferred until campaign cleanup. Capture enqueue/compression work,
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
is a render-path measurement, not public gameplay FPS. Fresh timing and CPU hotspot results appear below. Offline completed average
throughput exceeds60FPS for an explicit verified combat prefix;60FPS gameplay
and a steady frame floor remain unproven.

## Provenance and remaining boundaries

The incoming merged-master first-parent range is reviewed separately in
`coverage/resident-profile-base-history-20261011.json`; all 360 affected exact file
versions have retained receipts, with no unresolved gap. Historical renderer and
public resident evidence retain their hashes and may become stale for shared
source edits. New tests do not refresh those older execution records.

Current results prove bounded query/context/worker controls, fresh public CPU/GPU
and paired original pacing observations, and repeated actual combat-prefix
hardware throughput. A complete validated capture journey, causal attribution
of public presentation/original work and steady60FPS remain pending.

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

## Fresh CPU/GPU/original pacing observation

`resident-profile-cpu-pacing-20261011.json` preserves a complete physical journey:
casting, Fireball, combat, movement, portraits, Mini Cancel and normal Quit,
5421 balanced completed/imported/verified-return Worlds, zero ordinary native,
World or RGBA readbacks and80-byte metadata ACKs. Both original pacer call sites
are observed in all5358 retained warm rows, with no dropped rows/queries and no
pending queries at retirement. The overall19FPS cadence gate **failed**; UI
frame/paint counters include repeated updates and are not distinct World FPS.
The CPU sample and symbolization overlap perturb this run. These timings are
observations, not a new uninstrumented performance guarantee.

| Warm World interval | Mean ms | Median ms | p95 ms |
| --- | ---: | ---: | ---: |
| Source acquire + decode + handoff + native consume |25.420|24.083|38.057|
| Native consume, including following nested stages |21.732|20.424|33.657|
| Native CPU producer (nested) |8.947|9.280|11.858|
| Native prepare (nested) |1.801|1.713|2.608|
| Native submit CPU (nested) |8.740|7.255|17.992|
| Renderer-context GPU timeline |9.707|9.013|20.070|
| Preceding observed original pacer |9.388|10.000|18.000|
| Unclassified between-World residual |9.869|8.000|14.000|

The current host World path alone exceeds16.667ms. It requires at least a1.53x
reduction in mean latency to fit that budget, before public UI/paint/scanout and
remaining original work. The GPU timeline is shorter than host consumption on
average, but its20.070ms p95 and CPU submission gaps prevent concluding that GPU
work meets60FPS in every frame. Original intervals remain near the unchanged
50ms target; making replay fast cannot itself produce60 distinct gameplay Worlds.

The30-second99Hz native user-thread sample reports no lost samples. Largest
resolved flat symbols are `CanvasSequence::minimap`15.88%, raster validation3.26%,
sort3.14%, channel polling2.17%, producer append1.63%, producerWorldDraw1.63%,
minimap terrain1.51% and masked upload1.45%. Driver offsets remain unresolved.
Sentinel planes and candidate sorting in minimap preparation are concrete CPU
optimization candidates, followed by validation/packing and GL submission; these
are priorities, not measured speedups. Do not add sample shares to stage times.

`resident-profile-compressed-capture-negative-20261011.json` preserves a second
failed capture:1857 completed Worlds and1858 written compressed records before a
native drawing-history GAP. Concurrent coverage work and long poll/command stalls
are recorded; their cause is unproven. It is not admitted as a complete replay
corpus. A clean serialized capture and representative offline replay are pending.


The next clean serialized Apprentice capture is retained as
`resident-profile-wizard-death-negative-20261011.json`:2176 last-reported balanced
Worlds, no native delivery error,19.270 observed World FPS, but player wizard
death before required combat completion. Owned Wine stopped and original media
verification passed. Final capture hash/size is recorded after teardown; it is
larger than the last in-game worker snapshot because teardown drained later
queued records. Neither this truncated physical journey nor the stale worker
snapshot is promoted to a complete corpus. An Initiate retry was rejected by the casting/combat harness before launch: this
journey requires Apprentice or harder. Its invocation error remains under
working/tests/resident-stage-profile/run-_7agvhtm. Subsequent attempts retain
the original Apprentice selector and unchanged combat admission gates.


## Explicit failed-journey prefix measurement

`resident-profile-serialized-gap-negative-20261011.json` preserves the next clean
Apprentice failure: Fireball damage and a player-party lethal hit appear in the
physical input record, but the native command producer later declares drawing
history GAP.2423 Worlds have matching completed/imported/verified-return counts;
one pending2424th World is excluded from diagnostic replay. This proves that
concurrent coverage was not necessary for this GAP; it does not identify a cause.
The complete spell/portrait/Mini/Quit journey remains failed.

The default benchmark admission still requires all independent full-journey and
complete-capture checks. Its explicit `--completed-prefix` mode instead requires
64..8192 contiguous generations with recorded original returns, balanced imports
and completions, stable source/owned cleanup/original verification, zero ordinary
readbacks/pixel replies and zero dropped/truncated diagnostics. Selected records
must already be written. Final capture size/hash and launch-owned input
fingerprints are checked; the entire compressed file, including its excluded
tail, is structurally validated; unselected rendering semantics are not inferred. The decoded count must match the accepted worker snapshot.
Normalization copies only the verified prefix, so no pending World enters the
measured workload. Failed campaign and missing full-journey checks are explicit
in the result; this mode cannot establish native menu/Quit integration or60FPS
gameplay. It supplies a bounded actual combat-rendering workload, with its
original failure retained, rather than synthetic repeated terrain.

Growing diagnostic JSON report publication every64 Worlds is outside stage
timings and can perturb live cadence. Its cost and the recurring drawing-history
GAP remain open. The offline consumer does not perform those publications.


`resident-profile-prefix-controls-20261011.json` reruns current worker/archive,
import/Quit and guarded clock controls. Two selected Qt/Python records match
exactly; eighteen new prefix admission/bounds/order/return/readback/drop and bad
excluded-tail controls refuse. The eleven existing compressed-input negatives
also pass. The default full-journey gate is unchanged.


## Completed actual combat-prefix replay and matched compiler settings

The fresh immutable records are `resident-profile-offline-o3-20261011.json`,
`resident-profile-offline-o2-20261011.json` and
`resident-profile-matched-input-comparison-20261011.json`. All2423 selected Worlds
are decoded, prepared and consumed in order; the complete2424-record compressed
file is structurally validated, and its one pending tail World is excluded.
97 compiled C/C++/header input hashes match the actual public capture. Both probes
freeze sources, verify all launch-owned static inputs, bracket original manifest
verification, refuse software/non-TITAN/desktop fallback and leave zero queries
or surfaces after session teardown. All six runs have the same terminal native
word SHA-256. Each performs one diagnostic readback after the timer, with zero
ordinary delivery/World readbacks. These are repeated terminal-output checks,
not per-frame original pixel comparison.

| Same verified input prefix | Public O2/Zink/Qt | Headless O2/NVIDIA EGL | Headless O3/NVIDIA EGL |
| --- | ---: | ---: | ---: |
| Warm decode + handoff + native mean ms |26.384|11.602|10.073|
| Native producer mean ms (nested) |8.647|3.906|2.699|
| Native submit CPU mean ms (nested) |10.358|4.766|4.538|
| GPU timeline mean ms |11.386|7.542|7.172|
| Warm pipeline p95 ms |43.772|19.874|17.621|
| Completed timestamp-disabled replay FPS |not applicable|78.289 /71.963|88.070 /85.655|

All-frame completion throughput includes fresh renderer/session initialization,
disk input, decode, handoff, native CPU and terminal GPU drain. The timestamps
mode completes at77.705FPS with O2 and85.889FPS with O3. Public production uses
RelWithDebInfo (`-O2 -g -DNDEBUG`); frozen O2 deliberately matches optimization,
while O3 provides a separate optimized reference. The headless driver is
`NVIDIA TITAN RTX/PCIe/SSE2`; public hardware is Zink over NVIDIA Vulkan with Qt
shared-context presentation and concurrent original simulation. Filesystem and
driver caches are not purged. Different host workloads and trial variation are
retained. Compiler optimization improves this isolated workload, but cannot
explain the entire public gap; no causal backend/context speedup is inferred.

This shows that the resident rendering path can exceed60 completed Worlds/second
on average for this actual combat prefix. Both pipeline p95 values still exceed
16.667ms, so a steady60FPS floor is not established. Public painting, UI import,
original work and its selected50ms pacing target are outside replay. Full live
capture/normal Quit still failed. No60FPS gameplay, crowded-scene throughput,
original equivalence, simulation policy change or DirectDraw restoration is
claimed.

The next measurement should run identical inputs under the public backend and
context-sharing route, separate UI import/paint and report publication, and
identify the repeated drawing-history GAP source/transition. Use this to choose
changes before treating minimap CPU work or GL submission as the sole bottleneck.
The original pacer needs its own simulation/presentation contract before changing
cadence. This batch adds measurement, not a production optimization or game-speed
change.
