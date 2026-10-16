# Native menu/loading source chunk draining

The retained `world-canvas-prefix-budget-refusal-20261015.json` reached131072
records before its next World. Runtime `emit_raw` terminates at this finite V3
limit; `continuous_next` previously reset storage only after an actual World
return. This is a confirmed capture lifetime defect, not an original game limit.

The V6 synchronous public resident policy adds source-only chunks between closed
World queues. At16384 records or4MiB the next emission first publishes the owned
prefix, waits for checked native application/acknowledgement, retires per-chunk
payload references and resets the same World slot. World ordinals, original
return/import credit and ordinary World history guards do not advance. Each
chunk has its own global nonzero ordinal, World generation, checksum and immutable
session nonce. V3/V4/V5 reject V6. World queues and deferred overlap bounds remain
unchanged. Queue/failure records in a source-only chunk refuse terminally.

This is an intentional native transport policy, not recovered original allocation
or scheduling equivalence. GPU/CPU history and resource lifetimes continue in the
same native producer instance across chunks. Acknowledgement follows application;
no original pixels or original pointer values seed the renderer. The producer
retains the emission lock through bounded acknowledgement to serialize background
writers. Deadline/cancellation and malformed metadata refuse before storage reuse.

The [first component controls](source-prefix-controls-20261015.json) passed with
115 compiler inputs declared and stable. They apply17 ordered chunks/278528 source
records before a real World whose pixels match independent CPU replay, exercise32
V6 chunk leases and strict malformed/stale/nonce/checksum/immutability/cancellation
and prior-peer refusals, and retain the existing21-queue lifecycle,512 resident,
prepared/retained and V3/V4 channel/ACK regressions. Source-only records grant no
World completion credit; resources all retire. The producer itself is built PE32;
this component result does not execute its original-game hook.

The [first90-second characterization](source-prefix-first-characterization-20261015.json)
completes original Save/Load, both display rebuilds, Main Load and normal Quit.
It drains1048576 records in64 chunks without a World between them, then finishes
353 balanced completions/imports/returns. It remains **failed** because the final
source-stability check detects the corrected test fixture made during execution.
Original artifacts verify after the experiment and private Wine stops. No old
source hash or report is refreshed, and no current validation is claimed from it.
A new settled-source run is required. Prolonged-dwell functional acceptance uses
an explicitly declared1FPS session floor because World updates intentionally stop
while menus draw; the ordinary repeat retains5FPS. These are functional checks,
not a new performance guarantee.

See [V6 byte layout and ownership](../formats/world-continuous-v6.md).
 Independent/asynchronous V5 and
finite compressed diagnostic capture retain their existing envelopes and bounds;
they do not silently claim source-only support. Other HRESULTs, arbitrary resource
churn, full original-pixel/simulation equivalence and consistent performance remain
separate. Source-only draining does not raise a World queue's existing bound.

## GPU-current reader boundary exposed by first chunk

The [next settled-source run](source-prefix-reader-refusal-20261015.json) refuses
its first source chunk at World166 during the in-game Load menu. All declared
sources remain stable, the immutable original manifest verifies and private Wine
stops. The retained slot is SOURCE_PROCESSING with16384 records; no chunk was
acknowledged. The error is `Native producer reads undefined canvas pixels`.
Its exact failing record/state is not in that historical diagnostic. New errors
include source sequence/kind/canvas/caller so later failures remain attributable.

Code review independently confirms that glyph, fade, RGB-add and blended,
displacement/projected-shadow preflights still call the CPU-only `known` helper.
It rejects both unknown0 and GPU-current2. Panels already use `requireCpu`, which
serializes publication and a native GPU snapshot, copies only established history
and leaves unknown pixels unknown. The other source readers now use the same
preflight route, preserving complete read/geometry validation before draw writes.
This repairs a confirmed component defect; the historical generic failure alone
does not prove which reader or validity state caused it.

Reader controls exercise nine families against independent CPU pixels after a
changed native GPU background, require exactly one snapshot per reader, and
refuse partial unknown history. These explicit reader snapshots remain separately
counted; ordinary World completion still returns no pixels. First component/live
records retain their original hashes and are historical after this extension.

## Retained navigation detour after reader correction

The [reader-corrected run](source-prefix-preferences-detour-20261015.json) drains
1949696 source records in119 acknowledged chunks with48 balanced Worlds, one
explicit producer CPU snapshot and no native renderer error. It remains failed:
automation expects Save entry, but the original owner reaches Preferences10 while
the host still has only Region Enter request3/action25. No native Mini Save request
was published. The [closed-channel witness](source-prefix-preferences-witness-20261015.json)
records those identities. The exact input/navigation cause is unclassified; do
not treat this as successful Save/Load or infer that relaxing history is required.
Sources stay stable, original artifacts verify and private Wine stops at the
bounded300-second flow deadline. Later reports must independently complete the
requested journey. Earlier component results remain historical after the reader
extension; the [new116-input controls](source-prefix-reader-controls-20261015.json)
pass all chunk, reader, lifecycle and prior-envelope regressions.

## Current source-bound functional acceptance

The [fresh complete journey](source-prefix-functional-live-20261015.json) passes
with unchanged declared sources/staged inputs, original manifests before/after,
private Wine stopped and original exit0. Original Save/Load and both display
rebuilds complete, Main Load is held60.151 seconds, and704512 source records drain
through43 acknowledged chunks during one World interval. The maximum chunk is
16384 records, source and channel capacities are unchanged, and original World
completion credit does not advance during the hold. Main Load then resumes on
canvas90 after canvas2, with332 matching completions/imports/returns, fresh ordered
checkpoints and normal Quit. Optional native calls total287968; forwards/history
refusals, ordinary World/native/RGBA readbacks and pending ownership are zero.
Two explicit source-reader GPU snapshots are separately counted.

Two marked NS_RECT failures are successfully consumed as scoped no-write records
in this accepted live journey, closing the preceding accepted-live branch gap.
Unclassified HRESULTs and general original-driver behavior remain separate.
The declared session floor is1FPS and measured aggregate is2.9998FPS across the
intentional pause; no reliable performance claim is made. The nine reader-family
branches have independent component controls; this live report does not count
every family separately or establish complete presented pixels/simulation.
The earlier failed source-stability, reader and navigation reports stay failed.

The [ordinary repeat](source-prefix-ordinary-repeat-20261015.json) also passes:
440 matching completions/imports/returns, canvas2→90, one16384-record chunk, two
accepted marked copy refusals and18887 optional native calls with zero forwards/
history refusal, source/input stability, verified originals and normal cleanup.
Its declared5FPS functional floor passes at10.3764 aggregate FPS. This remains a
single ordinary timing result, separate from the deliberate pause and retained
older10FPS failure. The repeat's single chunk does not independently demonstrate
beyond131072 records; the preceding60-second stress does.

```sh
python3 tools/working-retention.py run --reserve-gib 3 -- \
  python3 -B tools/test-native-canvas-transition.py
python3 tools/working-retention.py run --reserve-gib 5 -- \
  python3 -B tools/test-native-campaign.py --recreate-world --difficulty 1 \
  --resident-world --other-pixel-producers --legacy-audio --skip-movies \
  --loading-menu-seconds 60 --min-fps 1 --timeout 480 \
  --source-game working/tests/pixel-staging/run-qeshupqr/source-game
# Repeat without --loading-menu-seconds and with --min-fps 5 for ordinary flow.
```

Current scope is synchronous public resident V6 without finite compressed
profile capture. World/deferred bounds,512 initialization witnesses, resource
capacity, arbitrary churn, other maps/HRESULTs, original menu-navigation
reliability, hardware and whole-session original pixels/simulation remain open.
The Preferences detour is retained as a separate navigation gap. No original
behavior comparison or live replacement claim is added.
