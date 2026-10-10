# Native ordered World rolling channel v1

Intentional native policy, separate from the drop-latest shadow channel and
finite V3 capture. All fields are little-endian integers; there are no pointers
or shared C++ layouts. The file is created exclusively for a nonzero64-bit
session. One producer and one consumer claim their roles once. The current
implementation supports little-endian QFile mapping with lock-free32-bit
acquire/release atomics, validated on this Linux host only.

The64-byte header has magic `MNMROLL1` at0, version1 at8, header bytes64 at12,
session low/high at16/20, two slots at24, input capacity16MiB at28, reply
capacity8MiB at32, slot extent at36 and file extent at40. Words44/48 are atomic
producer/consumer role claims,52/56 reserved zero, and60 terminal cancellation.
The total file is64 +2*(64 +16MiB +8MiB) bytes; capacity never grows.

Each slot starts with64 metadata bytes: atomic state at0, monotonic64-bit
sequence low/high at4/8, input byte count/checksum at12/16, reply
width/height/byte count/checksum at20/24/28/32, reserved zero through63. Input
bytes follow the slot header; RGB565 output bytes start after the fixed16MiB
input region. FNV-1a32 checksums detect accidental corruption; this cooperative
channel is not an authentication/security boundary. Output dimensions are
positive, at most2048 each, and their exact product must match the reply extent.
High bits in native pixel words are refused before any completion is published.

Ownership is FREE0 -> WRITING1 -> READY2 -> READING3 -> COMPLETE4 -> FREE0.
Only the producer publishes/frees and only the consumer leases/completes.
Release publication and acquire ownership protect ordinary metadata and payload
words, including the split64-bit sequence. Slot `(sequence-1)%2` admits only the
next sequence. Full slots return backpressure, never drop or supersede input.
The consumer copies the input into owned storage before execution; its slot
remains leased until completion. The producer owns a copied reply before freeing
the slot. Both peers refuse unknown states, stale identities, gaps, malformed
extents/reserves/checksums and wrong roles. There is no sequence wrap. The
orchestrator must bound waits; missing peers require terminal cancellation and
a newly created channel/session. Reattaching to continue abandoned history is
unsupported.

Input packets contain one complete World queue plus its preceding native
startup/HUD producer operations. The adapter reuses V3 record validation by
constructing its standard64-byte header, packet-local owned sources and local
record numbering1..N; entry/return queue ordinals are locally1. Its V3 header
still declares32, but is parsed as an incomplete capture and then strictly
validated as exactly one closed queue. This is explicitly a new native packet
policy, not an indefinite V3 append. Source-definition records25 deduplicate exact owned font/raster bytes inside that packet, bounded to2048 definitions/16MiB. Definitions from the saved capture are rebuilt with local record identities; references cannot cross packet/session boundaries. Requests beyond the table bound remain inline subject to the same16MiB packet limit. The rolling channel sequence
owns cross-packet ordering. Exactly one World-canvas checkpoint, at least one
raster, unchanged canvas/extents, at most12320 rasters, supported inside-World
operations, and a final matching return are required. No raster may follow the
World checkpoint. Packet bytes are bounded by16MiB before decoding; no source
table or capture journal accumulates across packets.

Completion is native GPU RGB565 output. Native startup/HUD producer history and
source requests are the only drawing inputs. A persistent independent native
CPU producer asserts GPU completion; saved output digests can assert results
but have no pixel-input path. Live PE32 producer adaptation, original state/AX
guards, original cadence, actual process death supervision, physical input and
hardware execution remain separate milestones.

The World adapter has an explicit rolling mode: local producer entry/return ordinals remain1, while `beginRolling`/`completedSequence` carry ordered64-bit session sequence. It refuses mixed finite/rolling modes and exhausted counters. Legacy finite callers keep their existing32-bit queue/return contract; `completedQueues` remains the finite-mode accessor. The rolling renderer retains only its latest exact-byte decoded packet within the packet/source bounds, and re-decodes any changed byte. Completed histories and decoded packet sources remain independent.
