# Native command continuation versions1 and2

This is a native application format inside the outer native `State.systems`
checkpoint. It is unrelated to original Magic & Mayhem save/object layouts.

The byte stream starts with four ASCII bytes `CMD1`, then little-endian32-bit
version2 for new writes. Every integral scalar/enum uses one little-endian32-bit word. Signed
integers use two's-complement words with explicit checked reconstruction; narrow
values reject overflow. Booleans are exactly0 or1. An optional is a0/1 word and,
when present, its value. A vector begins with its word length; a fixed array has
no length and retains declaration order. No padding, address, native pointer,
function or C++ object representation is written.

The continuation field order is world, execution vector, callback-binding vector,
generation vector, scheduler, scheduler environment, seed, backend byte vector. Version2 then appends maintenance and movement vectors.
Nested field order is explicitly listed by the `fields` functions in
[codec.cpp](../../apps/command-session/codec.cpp); these are the explicit schema,
not automatic host layout reflection. Changes require an explicit version/migration
review. Arrays include all AI coefficient tables and42 inventory entries; orders
include serial, kind, destination, target, optional effect token, reference
position and repeat flag. Backend bytes are owned continuation, not handles to
external mutable services. Transient operation-local recursion guards are not
saved; publication occurs only after those operations complete.

Bounds are1..128 actors, at most15 orders per actor, at most65536 backend bytes
and at most1MiB encoded continuation. Dimensions are positive and bounded to
4096×4096×256. Actor indices/generations, callback tags0..4, positions/references,
queue counts/unique serials/next serial and scheduler cursor/phases are validated.
Empty queues may retain the historical logical cursor; nonempty queues need a
live cursor or admitted cycling end. Noncycling end continuation is rejected.
Unknown versions/magic, overflow, malformed booleans/optionals, excessive lengths,
truncation and trailing bytes refuse before publication. The runtime additionally
checks exact profile identity, fixed pool/geometry and entity projections.

Evidence distinguishes native format/session integration from recovered engine
body comparison and complete original campaign behavior. The original sentinel
mapping is only understood within an explicit zero-payload domain; native zero
sentinel and unsafe-publication refusal are registered policies.

Version2 maintenance entries store every scalar, eight visibility ages and optional
status/damage/flight effect tokens. Movement entries store eight renewed-order
arguments, landing kind and next route cell. Both vectors are either absent or
exactly match the dense actor pool. This continues the same word encoding, bounds
and explicit nested field order; it does not serialize providers or host layouts.

Version1 reads exactly the prior prefix and leaves both vectors empty. No missing
engine state is invented. Runtime profiles requiring maintenance/movement roots
reject such restoration; supplied-body profiles can migrate without changing
previous fields. An unsupported version, partial pair of vectors, pool mismatch
or trailing bytes refuses. The outer native map/profile identity stays stable
and remains separate from the continuation version.
