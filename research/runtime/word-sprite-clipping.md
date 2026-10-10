# Direct-word clipping comparison

This increment compares the existing native GPU sprite clipping operation with
unmodified original entries `0x596cb8` and `0x597086` in the pinned No-CD build.
It tests pixels only. Original scratch state, argument mutation, registers and
return flags still require separate recovery before clipped live admission.

The corpus covers fifteen placements across 64 deterministic positive direct-word
frames: interior, exact edges, partial edges, two corners and fully hidden sprites.
It includes signed origins, opaque zero, transparent/opaque/mixed masks, odd and
even runs, padded destinations and two WORD alignments. Native drawing consumes
encoded input and the initial canvas; it never reads expected after pixels.
Each original process checks destination canaries. Clip globals are zero.
Nonzero clip globals, smaller viewports, auxiliary planes, empty/malformed/indexed
frames, general installed assets and live replacement remain outside this scope.

Reproduce with a prospective declaration for `RS.word-clipping`:

```sh
python3 tools/draft-coverage-claims.py --behavior RS.word-clipping --output working/tests/word-clipping-claims-new.json
python3 tools/test-word-clipping.py --claims working/tests/word-clipping-claims-new.json
```

The runner uses a fresh Debug build, independent expanded-mask expectations,
both original backends and the existing native GPU `drawClipped` operation.
Software Mesa execution does not validate a physical driver or live bypass.

## Recorded result — 2026-10-10

[Independent comparison](word-sprite-clipping-complete-deps-20261010.json) passed
960 GPU placements and 1,920 original-backend executions over 447,330 complete
canvas pixels per path. All fifteen placement groups contain 64 frames. Original
destination canaries, borrowed row padding, opaque zero and fully hidden no-ops
pass. Source hashes remained stable and both immutable-input checks passed.
The driver was Mesa llvmpipe 26.2.1; this is software GPU-driver evidence.

For this corpus, both selected original backends match the half-open intersection
of the decoded sprite footprint and `[0,width) × [0,height)`. Exact right/bottom
edges render normally; the live MVP still refuses them because original branch
selection and scratch-state writes differ from its admitted branch. This result
does not establish complete clipping behavior or authorize broader live admission.

The first [harness run](word-sprite-clipping-harness-failure-20261010.json) stopped
before GPU drawing because the test used the incremental upload constructor
without completing `advanceUpload`. Its immutable failed report remains retained.
The corrected rerun completed upload before drawing; runtime renderer code was
unchanged. Workspace hashes in the corpus are diagnostics, not workspace
equivalence checks. Next recover the clipped branch's required scratch/argument
writes and ABI before implementing a guarded live adapter. Other clip globals
and viewport origins also need separate fixtures.

The reviewed source tree is commit `3931268` plus this test and accounting
increment. Concurrent uncommitted World-throughput edits were excluded; a later
renderer edit needs a fresh run before claiming current-code equivalence.

The earlier successful [pixel run](word-sprite-clipping-comparison-20261010.json)
retains its original fingerprints. Compiler dependency review then found
`protocols/include/mnm/render_stream_v3.h` missing from that source map. A new
prospective declaration and complete rerun add that dependency and explicitly
check every compiled project input against the recorded source map. The newest
result supports current scoped comparison; the earlier run stays historical.
Intermediate indexing snapshots remain under `working/tests/`; only the final
reviewed census is admitted to the durable register.

### Caller-state follow-up

[RS.word-backend-state](word-backend-state.md) recovers and tests clipped
workspace/argument effects and original ABI in 4,448 fixtures, plus production
entry assembly with a host admission stand-in. It fixes two existing unclipped
compatibility issues. Clipped requests still forward to original drawing; native
CPU clipping and full Win32 hook/exception behavior remain separate milestones.
