# Surface metadata and unsupported mutation callback admission

Continuous-only opt-in `MNM_RENDER_ORDERED_COPIES=1` now admits application
GetSurfaceDesc, GetAttachedSurface, AddAttachedSurface, DeleteAttachedSurface,
Restore and BltBatch through the existing drawing callback lease. This is
intentional native scheduling policy; it does not recover driver ordering or
change default enablement. Original arguments, results and LastError are retained.
Scoped cleanup covers success and failure. The lease spans original execution
and the existing successful metadata observation or invalidation. No tracker is
held across original execution or the bounded wait. Existing 50ms admission,
8ms tracker and memory/queue/surface budgets remain unchanged.

An admission timeout forwards the original once and invalidates metadata/pixel
epochs before and after forwarding, without observing returned descriptions or
interfaces. The interrupted producer stream refuses rather than committing a
copy against uncertain metadata. Returned unseen attachments need a later
admitted observation. Attachment relationships are surface metadata, separate
from QueryInterface alias provenance.

Admitted failed originals leave tracked state unchanged. Successful attachment
mutations invalidate metadata. Successful Restore and BltBatch retain their
existing target invalidation: restored pixels and batch semantics are still
unsupported. This extension orders their refusal after an already admitted copy;
it does not manufacture a checkpoint or replay their mutations.

## Validation

The [retained native matrix](opengl-metadata-order-native-20261006.json) passes
12 cases with18 complete frame comparisons, two valid and ten refused sessions.
It exercises a pending primary copy against each of the
six callbacks, with a worker performing a failed original followed by successful
retry. Positive cases require the original worker to wait until the copy commits;
timeout cases force the outer original to wait for the worker and require refusal
of its pending copy. Exact original call counts, forwarded arguments, signed
results, LastError, independent full-frame hashes and terminal storage are checked.
The positive unsupported cases allow a100ms consumer drain window between the
failed call and successful invalidation; prompt cancellation is not promised to
drain previously queued commands. Expected pixels come from the fixture's native buffers, never producer state.

Description and attachment queries retain two complete presentations in the
ordered case. Unsupported successful mutation cases retain the preceding copy's
presentation before GAP. Every timeout retains only the initial presentation.
The [nine-case regression](opengl-metadata-order-regression-20261006.json) passes
745 complete frame comparisons and covers shared palette resource recreation, nested metadata,
source-key invalidation and ordered DC/flip/CPU handoffs.

Original application execution and driver equivalence are not rerun in this
chunk. Historical live evidence remains historical. Dedicated nested metadata,
attachment recreation/stale outputs, Restore/BltBatch reconstruction, startup
DirectDrawCreate admission, borrowed CPU/DC intervals and owner termination,
missed-object interception recovery, dedicated palette lifetime races, repeated
scenes/actions and animated/full-frame/original-driver equivalence remain pending.

Exact committed history `420f919..9e8868e` was reviewed against parent/current
file and behavior receipts, preserving old evidence hashes and receipt order,
with no unresolved entries. This review does not assert new validation of those
intervening changes.
