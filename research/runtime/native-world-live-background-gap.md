# World initial-canvas boundary, 2026-10-07

Follow-up: [canvas history investigation](native-world-canvas-history.md)
confirms retained pre-consumer pixels in a separate four-frame capture and adds
a native history service with contiguous source admission. The seven-pixel case
below remains historical at its own fingerprints. Original startup producers,
buffer ownership and live delivery of complete intervening history are pending.

Confirmed execution finding: native rendering and private unmodified original
raster execution, each beginning with an independent zero canvas, agree on all
480,000 pixels of the retained failing frame. The actual live post-consumer
canvas differs at seven pixels. Original pixels were never native inputs.
Evidence: `native-world-background-comparison-20261007.json`, sourced from
`working/tests/native-world-background/run-n3ldmkvs/report.json`.

The retained frame is
`working/experiments/scene-observer/run-zbdbutg0/world-1.bin`, SHA-256
`539a4526f41a96afeb2d7f5658fa4a967bb52ddbbe0338e039c0f0d5d1a09b21`.
The mismatching coordinates are (232,466), (233,466), (232,467), (233,467),
(231,468), (232,468), (233,468). Native/private-original words are zero; live
words are nonzero. The captured terrain masks covering this area skip the
affected positions. This rules out an ordinary difference between the selected
native composition and independent original replay for this snapshot.

Hypothesis, not a confirmed lifecycle recovery: the live canvas retains earlier
pixels in holes between terrain masks. An untraced producer is another remaining
possibility. We must observe the initial canvas and its writes, ownership,
buffer identity and reuse/clear lifecycle before claiming the cause or replacing
the whole consumer. Native replay cannot be initialized from original pixels to
hide this gap.

Other retained failures show 13 pixels in `run-ov1gji5a` (no retained requests
for the first packet) and 11 pixels in `run-k5c2bctq` (requests and pixels retained;
includes 55 displacement draws). A separate earlier live run `run-zkefhztm`
matched 24 canvases/11,520,000 pixels exactly. That report predates the subsequent
batched scene change and remains historical at its original fingerprints. These
cases do not support general live World equivalence.

The current ordinary run `run-kvbyg9w8` presents 24 complete owned native World
frames with zero CPU pixel readbacks and zero viewport image uploads, drops 53
publication attempts and supersedes 23 older completed packets. It validates
continuous native presentation and bounded delivery, not original pixel
equivalence. The channel remains shadow-only and leaves original drawing active.
The default native initial background is explicit native policy. Cold admission
and asset loading still cause a roughly 1.2-second worst poll; steady drawing is
split into 32-request batches. Performance and asynchronous cold loading remain
pending. The standalone viewer also keeps menus/input in the original window.
