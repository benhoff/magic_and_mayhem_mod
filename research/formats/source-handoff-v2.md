# Immutable source handoff journal, version 2

All values are little-endian DWORDs; no host pointer appears in this journal.
Header: 64 bytes, magic `MNMSHF02`, version2 at DWORD2, record size256 at DWORD3,
pinned build marker40209ca7 at DWORD4, maximum131072 records at DWORD5 and maximum
2048 handoffs at DWORD6. Header DWORDs7..15 are zero. Version1 (`MNMSHF01`) remains
readable and admits only original private allocation kinds1 and2.

Record DWORDs0..20 retain the version1 ordinal/phase/thread, submission,
texture/generation/resource, kind, dimensions/pitch/packed size, CRC, private stage
owner, epoch, physical-journal prefix, refusal reason, original heap generation,
heap offset, original owner and state fields. Phase1..7 are capture, snapshot
begin, sealed value, binding admitted, consumption finished, original restored,
and private stage retired. Phase8 is terminal; phase9 is refusal.

Kinds3..6 respectively identify shared, file-mapped, imported and GPU providers.
For those kinds, original private provenance DWORDs17..19 are zero and the
following exact provider identity is repeated unchanged through every phase:

| DWORD | Meaning |
| --- | --- |
| 21 | Nonzero lifetime provider ID |
| 22,23 | Nonzero 64-bit producer object identity, low/high |
| 24 | Nonzero object generation |
| 25 | Nonzero monotonically increasing provider lease token |
| 26,27 | Required producer fence, low/high |
| 28,29 | Completed producer fence, low/high |
| 30 | Exactly15: stable read, all cooperating writers excluded, lifetime pin, binding transfer |

Completed fence must be at least required; GPU required fence must be nonzero.
The provider kind cannot change for an ID, tokens cannot repeat/go backward,
and object generations cannot go backward. Private kinds1/2 require zero
DWORDs21..30. DWORDs31..63 are always zero. Terminal records carry no provider
identity. A complete record sequence is metadata accounting, not independent
proof of a provider's enforcement or hardware execution.
