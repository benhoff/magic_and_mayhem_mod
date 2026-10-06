# Native surface command stream v2

Native producer/consumer policy, distinct from the mapped render-commands v2 ring.
All integers are little-endian u32. Header: `MNMCMD02`, version 2, size 16.
Each record has opcode, contiguous sequence starting at 1, payload byte length.
Existing surface operations retain their v1 layouts, except opcode 4 is refused.

| Opcode | Payload |
| --- | --- |
| 12 CREATE_PALETTE | palette ID, immutable lifetime generation, 256 RGB triples |
| 13 UPDATE_PALETTE | palette ID, generation, first entry, count, count RGB triples |
| 14 BIND_PALETTE | surface ID, palette ID, generation |
| 15 DELETE_PALETTE | palette ID, generation |

Palette IDs occupy an independent namespace, qualified by the enclosing fresh
ring session. IDs are nonzero, strictly increasing for each creation, never
reused. Generations are nonzero; updates, bindings and deletion require the exact
live generation. Binding 0/0 detaches. Only indexed surfaces accept bindings;
indexed PRESENT requires a live binding. Deletion requires all bindings removed.
END requires no live surfaces or palettes and at least one PRESENT. At most 32
palettes of 256 RGB entries coexist. RGB updates fan out to all bound surfaces;
entry flags do not affect color and do not generate redundant updates.

The decoder accepts v1 and v2, but never mixes versions in a session. Ring
transport framing, cancellation, byte budgets and freshness checks are unchanged.
The schema owns constants; these semantic conditions are implemented in both
producer and consumer. No host pointer crosses the wire.
