# Native command continuation versions 1–9

This is a native application format inside the outer native `State.systems`
checkpoint. It is unrelated to original Magic & Mayhem save/object layouts.

The byte stream starts with four ASCII bytes `CMD1`, then little-endian32-bit
version9 for new writes. Every integral scalar/enum uses one little-endian32-bit word. Signed
integers use two's-complement words with explicit checked reconstruction; narrow
values reject overflow. Booleans are exactly0 or 1. An optional is a0/1 word and,
when present, its value. A vector begins with its word length; a fixed array has
no length and retains declaration order. No padding, address, native pointer,
function or C++ object representation is written.

The continuation field order is world, execution vector, callback-binding vector,
generation vector, scheduler, scheduler environment, seed, backend byte vector. Version2 then appends maintenance and movement vectors.
Nested field order is explicitly listed by the `fields` functions in
[codec.cpp](../../apps/command-session/codec.cpp); these are the explicit schema,
not automatic host layout reflection. Changes require an explicit version/migration
review. Arrays include all AI coefficient tables and 42 inventory entries; orders
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
writes refuse losing action or maintenance state; that schema writes version3.
Motion division/sample and cyclic object/contribution semantics are checked by
the recovered bodies at execution, with the outer session retaining rollback.
Original-save rebinding and dynamic actor lifecycle remain separate.

## Version 4: native provider continuation

Version4 preserves the complete version3 prefix and appends a presence boolean.
If present, explicit little-endian words encode the immutable profile stamp,
sequence/tick/alternate flags, original 250-word RNG and cursors, actor/body/overlay/
auxiliary and eight World ANI channels, terrain/occupancy cells, open searches and
node records, known packed route context bytes, segment initialization and
environment pass state. ANI record positions are checked32-bit ordinals rather
than host pointers. The profile stamp is two32-bit words on both i386 and amd64;
execution records separately retain SHA-256 provenance. It is not a security hash.

Readers admit versions 1–3 with absent native state. A profile requiring native
providers rejects such state. Explicit legacy writers reject dropping a present
version4 payload. Fixed actor roots, channel bounds, RNG cursors and occupancy
references are checked; the 1MiB envelope and 65536-cell/search bounds are native
admission policies, not original save compatibility or exhaustive campaign scale
validation. This schema originally persisted per-actor searches with multimap
ordering and node records. The version5 reader's migration restrictions follow.

## Version 5: shared actor request and sample ordinals

Version5 retains the version4 prefix, then appends the shared actor-route context
and per-actor route metadata after the environment-pass state. Context order is
active, displacement continuation, mode, signed shared budget, signed request
budget, destination, origin, complete route scratch, dirty/region/diagnostic
booleans. Scratch contains destination, cursor/count, sixteen seven-word points
and fourteen motion words. Metadata is a vector of region destinations and
marker69c words, absent or exactly the actor count. Region-enabled and displacement
continuations remain unsupported by this native binding and refuse validation.

Shared request active and scheduler continuation represent the same original
word at 6df184. The binding synchronizes them at scheduling boundaries; conflicting
saved values refuse validation.

The existing search vector retains its actor-count shape; slot0 now holds the
single scheduler search, matching the original shared request. Immediate512800
requests have an independent250-expansion budget and finish synchronously. They
do not change scheduler continuation or consume its budget.

Native motion sample cursors now index the complete48-word type table, allowing
reverse playback to cross into the preceding sample bank. Inline samples retain
their existing representation. Version4 reads/writes are admitted only with no
initialized searches or active motion; such old state lacks shared request
identity and uses relative sample cursors. A version4 write additionally refuses
non-default request scratch or metadata. Quiescent state migrates without
inventing active search state. Unknown versions and malformed route counts,
cursors, active flags and metadata extents refuse before publication.

## Version6 friendly displacement

Version6 appends an actor-count vector of displacement-owner slot indices after
the version5 native route metadata. Priority, saved goal and one-segment pause
already live in the execution/motion entries. The shared route displacement flag
is now admitted as0/1. Owner roots match the actor count and references remain in
range; shared active still aliases scheduler continuation. Native failed-frame
publication is atomic, including temporary displacement search and owner state.

Version5 reads seed zero owner entries. Writes to versions 4/5 refuse a nonzero
owner or displacement continuation rather than discard it. Existing version4
search/motion restrictions remain; no old heap pointer is rebound. Native version6
is independent of original game saves.

Version7 appends owned regional navigation after the version6 provider prefix:

1. Region vector: canonical position, eighteen connections (signed cost,
   destination index, twenty signed clearance bytes), parameter and traffic
   words, eight seven-byte danger rows and deducted rows, and the danger-base
   byte. Narrow fields still use checked32-bit wire words.
2. Regional search: dirty/initialized flags, start/destination/best indices,
   best heuristic, ordered open entries (priority/index) and node records
   (index/priority/predecessor/28-byte movement payload).
3. Two unsigned danger power inputs, corresponding to the explicitly supplied
   original regional danger coefficients.

Each regional vector is bounded to 4096 entries, subject to the independent total
one-MiB wire bound. Nonzero edge costs must be positive and name a region;
positions are canonical and sorted by X. Initialized search roots, unique node
records and open references are checked. Pending actor regional paths must name
valid graph indices or the−1 terminator. Their fifty-entry arrays, cursors and
final-destination metadata remain in the earlier actor/request prefix.

Version1..6 reads default to an empty graph/search and zero powers. Pre-v7 writes
refuse to discard any regional graph, active search, regional mode or nonzero
power input. An enabled empty graph can still use the original local-routing
fallback under v7. Failed staging or malformed restoration publishes no state.
Graph construction/import and dynamic World danger production remain separate
from this portable continuation policy.

## CMD1 version8: regional graph maintenance (NS74)

Version8 appends an optional maintenance record after the version7 provider fields.
When present, it contains28 normalized `{height,width,movementKind}` triples;
1345 `{node,mode,priority}` ring entries; unsigned read/write positions; signed
active/other/stage/height/probe-budget cursors; opening/closing booleans; working
and previous connection descriptors; and a generic probe continuation. The probe
stores FIFO ordered `{priority,canonicalCellId}` open entries, unique cell records
with predecessor/movement payload, best cell/heuristic and initialized flag.
Canonical cell IDs are one-based indices into the owned terrain array.

Every scalar follows the existing explicit DWORD wire encoding. No C++ layout,
legacy pointer, tree node or borrowed geometry view is serialized. The decoder
checks queue/cursor/domain/reference limits and the existing1MiB encoded limit.
Older versions still decode with absent maintenance. Explicit writes of versions
1..7 refuse a present maintenance record. Native startup retains the 28 shape
triples needed by later repairs; an older graph is not silently reconfigured.
See [regional graph maintenance](../runtime/regional-graph-maintenance.md) for
recovered semantics, native edit policies and execution boundaries.


Version 9 appends an optional regional danger producer after the version8 native
maintenance state. It owns dimensions/tick, eight enabled owners and commander
slots, alliance/sharing bytes, ordered per-owner memories (key, observation tick,
logical actor snapshot), current live actor projections, declared effects,
observation points, base hazards and fire/animation range parameters. Actor
snapshots store slot/owner/region/position/type/class/shape/movement, health,
resource/maximum/inventory/threat/aging/range, status flags and eight resource
estimates. No original pointer or tree representation is saved.

Producer state stores independent reset flags and owner/memory/actor/region
cursors, current commander, pending totals, pending seven-byte danger/deduction
rows, pending base bytes, eight published owner totals and per-actor support.
Integral and byte fields still use the canonical 32-bit wire words above. The
published danger/deduction/base rows remain in the version7 regional records.

Enabled production requires a configured nonempty regional graph, matching
world dimensions and canonical live actor slots. The pool is bounded to 125,
memories to 125 per owner, graph/pending rows to 4096, and effects/hazards/observation
points to 128 each. Commander, region, shape, owner, ordered unique memory and
pending cursor references are checked; pending row and support lengths must
match their roots. The existing1MiB encoded limit remains independent, so the
largest individually admitted dimensions need not fit a checkpoint together.
Versions 1..8 read with absent danger state. Explicit older writes refuse when
danger production is enabled. Native observation, resource and frame policies
are documented in regional-danger-production.md; this format establishes no
original save equivalence or full World integration.
