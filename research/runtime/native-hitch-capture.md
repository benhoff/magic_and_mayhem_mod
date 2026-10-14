# Bounded native rendering hitch capture

Native diagnostic policy, opt-in through `MNM_NATIVE_HITCH_CAPTURE=1` and
`test-native-campaign.py --frame-trace --hitch-capture`. Preserve up to eight
largest eligible warm active completed-paint gaps of50ms or more, with100ms
before the preceding paint and250ms after the trigger. Each window has at most
2048 owner polls and512 GUI events. Preallocated histories and window storage
perform no live JSON serialization or pixel reads; clipped/missing post windows
are reported. Completed windows can be replaced by stronger gaps. Hidden,
movie-paused and non-World completed paints reset continuity; generation64+
excludes cold startup. Largest-window retention and overlapping windows are
observations, not an exhaustive or independent hitch count.

Trace schema2 exports its native steady-clock origin. Campaign rate windows
include separate host_begin_us/host_end_us fields from that same clock; existing
Qt rates and original uint32 timestamps retain their independent meanings. Owner
rows add UI/seal/source generations, source and GPU delivery counters and export
lease refusal. Original FIFO, two credits, engine pacing, GPU leases and native
pixels remain unchanged. Ordinary trace/production modes do not enable capture.

Source-bound synthetic/TITAN controls and a complete traced gameplay reproduction
are pending. Timing includes driver waits, not GPU busy time; completed paint
is not scanout. Correlation identifies observed gates, not causal root cause
without a targeted experiment. Native rendering fixes remain a separate milestone.

## Controlled and interrupted live findings

TITAN `run-0cgd9vjo` passes the source-bound capture controls and128 exact
World comparisons, generic queue/worker/retirement/presentation checks with
stable source/inputs/original manifests. History review `ee3a1e42..ea97c742`
checks29 exact committed transitions with zero missing receipts.

Storage guard stops the outer `run-4aowab2c` below100GiB. The native host
independently completes5154 balanced Worlds/returns at19.01482engine and
55.06962native FPS, preserving27 observed warm hitches/eight strongest windows.
Runner final casting/source/assets checks were not executed: retain interruption
without promoting successful live integration. Manual original verification and
private Wine/X cleanup pass. Free space later recovers to127GiB; reviewed
archival plans remain unexecuted and no files are pruned.

The strongest1008.957ms gap preserves898 owner polls,869 with reason2
(pending World import), old sealed World/UI2435 and completed/pending2436;
GPU2437 waits with two source credits held. There is one native frame/export
and no export-lease refusal. Owner World max0.043ms, commands21.55ms, UI
0.002ms, export0.186ms, status0.585ms and waits1.151ms do not explain a
one-second CPU block. The native pending-import/generation guard intentionally
refuses redraw of prior imported2435 after completion2436. This gate explains
the observed display suppression; the cause of delayed original command
publication is still unknown. Other retained393..779ms gaps show the same gate.

Next native policy: keep the last imported immutable World and matching sealed
UI displayable while the next completed generation waits for command import.
Preserve FIFO, two source credits/two GPU slots, original verified returns and
strict UI/World generation matching; do not advance simulation or import early.
Prove pixels/counts/leases in controls and rerun the full public journey.
