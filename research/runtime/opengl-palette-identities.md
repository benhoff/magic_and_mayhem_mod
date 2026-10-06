# Explicit native palette resources

Native policy: session-qualified palette IDs and immutable lifetime generations,
observed interface aliases, bounded palette storage and ordered
create/update/bind/delete records. Original COM calls remain active.
[Stream v2](../formats/render-stream-v2.md) describes the wire records.

Continuous launch defaults to `MNM_RENDER_PALETTE_RESOURCES=1`; explicit 0 keeps
legacy surface-local palettes. Bounded archive runs retain stream v1. Interface
aliases discovered by successful palette QueryInterface share one lifetime.
Each newly observed lifetime has a monotonic generation independent of mutation
guards. Pending updates require both guards; pointer reuse cannot inherit them.
Creation at a still-observed pointer refuses the stream. Independently published
palette identities cannot subsequently be merged. Capacity, identity exhaustion,
unknown colors and ambiguous ownership refuse rather than wrap or infer state.

Only original Release returning zero proves palette retirement. Dependent
bindings are detached before DELETE. Pending locks or DC handoffs prevent safe
retirement and invalidate the session. Tracker records are retired independently
of consumer resources, which survive until their ordered DELETE or session abort.
Shutdown deletes surfaces before remaining palettes. Recovery discards the old
palette namespace; immutable observed lifetime counters never reset.

Synthetic validation covers shared updates, independent complete GPU pixels,
fragmented decoding, rebinding, exact-generation guards, bounded capacity,
nonfinal/final Release, aliases and repeated same-pointer recreation. It does not
establish actual-game/driver equivalence, implicit COM destruction, unobserved
interfaces or allocation failure behavior. Counter exhaustion and conflicting
published aliases still need producer fault-injection tests. Late attachment
and complete retained-state recovery remain separate work.
