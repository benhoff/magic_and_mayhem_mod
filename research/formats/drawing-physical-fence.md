# Guarded original post-copy physical witnesses

This opt-in diagnostic policy extends the existing two guarded opaque-copy sites.
It preserves their original COM call, result/register/FP/LastError forwarding and
source/queue/canvas/menu-phase record. `MNM_DRAW02_PHYSICAL_FENCE=1` additionally
flushes queued GDI output and holds the original draw thread after each successful
primary readback until a reference-only X11 peer acknowledges a full physical
capture. Other original threads are not suspended; performance and unobtrusive
gameplay equivalence are not asserted.

`physical-request-NNNN.bin` is the exact 96-byte original publication record.
The peer validates the request, discovers the unique original 800×600 client,
requires the entire client to fit the physical screen and takes two complete
RGBA8888 X11 captures 5 ms apart. Unequal captures refuse. The reference-only peer
never reads the primary or producer oracles. It writes the full physical plane,
its identities/hashes and `physical-ack-NNNN.bin`: 8 bytes `MNMPACK1`, little-endian
frame ordinal and original attempt ordinal. A malformed acknowledgement or a 5 s
timeout terminates the diagnostic experiment; it cannot claim completion.

External admission requires exact request bytes from the immutable original
trace, exact acknowledgement identities, all physical artifacts in order,
typed source/thread/action metadata and full-plane equality with the independent
original RGB565 readback's normalized display expansion. In the tested Xvfb/Wine driver path, each channel expands as `(channel * 255 + maximum / 2) / maximum`, using integer division and maxima 31, 63, 31. This is a fixed `normalized-nearest` display policy, distinct from the native presenter's default bit replication. The first executed acquisition rejected bit replication; diagnostic comparison of all 49,440,000 pixels found zero differences with nearest normalized expansion and observed all 32/64/32 channel levels. Fresh execution is required for the updated policy; no tolerance, measured colour table or oracle enters native rendering. Native rendering receives only
the owned producer history and encoded resources. Physical pixels, primary
readback pixels, expected hashes and original call results stay in the comparator.

The boundary is `fenced_opaque_primary_point`. It supplies synchronized witnesses
at these original call boundaries; it does not account for all global presentation
routes or freeze the whole process. Complete journeys, semantic input, broader
battle/save/load source ownership and physical-driver recovery remain separate.
