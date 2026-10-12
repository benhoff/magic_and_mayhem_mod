# Native creature encounter subsystem v1

`MNMCAI01` is an owned native subsystem within the existing native World snapshot.
It is not an original game save or a packed original creature record. All words
are little-endian DWORDs; identities are two DWORDs. There are no host pointers.

The eight-byte magic is followed by rules and health identities, a length-prefixed
RNG blob, quiet-tick count, dimensions XYZ, length-prefixed wall bytes, and actor
count. Each actor contains handle slot/generation, home XYZ, health, facing,
cooldown, recent attacker plus one, recent age, signed target slot, signed stun
timer, phase, and length-prefixed animation cursor. Phases are idle, pursuing,
attacking, ready, stunned, dead, blocked, knockback (0..7).

The exact three installed input blobs remain in the campaign profile `MNMMPR01`;
configuration is decoded and the adapter identity checked on every fresh-process
load. All 250 RNG words and both indices and the seven-word animation controller
are persisted. The bounded map is 16×16×3, four type17 actors, two owners, flat
layer1. Validate living target identities, hostility, health/lifecycle, phase,
footprint non-overlap, wall bits, cursor reachability, RNG indices, quiet-count
and no unexpected movement/animation bindings before accepting a continuation.

A staged tick publishes only after complete validation; refusal preserves the
previous state. Unadmitted ticks preserve bytes, alternate mode advances only the
base World clock, and terminal ticks stop. A native stalemate after 160 ticks
without progress is serialized independently from death. This wire format does
not assert equivalence with original AI or movement scheduling.
