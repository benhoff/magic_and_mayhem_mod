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


## Step 2 current combined source/delivery and lifetime reassessment (2026-10-14)

GPU recorder teardown is committed as3f4614f8 after the actual staged gate and
normal hook passed. The6453a4c9..3f4614f8 first-parent range has15 exact file
receipts and zero gaps; no retrospective execution claim.

[Current source/worker/EGL analysis01](draw02-integrated-source-egl-01.json)
revalidates raw original32pilot03 API/results, submission, source-map, packet,
worker/context and physical-peer records. All2,108 observed source submissions
bind to workers. All355 eligible post-EGL copy regions and355 whole source images
match, with170,400,000 whole-image RGB pixels and114,905,600 copied-region pixels.
There are zero source/witness/boundary refusals, pixel mismatches or API mismatches
before a matching EGL witness. Only355 eligible EGL images are compared; the
capture's356 API planes are not relabeled as one global presented-frame catalog.
Whole-image agreement outside a copied rectangle is corroboration, not attribution
of every pixel to that command. No all-context/hardware/scanout or native proof.

[Current backing analysis01](draw02-integrated-backing-01.json) replays unchanged
protected writer/lifecycle records and passes4classifier tests. The2,179 unlocated
cache entries cover49canvas identities:2,169 lack any observed Lock return during
the window, and10precede later Lock returns. The analyzer's report scope now names
the supplied completed capture rather than hardcoding the older original02. This
label correction changes no classifier semantics or historical evidence. Absence
of a Lock observation leaves backing unknown; it is not a GPU-only classification.

[Current lifetime analysis01](draw02-integrated-lifetimes-01.json) passes16lifetime
and8interval tests. It observes70successful query links,68alias groups,1,504CPU/DC
leases,117retirements and zero lease gaps/remaining leases. Of356 physical witness
intervals,100expose globally open leases, none expose known source/destination
leases, and122contain competing mutators.28intervals have known shared competitors
and113have unresolved listed competitors;19have both. None has unlisted competitors.
Query knowledge is temporal; shared sources alone do not prove actual writes, and
zero known lease exposure does not establish exclusive backing ownership.

Reassessment: current bounded reference association is stronger; source allocation
and alternate views, unhooked writers, all presentation routes,120whole-update
source/action/frame identity and independently verified semantics remain open.
Step2 remains active, all47journey/21operation closures and steps3–7 remain open.
No native-policy status promotion and no historical fingerprint refresh.

The main checkout's non-destructive storage inventory is retained at
`working/retention/draw02-original32-storage-review-20261014-01.json`:1378.0236GiB
working and107.6993GiBfree at its measured timestamp, with0.8595GiBreview candidates.
Those are candidates, not deletion permission. Registered worktrees, pins, referenced
evidence and current work remain preserved; no cleanup or archive executed. The
worktree1392GiBcap/100GiBfree reserve and disabled automatic archive remain in force.
