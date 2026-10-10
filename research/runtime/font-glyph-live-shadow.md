# Glyph-bearing live native shadow admission

This bounded increment exercises the existing canvas producer pipeline with the
current atomic glyph raster implementation. `0x581ec0` is already observed by
`runtime/scene/canvas_producers.c/.S`; adding a second hook would introduce an
unnecessary conflict. The observer forwards original drawing and snapshots only
glyph source bytes, tint, format/masks and initialized coverage values. Native
destination storage comes from earlier owned source operations. Original completed
canvas files are comparison oracles read externally by the capture runner.

`tools/test-font-glyph-live-shadow.py` builds the native live consumer, runs a
private Xvfb/Wine Quick Battle with four startup World returns, requires positive
glyph traffic and complete original/native CPU canvas comparisons, and verifies
each retained GPU mirror. Prospective claims bind the scoped behavior and scenario
before execution; compiler dependencies, observer/preparation source paths and
manifest checks remain part of the result. All binary preparation uses the existing
hash/signature-checked scripts on a disposable installation.

```sh
python3 tools/draft-coverage-claims.py --behavior NR.font-glyph-live-shadow \
  --scenario font-glyph-live-shadow-20261010 \
  --output working/tests/font-glyph-live-claims-next.json
python3 tools/test-font-glyph-live-shadow.py \
  --claims working/tests/font-glyph-live-claims-next.json
```

Recorded execution: `working/tests/font-glyph-live-shadow/run-wbgjwym3/`, retained
in [comparison report](font-glyph-live-shadow-comparison-20261010.json); its child
capture is `working/experiments/scene-observer/run-0dhv0b9y/`. The current native
consumer admitted 1,935 tinted glyph requests through four World returns. All
941 completed canvases matched the separate original oracles: 416,635,312 pixels,
zero mismatches, every retained GPU mirror equal, no producer failure or original
work bypass. Source/input fingerprints and both immutable manifest checks passed.
Confidence is high within this bounded startup scenario. This provides current
glyph-bearing shadow evidence without promoting broader historical producer
contracts or claiming a complete rendering replacement.

This compares completed glyph-bearing producer histories,
not each individual live glyph entry. The isolated raster and byte-consumer
comparisons supply their separate finer-grained evidence. The recovered byte
state adapter is not installed into the live game. Original rendering remains
active; text takeover, state lifecycle/reset, higher string layout, broader
resolutions, prolonged gameplay and presentation-driver equivalence are pending.
