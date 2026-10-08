# World channel v1

Native policy, not a recovered original protocol. The fixed little-endian file
mapping is 83,886,240 bytes: a 128-byte header and two 41,943,056-byte slots.
Both implementations require aligned 32-bit atomic ownership words. Original
addresses and C++ layouts never cross the channel.

Header offsets are defined in `protocols/include/mnm/world_channel_v1.h`.
Magic `MNMWCH01`, version 1, exact size, nonzero session identity, build token,
capacities, diagnostic flag and reserved zero words remain immutable. State is
WAITING, ACTIVE, ENDED or FAILED; cancellation is a separate host-owned flag.
Published, consumed and presented sequences are distinct diagnostics. The
producer owns its dropped counter; the host owns its superseded count locally.

Each slot contains state, sequence, input size and oracle size in its first
16 bytes. The next 32 MiB contain one closed [World frame](world-frame-v1.md).
An optional separate 8 MiB area contains tightly packed original RGB565 output
only when the immutable verification flag is set. It is never a raster input.
Ordinary mode writes no original output and performs no native pixel readback.

Ownership is FREE → WRITING → READY → READING → FREE, with acquire/release
publication and compare/exchange acquisition. A producer cannot overwrite READY
or READING. With no FREE slot it increments its dropped count and continues
original drawing. The consumer copies the newest READY packet into host-owned
bytes and can release an older READY packet as superseded. WRITING is invisible.
The finite closed packet is decoded and all native resources are resolved before
presentation. Invalid identity, ownership, extents, sequence, unknown native
operations or diagnostic mismatches cancel/refuse the session. No partial frame
is presented.

Cancellation at an original queue boundary publishes ENDED. Verification mode
publishes FAILED for every capture failure. Normal shadow mode treats capture
reasons 7 (uninitialized/unsupported wave) and 8 (unknown queue kind) as whole-frame
capability refusals: ACTIVE is retained and a READY packet contains only the
80-byte World header, zero raster count and zero oracle bytes. Partial captured
draws are excluded from the published extent. The host validates the closed
refusal envelope, clears its native viewport, shows a waiting/unsupported status
and can resume on a later complete packet. All other capture failures publish
FAILED. A malformed refusal or any refusal in verification mode is fatal.
The layout and version remain unchanged; older consumers fail closed on nonzero
World failure codes.

Refusal counters describe packets consumed by the host. Dropped/superseded
packets can hide other producer refusals. This policy does not render unsupported
draws or establish complete canvas history. An original process killed without DLL detach
need not publish ENDED: launcher process supervision handles that exit; it does
not claim a completed producer shutdown. This shadow channel never suppresses
the original consumer or its raster work. Sequences do not wrap; overflow fails.
