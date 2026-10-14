# DRAW-02 temporal mutator surface relationships

Step 2 needs to distinguish observed shared surfaces from unresolved backing
relationships at physical API witnesses. This is an offline diagnostic policy,
not a recovered driver ordering contract or an ownership admission rule.

The existing interval decoder retains each competing surface call whose original
body overlaps the witness's entry-to-ACK interval. Its conservative mutator set
includes lifetime changes, CPU/DC acquisition/release and presentation methods;
membership does not prove that a call wrote pixels or succeeded.

`drawing_surface_lifetimes.py` additionally records whether a listed competitor
shares a local generation or a successful QueryInterface alias with the witness's
source or destination while that competitor's original body remains pending.
Query links take effect at the observed original return. Failed queries and
attached-surface acquisition cannot link generations. A query returned after a
competitor's original return cannot retrospectively classify that competitor.

Only Blt, BltFast and Flip carry a related surface generation in this classifier.
Other related fields cannot establish surface sharing. Two calls may merely read
the same source, so even a known shared identity does not prove a write conflict.
Unlinked generations have an unresolved backing relationship; they are never
classified as disjoint surfaces.

For every witness, the result preserves the decoder's full competitor count,
the bounded listed count, known shared relations with their first observed event,
unresolved listed entries and the unlisted count. The existing 256-entry limit
remains explicit; an unlisted competitor cannot become a classified success.
Disjoint storage, actual writes, exclusive frame ownership and complete backing
alias resolution remain false independently of the observed counts.

Six additional synthetic guards cover prior successful/failed queries, queries
during overlap, future-query refusal, common read sources, attachment refusal and
the listing boundary. These tests supplement the existing ten lifetime and eight
interval guards. Pinned original-journal analysis is a separate milestone; it
does not rerun the original game or prove native pixel parity, all output routes,
raw CPU writes, driver completion or full 47/21 closure.


Current original120-update journal analysis02 passes all24 guards and preserves
every pre-change lifetime field. Of859 physical witnesses,139 have217 known
shared competing BltFast calls and515 have2052 unresolved listed competitors;
84 witnesses contain both categories. All competitor listings are complete for
this capture.1804 observed leases match releases, but unknown backing aliases and
raw CPU writes remain unresolved. No write, disjointness, exclusive ownership,
native parity, driver completion or complete presentation claim is made.
