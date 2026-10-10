# Native World resource reuse

This intentional native policy keeps raster order, original guards, V3 wire
contracts, CPU reference composition and GPU readback comparison unchanged.
The recovered raster contracts remain separately accounted for.

`SnapshotResources` verifies a chosen decoded visual once per immutable resident
resource revision. Encoded identity and palette ambiguity checks still run on
every lookup. Loading, reloading or adopting a resource assigns a new revision;
failed visual checks never cache that revision. Native World completion now uses
the existing bounded indexed atlas, including its palette updates, word frames,
shadow shapes and eviction rules. One outer GL batch retains the context during
synchronous queue completion, preserving each draw's order.

Focused tests cover warm reuse, reload verification, repeated refusal of changed
pixels, restoration, indexed palette ambiguity and context restoration. A frozen
prechange consumer and the current consumer replay the same owned-source capture
without reading original oracle pixels. Live guarded startup execution and exact
original raster/AX comparison are separate milestones. Results are pending until
recorded below. Debug/software OpenGL measurements do not establish hardware or
unbounded interactive performance.

Frozen owned-source replay passed all1,062 checkpoint comparisons and five native
checks. Warm queues2..16 medians: resource preparation310.87→137.93ms, GPU
submission268.58→67.88ms, GPU readback1.28→2.01ms. Preparation plus submission
fell64.5% in this diagnostic run. The16 queues used252 atlas uploads,22,430 cache
hits and no evictions;247 visual checks served22,183 reuses. The last queue used
zero uploads/rechecks,1,402 hits/reuses and two atlas surfaces. These single-run
Debug/softwareGL timings include ordinary host load and carry no real-time claim.
The immutable [replay record](native-world-resource-reuse-replay-20261009.json)
retains prospective claims, dependency hashes, assets, binaries and raw reports.

The baseline source/binary is retained under
`working/tests/world-resource-reuse-baseline-20261009`. The
[prechange source patch](native-world-resource-reuse-prechange.patch) reverses
only the seven affected native/test files. Apply it to an isolated copy of the
current source closure and verify every baseline source hash from the replay
record before rebuilding `mnm-canvas-producers-live` as Debug. The benchmark
accepts that frozen baseline (`baseline.json` with its `sources` map and the
binary under `build/`), plus a closed capture report. It copies only owned producer
inputs and required assets, excludes original oracle/reply pixels, and compares
all outputs. Optional `--prepared` reuse checks every compiled dependency against
the current prospective declaration before accepting an existing optimized build.

Fresh public execution completed16 guarded queues and1,058 matching checkpoints.
The [live record](native-world-resource-reuse-live-20261009.json) records275 atlas
uploads,18,462 cache hits,274 visual checks,18,188 visual reuses and no evictions.
Warm medians were108.51ms preparation,69.53ms submission,2.99ms readback and
143.38ms independent CPU composition. Native entry-to-return was533.44ms, also
including publication, validation, checkpoints and ingestion. This is a different
startup capture from the frozen A/B corpus; compare those timings only within
their recorded domains. The live route remains bounded to the first16 queues;
manual menu traversal, all maps and continuous gameplay remain outstanding.

The [independent original comparison](native-world-resource-reuse-original-20261009.json)
matched all23,843 admitted precise raster canvases and low16AX returns,16 final
GPU/live World canvases and1,058 checkpoints. Five native tests passed in its
frozen build; all80 compiler dependencies were declared, and sources/inputs and
original manifests remained stable. This fresh result supports the existing
recovered batch-return and intentional native transport scopes. The resource-reuse
policy retains comparison/replacement `none` and separate live-observation support.
Older shared-source evidence remains historical; this does not renew other scenes,
manual/cancellation scenarios or unrelated native contracts.
