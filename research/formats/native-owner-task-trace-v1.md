# Native ordinary owner task trace, version 1

This is a generated test artifact, not an original game format. It records the
ordinary task-execution projection used by the NS86 offline oracle. The encoder
and bounded reader are `tests/owner-task-trace-wire.hpp`; no host struct layouts,
pointers, callback addresses or compiler padding are serialized.

Files begin with the eight ASCII bytes `MNMOT001`. Each following record has a
little-endian uint32 payload-byte count, then that many bytes. All payload words
are little-endian 32-bit integers; signed fields use two's-complement encoding.
Booleans are 0/1. Lists start with a word count. Positions are signed x/y/z words.

Payload order:

1. Tick, owner, risk percentage, initial and final count budgets; world dimensions.
2. The 8 × 8 relation matrix; creature count and ordered creature records. Each record
   has slot/type/kind/health/owner/physical/engaged/incoming/readiness/dirty932/
   active/ranged words followed by its position.
3. Member IDs, per-member raw/net danger and per-world-actor threat lists.
4. Current owner's memory count and ordered key/marked-tick/live-slot triples.
5. Task state before and after execution. Each contains root/risk/node count;
   each node contains memory key, parent, position, region, uncertainty squared,
   flag104, activity and ordered child IDs.
6. Result continuing flag, order count, then each order's continuing/stop/kind/
   target words and position.

The replay admits files up to 256 MiB and records up to 1 MiB; it bounds worlds to 8
actors, member/order lists to 3, memories to 125 and task trees/children to 251.
It requires complete record consumption and compares the recorded output against
fresh native64, original32 and sanitizer execution. The successful producer's
report fingerprints these files and its code closure. A transcript is supplied
input to the oracle, not proof of original whole-battle or World equivalence.
