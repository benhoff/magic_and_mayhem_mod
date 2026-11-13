# Owned World source chunks (V7)

V7 is an intentional native transport policy combining the owned World snapshot
receipts of [V5](native-world-snapshot.md) with [V6 source-only chunks](world-continuous-v6.md).
The 128-byte channel header and two fixed slots retain their sizes and offsets.
The header magic is `MNMCON07`, version 7 and mode 4. Prior peers reject it.
World replies remain exactly the V5 `MNMSACK1` descriptor/nonce/generation-bound
80-byte receipts; these acknowledge validated owned source, not GPU completion.
The producer still verifies the actual original return before native slot reuse.

Source-only slots use V6 states 6/7/8 and global nonzero u64 chunk ordinals in
slot words 22–23. They keep the current World generation and contain source
bytes/CRC only: no World request, reply or actual return. Thresholds remain
16,384 records or 4 MiB before the next emission, outside an open World queue.
Existing single-operation, whole-World and deferred-overlap bounds remain.

One capture worker validates both kinds of item. It can queue one source-only
item beside at most one unimported World, then waits. The native render owner
applies that source history through the same producer instance only after all
prior Worlds have been imported. It signals successful application by the exact
chunk ordinal; only then does the capture worker verify immutable slot bytes and
publish the source ACK. No subsequent item can pass that handshake. Source ACKs
never spend or return World credits, increase World generations or complete a
World. Source failures/cancellation stop the worker without acknowledging reuse.
Unknown pixels and malformed World/failure records in a chunk still refuse.

The public independent, non-asynchronous path selects V7. Explicit asynchronous
GPU delivery retains V5. Finite profile capture retains its earlier envelope
unless `MNM_RESIDENT_CAPTURE_SOURCE_CHUNKS=1` is selected; that selection enables
source-aware capture together with V7. Existing V3/V4/V5/V6 layouts and semantic
milestones remain separate.
