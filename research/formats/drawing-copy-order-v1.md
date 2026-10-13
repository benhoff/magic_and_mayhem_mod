# Original-active copy request journal V1

`MNM_DRAW_COPY_ORDER=1` requires original-active producer trace mode. The
optional `copy-order-0000.bin` sidecar keeps a local request ID allocated at
copy entry and the emitted producer record sequence at copy return. It records
producer observation order; driver execution and API/worker request binding are
separate evidence. No original surface bytes or process pointers are serialized.

The 64-byte little-endian header contains `MNMDCQ01`, version1, row size128,
build marker40209ca7, record limit65536and ten zero DWORDs. The limit includes
the terminal; write/record exhaustion refuses. Historical producer stream
formats remain separate from this new journal.

| Row DWORDs | Meaning |
| --- | --- |
| 0–7 | Ordinal, phase1entry/2return/3terminal, request ID, local owner thread, pinned producer hook tag, tick, producer prefix, queue |
| 8–12 | Destination/source logical canvas IDs, destination width/height/stride |
| 13–19 | Destination x/y, source left/top/right/bottom, raw key (ffffffffopaque) |
| 20–21 | Original caller RVA when in pinned code range; emitted producer return sequence (zero at entry) |
| 22–23 | Reserved zero |
| 24 | Active requests after this event |
| 25–31 | Reserved zero |

The terminal request field is the total request count; descriptor/owner/tag/tick/
return fields are zero. Producer prefix and queue identify the bounded end. A
nonzero active count or missing terminal remains incomplete. Entries are unique
and monotonic; paired returns must preserve owner, tag, source/destination,
geometry/key and caller, with a later emitted producer sequence. Return order
may reverse entry order. Strict decoding does not grant driver ordering, API
binding, source ownership or native parity. Raw coordinates retain original
unsigned bits and are not reinterpreted as a new clipping policy.

Recorder calls occur under the existing private producer trace guard, which
ends before returning to original code. The helper preserves LastError and
reads the portable copy request without modifying it. Original callback
forwarding/FX preservation still belongs to the existing observer boundary;
the owned journal fixture alone cannot validate every original hook effect.

Four decoder guards and the full scene build pass. The retained owned PE32
five-case fixture passes actual two-thread reversed returns, disabled/invalid
configuration, incomplete terminal and row-budget refusal, with LastError and
request preservation. Original32 capture retains913paired requests and actual
game exit0. Exact producer cross-check links all913COPYrecords, with0unmatched
and45adjacent entry pairs reversing return order. These are separate retained
results; request/API/worker binding and source-only ordering repair remain open.
