# Continuous World V6 source chunks

V6 (`MNMCON06`, version6, mode3) is the synchronous public resident extension to
V4. It retains the128-byte shared/slot headers, two existing slots,16MiB source
capacity and ordinary80-byte resident World acknowledgements. V3/V4/V5 envelopes
and peers remain distinct. Independent/asynchronous V5 and finite compressed
World-profile captures retain their prior protocols and bounds.

World handshake states0..5 and its u64 generation are unchanged. Source chunks
use states6 SOURCE_REQUEST,7 SOURCE_PROCESSING,8 SOURCE_ACK on the current World
slot. During these states only slot words0,2..3 (World generation),4 (source byte
count),6 (source CRC32C),22..23 (global u64 source ordinal) may be nonzero. Source
bytes must exceed the64-byte V3 producer header and stay within the fixed source
area. Source ordinals start1 and remain contiguous across World generations.
There is no request/reply framebuffer, original-return record or World credit.

The producer publishes only between closed World queues, under its emission
lock, before a new source operation's payload-cache lookup. At16384 records or
4MiB it drains the existing prefix. A definition+draw is never split by this
operation-level check; a record-count chunk can therefore contain16385 records.
A single source operation may cross the4MiB target but retains existing maximum
payload/16MiB guards. No World queue or deferred-overlap bound is increased.

Native checks immutable session envelope/nonce, producer role, World generation,
source ordinal, reserved fields, extent and CRC before taking a copied lease.
It decodes the V3 prefix and rejects World entry/return, failure or update-end
records before applying any operation. The existing producer/GPU mirror retains
canvas identities, defined pixels and lifetime history between chunks. A failure
is terminal; there is no partial-history retry. It publishes native writes before
acknowledging the unchanged prefix. ACK cannot count as a World return or import.

The producer waits using the existing bounded cancellation/deadline policy and
checks acknowledgement identity before retiring payload cache pointers into the
slot. It clears local source sequence, checkpoint IDs and payload definitions,
then resets the same slot to WRITING with the same World generation. Future
chunks define their own payload references. Native canvas/history state persists.
Ordinary World completion still requires its separate actual original return,
command import and contiguous generation before releasing/flipping the slot.

Resident reports separately count acknowledged chunks, source records/bytes,
maximum chunk records and maximum drained records within one World interval.
The last metric is a lower bound: it excludes the undrained tail. Successful
shutdown does not claim a complete presented-session pixel trace, and the last
partial source prefix is still retired with the original session.

This is a native transport policy. It does not establish original-driver pixel,
allocation or timing equivalence, support arbitrary resource churn, or validate
other modes' finite capture limits.
