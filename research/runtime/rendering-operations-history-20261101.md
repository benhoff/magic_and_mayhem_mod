# Operations and destination-history residual review — 2026-11-01

This EP06 lane reviews retained caller evidence and destination dependencies.
It owns no live runtime slot and changes no rendering, capture or game behavior.
The other lanes own startup/watchdog repair and campaign/progression preparation.
Base: `65095aee6d6722bcd73a38cecac8b5a0fb7b25f9`.

## Results and confidence

[Offline review](rendering-operations-history-results-20261101/offline-review02.json)
runs the current metadata/pixel readers and 15 negative controls. Raw archives
and selected members are hash verified; transient decoding uses RAM. New results
are immutable SQLite evidence. Historical source hashes remain unchanged.

* **Cursor refusals explained within the retained call scope, high confidence.**
  Calls 3000 (`0x58c488`, flags `0x10`) and 3001 (`0x58cbb2`, flags `0x11`)
  are failed BltFast operations, both returning `0x88760096`, emitting no COPY.
  The first reads `[640,512,680,552]` from a 640×480 surface; the second writes
  that rectangle to a 640×480 surface. Independent signed geometry agrees with
  the recorded `blit_reject_source_rect` and `blit_reject_target_rect` hashes.
  Four cursor-client refusals are a different journal counter from these two
  copy failures. The journal closes with zero active/pending clients. Coordinate
  provenance and original-driver return equivalence remain unknown. Keep strict
  admission; a successful emitted-COPY check cannot excuse a refused caller.
* **Later clipped COPY evidence passes current offline readers, high confidence.**
  `CPUCLIP.quick02-20261026` contains 1,370 application calls: 1,204 CPU COPY,
  141 GPU COPY and 25 fills. All 1,345 emitted COPY commands bind to their callers;
  686 CPU-keyed calls are distinct application rows. Retained independent pixel
  witnesses recheck 3,487,760 words. The historical analysis reproduces exactly,
  including 88 geometrically reduced GPU calls and clean diagnostic END/lifecycle.
  The historical execution also contains 124 clipped CPU calls and one clipped
  fill. Fills have separate UPDATE semantics and receive no COPY pixel credit.
  Natural self/keyed-GPU positives and empty CPU intervals remain unobserved.
* **Late history mismatch remains reproducible in retained bytes, high confidence.**
  Every one of eight 800×600 zero/live comparisons has exactly one difference:
  `(548,433)`, native-zero 0 versus live 6338; entry target value 6372. The retained
  zero bytes match each historical native output hash. The original-only watch
  places a terrain clear at record 1236 and distortion at record 1261/backend 9.
  The selected owned row offset is 15, locating the required destination source
  at **`(563,433)`**; its frame-entry value is **6338**. This scalar locator is
  consistent with production decoder/shader indexing. It does not execute the
  raster prefix or prove sprite coverage, source survival or the earlier writer.

These are fresh reader/control executions over historical acquisitions, with
**no fresh native raster or game execution**. The old COPY inventory has 117
changed files among 4,210 declared sources; the late-history inventory has eight
among 311. Both include engine changes. Passing an offline reader renews its
bounded evidence interpretation, not engine equivalence or current-player
acceptance. The report records exact changed paths and old input hashes.

## Smallest remaining captures

1. **Cursor coordinate provenance:** in a current normal player journey, retain
   the closed v2 call census, cursor journal and complete all-COPY output binding.
   For the two selected return PCs, retain source/target generations, dimensions,
   input coordinates, flags/results and the coordinate producer's window/client
   state. Exercise resolution transition and edge/corner cursor motion. Compare
   any refusal with the same supported original-driver context. Require END,
   normal Quit and zero owners. Zero refusals in another run alone does not
   establish correctness of the retained out-of-bounds requests.
2. **Source-history provenance:** if the selected effect is reachable in the
   required current player, capture a closed producer/storage prefix from the
   actual Lock-return/owned initialization through the first failing checkpoint.
   Watch `(563,433)` immediately before record 1261 and its last earlier writer;
   also watch the output `(548,433)`. Reconstruct the prefix from native-owned
   inputs and compare every checkpoint to a separate original oracle. Never seed
   native history with original-before pixels. Use the existing
   `tools/check-first-world-history.py`/`canvas_initial_history.py` admission model.
   If this old route is outside required scope, record a reachability/requiredness
   decision with call/dispatch evidence; this review cannot infer exclusion.
3. **Only residual operation branches:** piggyback closed caller metadata on the
   next usable normal journey. Collect self/overlap, keyed GPU, stretch or an empty
   CPU clip only when naturally observed. Supplied controls stay component tests.
   Alternate storage, RGB555 and arbitrary CPU readers need explicit requiredness
   evidence; do not fabricate a branch to clear the matrix. A CPU empty interval
   still needs independent expected-piece accounting before output can be closed.

Campaign/watchdog work can proceed without this lane's code. The campaign capture
can carry item 1 and the producer journal for item 2 once its startup is viable;
no separate full campaign is needed just to repeat already checked COPY output.
The source-history watch should be targeted after reachability is established.

## Operation handoff and closure boundaries

All 18 EP06 operations remain pending acceptance in **both** `player-default`
and `player-software`: 36 obligations. This table groups shared acquisition work;
its rows are exact contract operation IDs. The three EP09 fault operations add
six separate obligations and remain with recovery work. No frozen gap, completion
contract or acceptance receipt is changed by this review.

| Operations | Evidence to reuse | Smallest remaining proof |
| --- | --- | --- |
| `copy-opaque`, `copy-keyed`, `copy-clipped` | Current offline CPUCLIP replay and historical original caller bindings | Current required configuration/caller source binding and complete output/lifecycle; do not rerun old geometry controls by default |
| `copy-stretched`, `copy-alias-overlap` | Adapter CPU/GPU controls and COPY route census | Natural caller witnesses or evidence-backed requiredness; preserve whole alias snapshots and flags/results |
| `fill-full`, `fill-clipped`, `partial-write` | Adapter controls, colour comparison and observed clipped fill | Separate UPDATE destination comparisons, dirty footprint and preserved outside pixels |
| `cpu-write-readback`, `destination-dependency` | Storage comparisons, first-World admission tools and the eight retained negative samples | Closed native initialization/writer/readback chain, including the source pixel above when applicable |
| `text-font-dc-lifetime` | Glyph original comparisons and production-route evidence | Current same-session font/DC/callback ownership plus independent output and retirement |
| `image-cropping`, `sprite-indexed`, `sprite-direct`, `sprite-auxiliary` | Existing image/word-route and World comparisons | Applicable real dispatch/clip/asset identities in required configurations; exact missing auxiliary branches |
| `palette-remap`, `palette-cycling` | PALETTE closure/modes/chains/provider comparisons | Actual selected palette versions, builder/provider bindings and output in required configurations |
| `flags-and-return-values` | Adapter/image/glyph/word controls and exact refused-call analysis | Same-call original arguments/flags/results and supported return/error policy; output pixels alone are insufficient |
| `surface-loss-restore`, `transition-recovery`, `failure-cleanup` (EP09) | Existing recovery/hand-off controls | Current fault/restart/owner lifecycle execution; outside EP06 |

Join each execution only to the operation and configuration it actually proves.
The six previously admitted default-player leaves remain their original scope.
No full software verdict, original-driver verdict or whole-player completion is
inferred here. Remaining scope conditions need engineering evidence before any
user product-scope decision is necessary.

## Reproduce

From the owned checkout, with its configured verified local evidence:

```sh
python3 -B tools/review-rendering-operations-history.py \
  --report research/runtime/rendering-operations-history-results-20261101/offline-review-NEW.json
```

Run through the machine's storage guard; select a new immutable report name.
The CLI verifies archive/member identities and refuses mixed/incomplete COPY
acquisitions. It preserves negative history findings as expected results rather
than declaring rendering success. Fifteen controls reject unsafe/duplicate archive
members, symlinks, budget overruns, bad identities, inconsistent refusal reasons,
ignored off-target pixel differences and malformed/out-of-bounds history inputs.
