# Reusable command-byte ring, version2

2026-10-06. This native policy is an additive transport foundation, not an
original binary structure or a live hook replacement. Schema:
`protocols/schemas/render_commands-v2.json`; generated C/Python constants;
shared C/C++ primitive `protocols/include/mnm/render_command_ring.h`.
The existing append-only v1 wire and ordinary producer remain active.

## Mapping and ownership

The exclusively created mapping is1,048,640 bytes:64-byte header plus1MiB data.
Magic `MNMRDC02`, version2. Integers are aligned unsigned32 little-endian values.
Only verified GNU-style x86 atomic32 hosts are supported. No host pointers or
C/C++ objects cross the wire. The creator assigns a nonzero session and zeroes
all counters, state, reason, cancel and reserved bytes before peers attach.
The caller supplies a valid mapping of exactly the declared size.

| Offset | Field | Writer |
| --- | --- | --- |
| 0 | Magic8 bytes | Creator |
| 8 | Version | Creator |
| 12 | Mapping size | Creator |
| 16 | Session | Creator |
| 20 | Published total bytes | Producer |
| 24 | READY0 / WRITING1 / ENDED2 / FAILED3 | Producer |
| 28 | Terminal reason (same enum values as v1) | Producer |
| 32 | Cancellation0/1 | Consumer |
| 36 | Acknowledged total bytes | Consumer |
| 40 | Reserved24 bytes, all zero | Creator |
| 64 | Circular payload1MiB | Producer |

One producer claims READY using atomic compare/exchange. The owning application
must arrange exactly one reader; the header does not authenticate a reader PID.
The reader may attach before publication, or drain a completed sequence whose
bytes are still buffered. Reattaching after acknowledgement is not supported.

Published and acknowledged are monotonic total counts, not modular positions.
Physical offset is total modulo1MiB. Producer occupancy is published minus
acknowledged, never exceeding capacity. Release publication follows copying all
bytes. The reader acquires publication, copies at most64KiB into caller-owned
storage, then release-publishes acknowledgement. It never returns a borrowed
ring pointer. ACK means the private copy owns bytes; it does not mean commands
have executed or reached the GPU. Decoded queues and GPU scheduling require
separate admission limits.

## Backpressure and terminal behavior

Write returns WRITTEN1, FULL0, or REJECTED-1. FULL writes no bytes, changes no
counter/state, and never waits. The caller retains the exact input and retries
outside an original-engine ownership guard. Inputs larger than capacity must be
fragmented; command records may cross physical boundaries and multiple writes.
Read returns copied count, zero while empty, or -1 on refusal. Reads acknowledge
only private copies; unread ring data cannot be recycled. Failure is sticky for
the reader and requests cancellation when identity remains valid.

END is published after the last bytes; the reader continues draining before
completing the command decoder. Terminal state or final publication changing,
regressed counters, acknowledgements outside producer history, occupancy beyond
capacity, identity/reserved corruption and invalid budgets refuse. Cancellation
causes the next writer call to publish FAILED/CANCELLED. Identity changes do not
write into the rebound mapping. Counter addition beyond UINT32_MAX refuses
OVERFLOW; counters never wrap. This is reusable bounded storage with a finite
32-bit session lifetime, not an unlimited lifetime or lost-command recovery.

The v1 `MNMCMD01` command envelope remains unchanged. Its decoder still limits a
session to64MiB/4096 records. Removing those whole-session limits requires a
separate lifecycle/accounting design; transport reuse alone does not remove them.
No live hook adapter, owned queue, retry scheduler, launcher negotiation, consumer
restart, crash recovery or sustained original-game rendering is introduced here.

## Validation

```sh
cmake -S renderer -B working/build/render-ring -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/render-ring --target render-command-ring-gpu-test
xvfb-run -a python3 tools/test-render-command-ring.py working/build/render-ring
```

The C fixture tests full-ring refusal without overwrite, acknowledgement/reuse,
fragmentation/wrap, final drain, cancellation, forged/regressed counters,
identity/reserved corruption, sticky reader refusal, terminal changes and
counter saturation. Independent forked processes transfer70MiB using1MiB mapped
storage against an independently generated expected byte stream. Native and
ASan/UBSan builds run the same fixture.

The Qt/OpenGL fixture fragments a1MiB+CREATE header command across the ring and
reuses it for four changing primary frames. It feeds the existing production
CommandDecoder and CommandConsumer, compares every displayed pixel against
independent expected colors, and checks native cleanup and zero ordinary native/
RGBA readbacks or viewport image uploads. This bounded decoder scenario is
separate from the70MiB raw transport test and does not assert unlimited decoding.

[Recorded execution](../runtime/opengl-command-ring.json) passes16 cases in both
native and ASan/UBSan builds, each transferring73,400,320 bytes across independent
processes. The GPU scenario passes four complete changing display frames,
4,194,548 command bytes, explicit FULL/retry cycles and zero ordinary native/RGBA
readbacks or viewport uploads. Independent display diagnostics are explicit,
and terminal native surfaces are released. Existing incremental-consumer and
v1 live-channel CTests and all protocol contract tests pass. Initial GPU fixture
failure was letterboxing caused by the widget default minimum size; the final
fixture explicitly uses128x64 at the native2:1 aspect ratio. The copy-reuse test
also bounds each read to its remaining expected region before checking distinct
replacement bytes. Neither correction changes production drawing behavior.

Next integration boundary: a Qt mapped-file adapter for v2, producer-owned
bounded queue with retries outside the engine tracker, shutdown/cancellation and
launcher negotiation. Preserve an exact session archive separately; decoder
whole-session limits and longer original-game coverage remain independent work.
