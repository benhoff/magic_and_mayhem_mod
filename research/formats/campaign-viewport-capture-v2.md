# Campaign viewport capture version 2

This is an opt-in native diagnostic encoding, not an original game format.
`MNM_ACCEPTANCE_CAMPAIGN=1` selects it for the viewport role only. Other capture
roles and ordinary sessions retain version 1. The `MNMACE01` envelope and each
little-endian metadata/compressed-body length pair remain unchanged. Metadata
uses JSON; bodies retain the qCompress length-prefixed zlib encoding.

The open record has version 2, role `viewport`, policy
`campaign-lossless-viewport-v1`, stored limit 1 GiB, decoded limit 64 GiB, row
limit 131,072 (including open/close), and full-keyframe interval 128 nonempty
paints. The pending worker queue remains 256 MiB/256 jobs, including its current
job. A body still has a 32 MiB bound. These are capture-resource policies;
original simulation pacing, input, rendering and producer deadlines are unchanged.

Every nonempty paint body describes the complete native RGBA viewport and carries:

| Field | Meaning |
| --- | --- |
| `capture_body_encoding` | `full-rgba-v1` or `xor-previous-paint-v1` |
| `capture_body_base` | -1 for a full plane; exact record index of the previous nonempty paint for a delta |
| `capture_body_sha256` | SHA-256 of the complete reconstructed RGBA bytes |

A delta is the bytewise XOR of the current and previous complete owned planes.
Its dimensions and length must match the base. Paints 0, 128, 256, ... are full
keys; a dimension/length change also requires a full plane. Empty paints/swaps
do not become bases. Every paint and swap remains in FIFO order with its original
capture timestamp and identity. No original/reference pixels enter this encoding.

The worker computes encoding/hashes after the caller has frozen a deep copy.
Queue, raw, row, compression or stored overflow retains an explicitly incomplete
terminal record; it never silently samples frames or marks truncated proof complete.
Terminal byte counts describe complete reconstructed payload extents.

The reader verifies every reconstructed paint before admitting a closed stream.
It keeps compressed payloads and metadata, with a fixed number of plane-sized decode buffers,
then reconstructs planes on demand for the existing independent comparator. A
backward seek starts at a full key no more than 127 paints away. Successful codec
controls establish lossless diagnostic transport only; campaign outcomes,
renderer equivalence and populated lifetime handoffs need separate execution.
