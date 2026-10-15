# Original-active 32 World-return source acquisition

The recorder-only mode extends the original-active source prefix from 16 to 32
World returns. It uses existing V3 owned-source definitions and exact wire-size
slicing. Original CPU checkpoints and physical planes remain independent oracles;
native raster, batch, continuous, publication and glyph replacement are refused.

The default V3 replacement budget remains 131,072 records. A recorder-only header
may explicitly declare 262,144 records. Both Python and native decoders admit
only these two V3 budgets, with the existing 32 queues and 128 MiB byte cap.
The 2,048-source/16 MiB cache falls back to raw owned input when exhausted.
The trace runner reserves up to 3 GiB for separate CPU checkpoint files.
These are diagnostic storage policies, not recovered engine limits.

## Retained failures

`draw02-trace32-original-01.json` failed before Main: the scene observer had an
independent gate requiring raster batch replacement for sample counts over 16.
The observer now admits the explicit recorder binding; the producer installer
still refuses incompatible replacement configurations.

`draw02-trace32-original-02.json` reached World but refused admission because the
131,072-record stream ended inside queue 26. The offline diagnosis in
`draw02-trace32-cap-diagnosis-01.json` records 25 completed returns, 68,652,574 wire
bytes and all 2,048 owned-source definitions. Both original-manifest checks were
retained. The separate peers retained 476 API and 475 post-EGL physical witnesses,
but those witnesses do not repair the incomplete source prefix.

A closed 32-return prefix is not 120 consecutive advancing update windows,
complete lifetimes, all display routes or a semantic journey. Source submission,
action identity and physical frame ownership remain pending; all 47 journey
requirements and 21 operation contracts keep their existing denominators.

## Step 2 closed 32-return acquisition checkpoint (2026-10-12 UTC)

`draw02-trace32-original-03.json` closes all 32 World returns with 171,339
records and 81,091,310 wire bytes. The producer completion marker, original
sections, pinned sources and original-manifest checks pass. Separate peers retain
491 API and 490 post-EGL physical witnesses (235,200,000 EGL pixels).

The first attempt failed the independent scene-observer mode gate before Main.
The second retained 25 returns before the 131,072-record ceiling truncated queue
26. Both failures and the exact cap diagnosis remain registered. Recorder-only
V3 now declares a finite 262,144-record header; replacement defaults retain
131,072. Both decoders admit only these budgets, retaining 128 MiB and 32 queues.
V3 wire-prefix slicing now follows recorded lengths rather than expanded owned
source payloads. Four Python envelope methods, five source-prediction methods,
24 native owned-adapter cases/39 refusal cases and drawing-scene regression pass.

`draw02-trace32-prediction-01.json` replays source parameters and independently
pinned encoded assets. It exits successfully and matches 108 of 109 selected API
planes exactly; the retained initial-black witness differs by 468,103 pixels.
All 382 excluded API witnesses remain explicit. Original pixels and observed
raster returns are comparison-only. This is restricted prediction, not temporal
source ownership or full parity.

Reassessment: step 2 still needs actual submission/source ownership, full
lifetimes, alternate routes and independently observed semantic outcomes. The
57 passive snapshot ticks do not satisfy 120 advancing windows. Steps 3–7 remain
ordered and pending; all 47 journey and 21 operation requirements remain open.
Committed-history review449929e..4c4762a finds23matching exact receipts and no
missing versions. Historical reports/hashes are preserved. Storage remains about
879 GiB working/447 GiB free, within the 1280/100 policy.
