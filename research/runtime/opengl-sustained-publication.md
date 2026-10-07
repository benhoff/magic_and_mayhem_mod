# Sustained publication and bounded unlock partitions

## Evidence and measured cost

The optimized consumer and current producer at `2f95b25` complete the original
campaign route through Main, RegionEntry and World, deliberate failed-reader
recovery, independent Select/Summon Zombie OCR and original control count 1/15.
The diagnostic baseline then observes 60 seconds after the summon. It remains
Active, with 849 additional presentations over 71.519 seconds since World
recovery, 1,167 total native frames at consumer termination, no ordinary GPU
readbacks or viewport uploads, and no unplanned recovery. This is a healthy
bounded run, not evidence of overflow or proof of a frame-rate bottleneck.

The recovered World session publishes 571,708,670 bytes through consumer
termination. Its independently bounded archive stops at 66,857,310 bytes/1,068
records; 63,817,138 UPDATE pixel bytes represent only 18,748,044 bytes of changed
pixel storage. That is 70.6% unchanged pixel storage within the emitted UPDATE
rectangles. The replay analysis counts all native storage bits; it is not an
independent original driver comparison. Existing one-envelope dirty compaction
still spans distant changes and sends the untouched gaps between them. This is
the measured bandwidth cost addressed here; the run does not establish CPU,
disk, GPU, original-driver or game-pacing attribution.

## Final measured result

The final split-plus-weighted-consumer run requires every comparison to match
(`--require-route-pixels --require-world-summon --world-seconds 60`). All27
independent menu, guidance, summon/control-count, terrain and portrait comparisons
match. The original tutorial summon is independently confirmed; both original
and native remain active throughout the observation, with only the three
intentional reader-failure recoveries. The consumer ends at1,139 total frames,
72,019 World-session commands and zero terminal GPU resources. Ordinary native
and RGBA readbacks and viewport CPU uploads remain zero.

| Measurement | Baseline | Final |
| --- | ---: | ---: |
| World observation since recovered status | 71.519 s | 71.005 s |
| Additional native presentations in that interval | 849 | 837 |
| Recovered World-session wire bytes through reader finish | 571,708,670 | 220,296,440 |
| Stable region comparisons matched | 27/27 | 27/27 |

Recovered-session traffic falls61.47%; observed presentation rates are about
11.87 and11.79 frames/second. These are coarse single-run measurements with
asynchronous samples, initial session checkpoint/brief pre-status traffic and
terminal reader cancellation included in the session byte totals. They establish
bandwidth reduction at similar observed pacing, not a frame-rate increase or
controlled CPU/GPU performance comparison. Startup screenshots/timings vary.
The final RegionEntry title/difficulty samples have published==ACK at14,483,148
and15,001,804 bytes and already match the original; forced RegionEntry recovery
is subsequently still exercised as a separate test. The earlier split-only
mismatch/full-ring evidence remains preserved.

Fresh regression evidence includes10 continuous/archive cases with492 complete
GPU frame comparisons,26 packed unlock cases with55 frames,8 backpressure cases,
70 native-byte tiled presentations,18 literal consumer framebuffer comparisons
and5,948 cases in each native/sanitized partition reconstruction run. The
production and SELFTEST PE32 builds and optimized native targets pass. Root and
isolated immutable original manifests verify before/after experiments. The legacy
fixture corrections and intermediate aborted build/regression attempts remain
under working logs; they are not current passing execution evidence.

The final private snapshot also includes the independently committed
`b48b2a9` owned-surface work; its exact committed receipts were reviewed alongside
`2f95b25`. This does not qualify that entire independent subsystem or pending
viewport/resource/scene workspace changes. Fresh evidence is tied to the actual
staged source snapshot. Baseline and split-only reports retain their earlier
source hashes and are intentionally historical/stale after this change.

Evidence records: [baseline](opengl-sustained-publication-baseline-20261006.json),
[wire cost](opengl-sustained-publication-cost-20261006.json),
[split-only delay](opengl-sustained-publication-split-delay-20261006.json),
[final game](opengl-sustained-publication-game-20261006.json),
[partition reconstruction](opengl-sustained-publication-native-20261006.json),
[consumer work budget](opengl-sustained-publication-consumer-20261006.json),
[continuous fixture](opengl-sustained-publication-continuous-20261006.json),
[packed unlocks](opengl-sustained-publication-unlocks-20261006.json) and
[backpressure](opengl-sustained-publication-pressure-20261006.json).

## Intentional native policy

For successful full writable Unlock of an existing continuous resource with an
owned identical-layout baseline, partition the tight dirty envelope into a fixed
4-by-8 grid. Each nonempty cell emits its own tight dirty rectangle only when the
sum of pixel bytes plus 32-byte UPDATE record envelopes is strictly smaller than
one UPDATE. At most 32 nonoverlapping updates precede that unlock's CHECK/PRESENT;
dense changes fall back to one UPDATE, unchanged input still preserves PRESENT,
and differences in unused RGB32 bits remain native differences. Initial CREATE,
changed layouts, finite observation sessions and admitted partial locks retain
their prior policy.

There is no new wire operation, ID, session or changed original drawing order.
The producer still commits only successful original operations. One scratch
rectangle is allocated, budgeted and freed at a time; the quarantined baseline,
pending after pixels and scratch stay within the existing shared 64MiB budget.
Queue capacity remains 32MiB, ring capacity 1MiB, CPU surface entries 128 and
resident consumer surfaces 32. Transport refusal during a split leaves no final
PRESENT and retains explicit failed-session recovery; archive refusal remains
independent. The finite sequence and 32-bit byte lifetime still apply. A smaller
wire stream delays that lifetime ceiling but does not remove it.

The implementation scans the image for the whole envelope and then scans that
envelope again for cell bounds. This trades additional bounded CPU scans and up
to 32 commands for fewer wire bytes. The cost heuristic includes UPDATE record
bytes, not allocator cost, fragmentation, GPU work or time. Polling fairness,
latency and CPU cost across other scenes/drivers remain unqualified.

## Consumer work budget and intermediate delay

The first split-only live run remains healthy in World and reduces recovered
session traffic to222,037,462 bytes, but its RegionEntry title/difficulty checks
still show the prior Main image across three attempts until deliberate recovery.
Sampled publish minus ACK stays at1,045,092 bytes during those phases, almost a
full ring. Native progress is about6 frames/second through that transition. This
is a measured publication/consumer delay, not a passing menu pixel comparison;
that intermediate report is retained with all25/27 matched regions and the two
misses. Original and native screenshots establish the delayed screen. The source
and sampled cursors support the inference that many split UPDATE records saturate
the previous32-command GUI budget; they do not establish independent CPU time.

The consumer now keeps the finite caller byte budget and32 full-cost operations
per poll. In a v2 streaming channel only, UPDATEs of <=16,384 pixels cost one
eighth of an ordinary command, with at most256 such small uploads per poll.
CREATE, copies, larger UPDATEs, other operations and all v1 commands retain full
cost. Mixed batches stop before the next command exceeds the256-unit work budget;
already decoded commands drain in order, with END only after retained work drains.
This also preserves the distinction between owned-copy ACK and GPU submission.
There is no larger queue, ring or decoded-storage budget. More small uploads may
increase poll wall time; the policy is a finite work bound, not a hard time bound.
Two literal v2 fixtures independently compare every displayed pixel, require
exact249/32 first-poll command counts for small-update/copy cases, ACK ahead of
pending submission, no early PRESENT/END and ordered second-poll cleanup. Existing
v1 first-poll32-command and byte-budget/checkpoint tests remain.

## Validation boundaries

Native and Address/UndefinedBehaviorSanitizer execution each reconstruct 5,948
independent cases across 8/16/24/32-bit storage, including odd/tiny dimensions,
nonoverlap, cost selection, unchanged input, dense fallback, distant changes,
every partition cell and unused bits. The real PE32 fixture includes a 512x512
padded, poisoned RGB32 borrow with distant/per-cell changes, unchanged locks and
dense replacement. Independent fixture pixels validate every PRESENT through
OpenGL; plain row replay additionally checks all native bytes at 70 presentations.
The legacy packed fixture now explicitly calls RenderShutdown before ExitProcess;
its independent RGB565 GPU expectation follows the already committed bit
replication display policy, separately from floor-scaled legacy frame-v1 bytes.
These fixture corrections do not change producer or renderer pixel semantics.
Continuous/archive, packed unlock and backpressure matrices are recorded under
new evidence IDs, preserving old reports and their source fingerprints.

The live route uses opt-in `MNM_RENDER_ORDERED_COPIES=1`, continuous production,
explicit palette resources and bounded session archive with the original Wine
window retained. It does not replace original drawing. Independent stable X11
regions verify selected menu/UI, terrain and portrait pixels with the harness's
existing tolerance. Whole animated frames, extended interaction/battle scenes,
movies, hardware drivers, default activation and live replacement remain pending.
The route ends by cancelling the native reader and killing the isolated original
process; it is not clean original-game shutdown validation.

`tools/analyze-render-update-cost.py ARCHIVE --output NEW_REPORT` reproducibly
measures UPDATE redundancy from a <=64MiB/4096-record owned archive. Analysis
records reference the preserved archive path/hash. The input archive may stop in
a diagnostic GAP. `tools/test-live-render-routes.py` records sampled ring session,
published/ACK cursors, state/reason/cancel at each phase; these are asynchronous
header observations, not linearizable queue snapshots or GPU completion claims.
The producer's private queue peak in a healthy session is not measured here.
