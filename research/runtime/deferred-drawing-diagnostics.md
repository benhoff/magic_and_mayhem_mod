# Deferred drawing diagnostics

`NP.deferred-drawing-diagnostics` is an intentional native policy. Continuous
sessions copy bounded owned diagnostic records while a separate process-lifetime
worker performs file I/O. Drawing admission still has a 50 ms deadline; original
calls run exactly once, unobserved writes still invalidate history, and recovery
still refuses incomplete pixel checkpoints.

## Confirmed mechanism and remaining uncertainty

The previous logger performs CreateFileA/write_all synchronously inside drawing
and tracker diagnostics, including successful contended admission. A controlled
writer barrier reproduces lost drawing history with that logger: two missed
tracking boundaries. The new logger preserves the 300 by 128 native mirror with
zero misses and zero file operations by a drawing/tracker owner while its writer
is stalled. The previous-native control is commit
`e529af2f41707247b70b01040e5ceb52e49f69ef`, header SHA-256
`e21d9134b6f2d0a9d7624a017a2ea0b52ee05aae6a249a1c84ccf8740959b5a0`.
This is native source comparison, not original binary equivalence.

It establishes a preventable contention mechanism, not the proven cause of the
reported first-victory failure. The 65 ms wait and 16 ms sampled owner age still
do not explain the entire wait. No failed-launch log directory/save was supplied.
A real first victory, Realm/Character progression and post-victory save/load
remain unvalidated.

## Implementation and lifetime

Producers publish up to 128 immutable records with an owned 40-byte reason and
nineteen values. Nonwaiting CAS admission, duplicate suppression, four records
per reason and LastError preservation remain bounded. The writer runs separately
from drawing, publication and administrative recovery; it owns no original,
tracker or borrowed pixel pointers.

Startup occurs outside DllMain. Creation failure retains bounded records without
callback-side I/O fallback; write failure disables optional delivery. Original
ExitProcess forwarding attempts a join capped at 50 ms. Current drawing/tracker
owners and concurrent/self joins are refused. A timeout retains handle/static
storage, and successful joins are idempotent. The adapter remains pinned;
DllMain detach only signals stop. Finite captures retain their synchronous,
nonwaiting concurrent-record refusal policy through the drain.

## Validation

[`deferred-diagnostics-final-controls-20261013.json`](deferred-diagnostics-final-controls-20261013.json),
`NP.deferred-drawing-diagnostics.final-controls-20261013`, retains the prospective
scope/source declaration, ten current controlled 32-bit cases and fourteen
current actual synthetic PE32/Wine/Qt cases. Sources remain stable. The controlled
cases cover original-once/result/error preservation, stalled I/O, owned names,
reentry/join refusal, bounded records, creation/write failure, join retry and
idempotence, and paired finite synchronous/concurrent behavior.

Both PE32 builds compile. Final Wine/Qt checks cover Blt/BltFast interleaving and
actual admission timeout; finite layout refusal; orderly/guarded/alias recovery;
explicit/automatic/held-owner/refused-join exit; complete static, incomplete and
failed-claimed checkpoints. Independent frame hashes, call counts, frozen retired
streams and expected refusal assertions are preserved. Incomplete checkpoints
use fresh observations rather than invented pixels. No original artifacts were
consumed. Current support remains partial native implementation; no original
comparison, real-game integration or live replacement status is promoted.

The bounded fixture previously depended on an 8 ms pause before intentionally
invalidating the next layout. It could fail before Qt captured its valid prefix.
A fixture-only first-frame marker now synchronizes that mutation with independent
Qt capture; the corrected bounded test passes without relaxing its oracle or
changing original callback deadlines. The runner can reuse an explicitly selected
initialized private test prefix under this checkout's working/tests. That prefix
belongs to the invocation for its wineserver shutdown.

Earlier execution records and source hashes remain unchanged: the initial eight
cases plus thirteen controls, expanded declaration closure, and ten finite-policy
cases have separate evidence IDs. Cold-prefix initialization failures, negative
finite attempts and the former free-space refusal remain in their original logs
and records. A mistaken main-checkout fixture edit was removed specifically and
moved into this isolated branch; other main work was preserved. Storage policy
3f15be2c keeps 100 GiB free without a fixed working-size cap. Final accounting uses
temporary Git snapshots on the separate memory filesystem to preserve disk space.
