# Kind9 sprite trail placement

Build: unchanged No-CD40209ca7. Original function4df840..4df99f; the
existing binary inventory already records this function and its boundary.
Confidence: high within the bounded, overflow-free original comparison domain.

## Recovered behavior

The primary/software kind9 branches pass four DWORDs from row+4's pointed
record, row+8, and signed row offsets to4df840. Here the first four values
are interpreted as start/end x/y coordinates. ECX is start x, EDX is start y;
stack arguments are end x, end y, source frame, shade and ordinal. The row+8
value is the source frame input on this branch. This helper does not dispatch
a virtual method or dereference a native receiver object. The row dispatcher
ignores its return and writes zero metadata; the auxiliary table skips kind9.
Natural producer inputs and source ownership still require separate proof.

4df840 stamps580a90 at each Bresenham point including both endpoints.
Coincident endpoints emit one stamp. Horizontal/vertical paths step by sign.
The major axis selects the general loop; the minor coordinate advances when
its doubled accumulated error is strictly greater than the major delta. A
half-step tie therefore stays on the previous minor coordinate. Each call
forwards the same frame/shade/ordinal and the current point. The helper returns
the final stamp's entire EAX unchanged, even though queue kind9 discards it.

`world_trail.hpp` implements this placement with an opaque source token and
injected raster backend. The unchanged original helper, with only580a90
synthetically bound, is compared over42483 cells spanning all octants,
coincident/axis-aligned endpoints, ties, signed positions/offsets and high EAX
returns. Its original instructions remain unchanged throughout execution.
This proves placement/call/return semantics, not580a90 pixels or a live writer.

## Intentional native work bound

The caller supplies a maximum stamp count. Native admission checks
max(abs(dx),abs(dy))+1 before any effect and refuses a request above that
budget. It also refuses deltas beyond INT32_MAX/2 rather than executing
legacy doubled-error signed overflow. These are native policies; they are not
original clipping or recovered allocation limits. Two rejection controls check
that no stamp is emitted for excessive work or the overflow domain.

The owned model is separate from live queue installation and native raster
admission. Constructors/writers, source resolution, palette/history, concrete
580a90 pixel semantics and complete rendering replacement remain pending.


## Queue-to-trail adapter

`worldTrailForQueueCall` validates the typed kind9 backend and seven logical
values, then decodes four signed coordinates, an owned frame token and two
signed raster options. The queue's nominal x field supplies the frame token
on kind9; it is not the destination coordinate on this branch. Native software
mode supplies shade zero, while primary mode retains the signed row shade.

The follow-up comparison executes the actual unchanged5002a0 consumer and
4df840 helper together, binding only580a90.84966 primary/software queue
cells match the native row-dispatch/typed-adapter/trail pipeline, including
zero metadata and untouched queue/container fields. The42483 direct helper
cells remain, for127449 total cases. Four native controls cover work budget,
error overflow, wrong backend and wrong argument count before any stamp.
The earlier WTRAIL report and all its original source hashes remain unchanged;
WTRAIL2 records this expanded source/contract/scenario closure separately.
The downstream raster, physical source ownership and live installation remain
outside this headless integration.
