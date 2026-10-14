# Native command continuation versions1,2 and3

This is a native application format inside the outer native `State.systems`
checkpoint. It is unrelated to original Magic & Mayhem save/object layouts.

The byte stream starts with four ASCII bytes `CMD1`, then little-endian32-bit
version3 for new writes. Every integral scalar/enum uses one little-endian32-bit word. Signed
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


Owned action selection/event integration uses the existing execution fields,
including optional logical `actionCallback`, animation facing/offset, previous
movement carry, route counters, readiness and ranged deadline. Callback identity
is independent of the current action word. No wire fields or version change.
The profile's ANI/RNG/cast provider state must be saved in staged backend bytes;
its schema is profile-specific, not an original save/ANI/RNG format claim.


Version3 appends an original-behavior-descriptor flag and complete action-body
state after the version2 prefix. Per-actor callback state stores sequence,
contributors and animation-break state. Motion stores all interpolation/sample
cursors,48 samples, residuals and flags. Segment state stores16 route points,
contribution graph/totals,50 region entries, cursors and overlay/landing fields.
The object-action world stores dimensions, WORD heads and logical object links.
Every field follows the same explicit word/vector encoding; no pointers or
providers are serialized. The three per-actor vectors are absent together or
match the dense pool; present state requires maintenance/movement and a complete
bounded object grid. Original descriptor profiles permit logical tags0..16,
including null12; supplied command profiles retain tags0..4.

Version1/version2 readers leave the appended state absent. Full action profiles
reject restoration without required roots or descriptor mode. Explicit legacy
writes refuse losing action or maintenance state; normal writes use version3.
Motion division/sample and cyclic object/contribution semantics are checked by
the recovered bodies at execution, with the outer session retaining rollback.
Original-save rebinding and dynamic actor lifecycle remain separate.
