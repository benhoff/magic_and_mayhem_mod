# Wrapping command-byte ring, version 3

Native transport policy, reviewed 2026-10-16. `MNMRDC03`/version 3 retains V2's
64-byte header, 1 MiB payload, aligned little-endian atomic32 fields, session
identity, states, reasons, cancellation and ACK ownership. The schema is
`protocols/schemas/render_commands-v3.json`; generated bindings are checked in.
V1 remains bounded; explicitly selected V2 retains its nonwrapping lifetime guard.
Continuous Qt launches now create V3, and staging accepts either ring version.

Published and acknowledged are positions modulo 2^32. The writer owns one local
published position and its last observed ACK. Let outstanding be unsigned
`published - acknowledged`. It must be at most capacity. A new ACK is valid only
when unsigned `new_ack - acknowledged <= outstanding`. This accepts progress
across zero and refuses backward or ahead-of-publication ACKs. The reader likewise
bounds unsigned publication advance by remaining capacity after its previously
observed pending bytes. Its own ACK field must still equal its local consumed
position. Neither peer can lap the other: publication stops after 1 MiB without
ACK progress. The power-of-two capacity divides 2^32, so physical offsets remain
continuous at rollover. Always-lock-free 32-bit atomics remain required; no torn
64-bit IPC loads, process-local locks or epoch reset handshake are introduced.

Both peers pin the negotiated wire version as well as session identity. A
same-session V2/V3 header substitution refuses and cannot cancel/fail the rebound
mapping. Creator identity/reserved fields remain immutable. One producer and one
reader are application ownership requirements; a second reader or reattachment
to a spent channel is unsupported. The ring does not authenticate malicious peers
or prove that externally forged bytes are meaningful commands.

The 32 MiB private queue uses the same bounded unsigned distance, with a
power-of-two capacity. Split appends and physical wrap preserve byte order and
owned-copy publication. Capacity exhaustion, sticky failure, 8/4 MiB pressure
pacing, timeout, cancellation, END and shutdown ACK requirements remain. Shutdown
validates ACK progress after END using the same bounded rule. The owned emitter likewise counts cumulative bytes only for bounded files,
retaining sequence cleanup and diagnostic archive budgets. The streaming
decoder now counts total bytes only in bounded-file mode; streaming limits
retained bytes and individual records rather than lifetime byte totals.

This removes the four 4 GiB byte ceilings. It does not remove finite command
sequence/surface identity domains, single/nested-callback capacity, no-progress
deadlines, unsupported drawing/history admission or the requirement for a fresh
producer after resident failure. Original simulation behavior is unchanged.


[Final source-bound execution](../runtime/render-command-lifetime-controls-06-20261016.json)
passes actual emitter/queue/ring traffic of 4,297,064,448 bytes and Qt decoder/
consumer traffic of 4,295,064,708 bytes, plus native/sanitized and cross-process
controls. [Runtime evidence and reproduction](../runtime/native-command-byte-lifetime.md)
keep PE32 integration, original-game limitations and finite sequence policy separate.
