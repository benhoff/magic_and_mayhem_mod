# Native melee exchange checkpoint

This is a native diagnostic format, not an original game save or shared C++
layout. Outer native snapshots provide checksum, extent and atomic publication.

`State.campaign` is `MNMMPR01`, followed by three little-endian length/byte blobs:
encrypted creature CFG, encrypted HTH CFG, Crocodile ANI. Exact bytes determine
a SHA256 digest; the first8 digest bytes interpreted little-endian form the
nonzero driver identity. The version pins sequence24, neutral response and
handled-stun endpoint policy. Each input is bounded at4MiB. No host paths or
pointers are saved. Changing rule semantics requires a new version/identity.

`State.systems` is `MNMMLX01`: driver QWORD, health-rules QWORD, RNG blob,
then EXACTLY two actor records. Each record contains slot, generation, health,
cooldown and phase DWORDs followed by a length/byte cursor blob. Individual
subsystem blobs are bounded at16384 bytes. No trailing bytes are accepted.

RNG is1008 bytes: left/right DWORDs,250 DWORDs. Cursors are below250, separated
by103 modulo250. An attacking animation cursor is28 bytes: PC, displayed record
(UINT_MAX means absent), active, delay, elapsed, repeats, break. The adapter
admits only states reachable in the finite selected sequence. Recovering/dead
actors have empty cursors; recovering clocks are1..34, others zero.

The app decodes the embedded profile before constructing the session. Identity,
health, complete generation handles, stationary geometry, phase/cleanup and
cursor/RNG invariants are checked before publication. Unsupported rules,
corruption, throwing adapters and failed loads preserve the prior session.
Legacy combat/skirmish snapshots use different versioned contracts and are not
silently migrated into this lab.
