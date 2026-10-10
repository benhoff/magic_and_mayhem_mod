# Ordered native World delivery without gameplay

`WorldRollingChannel` implements a separate two-slot source/completion channel
with ordered64-bit sequencing, backpressure, terminal cancellation and fresh
session identity. `WorldRollingSession` keeps producer canvases, decoded sprite
resources, visual/owned-frame identity caches, GPU atlas and retained canvas
across complete queue packets. It independently asserts CPU/GPU pixels before
publishing a completion. A failed packet poisons the session; recovery requires
a new session, rather than resetting a queue ordinal over retained history.
The [wire contract](../formats/native-world-rolling-v1.md) records its bounds
and independent original/live integration gaps.

`tools/test-native-world-rolling.py --corpus <successful-resource-reuse-run>`
freezes its declared compiler closure and builds an optimized consumer. A
synthetic producer sends the saved32-queue source prefix, then repeats the last
queue's requests over the same evolving retained canvas with one moved indexed copy request every64th queue, without replaying
startup or importing saved output pixels. Default512 queues therefore include
480 explicitly synthetic continuation queues. The saved prefix must match all
1224 checkpoint identities/digests and every World completion must match native
CPU composition. Replies1..32 also match their saved final digests at the
producer. Every queue must be acknowledged in order, including those after32.
Producer/consumer delays force full-ring backpressure. No upload, visual recheck
or raw identity hash is allowed for unchanged continuation resources. All
producer/identity/atlas/surface bounds and teardown are checked.

Before execution, continuation delivery completion median60/p95120/max300ms,
cold delivery completion500ms and post-queue64 resident growth32MiB are declared.
Delivery completion includes the owned channel copy/checksum, consumer delay, packet decoding, native
producer operations, GPU completion and serialized reply, measured from consumer polling to publication. Saved checkpoint assertion copies/hashes are timed separately and excluded from the budget; full verification time remains reported. Native work, initialization and memory
samples are separately reported. The warm budget applies after32, where saved
checkpoint hashing is absent; the saved prefix's hash assertion cost remains
visible in per-queue delivery times. These are host-specific offline limits,
not an original simulation FPS or physical presentation claim. Memory samples
check retained growth, not peak allocation.

An assertion-enabled cross-process fixture cycles1000 ordered inputs/replies
through separately mapped slots, mutates caller storage, forces backpressure
and exercises source/reply corruption, ownership, role/session reuse,
cancellation and incomplete publication. Four existing Debug producer, raster, handoff
and atlas fixtures remain required. Packet envelope, duplicate/incomplete queue
and terminal renderer refusals run with optimized checks that remain active in
release builds. `--gl33` disables copy-image for fallback validation; a prepared
build is admitted only with matching source, binary and compiler fingerprints.

This does not implement or claim original live producer replacement. Translating
the original queue source/lifetime/return guards into this native ordered
contract, supervising actual producer death/restart, camera/normal-mode/combat
cadence, input/desktop latency and hardware execution remain pending. Repeating
a saved queue is intentionally synthetic and cannot recover uncaptured gameplay.

The adapter library builds independently under `compat/legacy/world-rolling`. One exact-byte owned packet decode cache retains at most16MiB and the declared V3 record/source limits. Changed requests invalidate it; unchanged requests still execute against evolving native canvas history. Packet-local exact source definitions deduplicate only within one queue; they never become an indefinite journal. Native producer copy sampling is clipped with64-bit offsets, fully preflighted before mutation, snapshots only same-ID sources and bulk-copies opaque rows.300 seeded scalar copy cases cover alias/key/clipping/extreme offsets; undefined source, keyed undefined, atomic failure and partially defined alias cases remain explicit.

The [default frozen result](native-world-rolling-default-20261010.json) passes512 queues:1224 saved checkpoint assertions,512 independently checked native CPU/GPU completions/ordered acknowledgements,480 synthetic continuation queues including eight changed-coordinate requests,300 scalar copy cases,16 optimized packet/terminal refusals and five Debug fixtures. The process fixture cycles1000 inputs/replies, exercises27 refusal cases, actual producer exit, terminal cancellation and fresh-session admission. Continuation completion is54.79ms median/74.88ms p95/102.49ms maximum; cold full bootstrap/World packet is298.54ms excluding1982.52ms saved hash assertion work (full verified completion2281.06ms). Post-queue64 retained growth is0.19MiB and all resource/packet caches stay bounded. Every completion and lifecycle check passes; unchanged continuation draws cause no upload/visual recheck/raw-frame rehash.

The [GL3.3 fallback](native-world-rolling-gl33-20261010.json) repeats512 queues on the exact same verified binaries with copy-image disabled:57.14ms median/74.83ms p95/88.50ms maximum,269.73ms first completion and0.19MiB retained growth; all pixel/ownership/Debug checks pass. The [impossible budget result](native-world-rolling-budget-refusal-20261010.json) completes64 queues and1224 checkpoint assertions with correct pixels, but records performance failure and exits1 for0.001ms limits. It retains source/input/frozen/manifest verification. It is proof of budget refusal, not a throughput pass.

[Exploratory history](native-world-rolling-experiment-history-20261010.json) retains the573/544/520ms bootstrap trials above the unchanged500ms limit and the60.10ms continuation median above60ms. Raw development probes have no frozen compiler closure and do not promote current validation. Packet-local sources reduce first packet size from13.61MB to3.11MB; the continuation packet is3.63MB. Latest exact-byte decode reuse remains bounded; changing a request every64th queue forces invalidation. The decisive bootstrap improvement comes from eliminating unnecessary source snapshots and copying fully preflighted row spans: native startup/HUD CPU work decreases from roughly377ms to138ms in the probes. No budget is relaxed.

Committed c9f99e8..ae448ba accounting review covers15 exact file transitions with matching receipts and no gaps. Older shared native/World/producer/live evidence remains historical after the rolling-mode and row-copy source changes; no old source hash is refreshed. The [fresh final-source cold replay](native-world-prepared-admission-rolling-source-20261010.json) adds four sessions/128 CPU/GPU completions/4896 saved checkpoint assertions and three Debug fixtures: cold116.67ms median/134.05ms maximum, warm37.80ms median/47.44ms p95/53.94ms maximum and4MiB growth pass unchanged budgets. It registers a new evidence ID; the earlier cold report stays historical. The live producer still needs request/reply identity and extent checks, original lifetime/return/AX guards and ABI validation. Generic channel extent bounds alone do not establish that integration.
