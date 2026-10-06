# Tutorial summon observation and pending copy ordering

This extends the guided idle World test to a real original tutorial action. It
is observation, not replacement or full-frame/gameplay equivalence. The original
NoCD build and return PCs remain the pinned inventory's build-specific findings.

`tools/test-live-render-routes.py --require-world-summon` requires continued native
Active publication and stable-region agreement after the original summon.
`--observe-world-summon` instead records an explicitly classified successful-copy
conflict and terminal native fallback when it occurs. Both require initially
Active World publication, forced-reader recovery, and continuing original execution.
The diagnostic option does not accept an Active pixel mismatch or arbitrary failure.

The test independently reads the original window with finite Tesseract attempts:
Select Zombie Spell; left click `(523,575)`; Summon Zombie; original creature count
`0/15`; right click `(400,280)`; original count `1/15`. The right mouse button is
required by this tutorial. Original screenshots and OCR crops are retained under
the report's working run. Stable native instruction/count, terrain and portrait
regions are independent unsynchronized comparisons; moving actors are not asserted.

Reproduce with an optimized native build and the existing Wine/Xvfb prerequisites,
plus Tesseract:

```sh
cmake -S renderer -B working/build/renderer-live -DCMAKE_BUILD_TYPE=Release
cmake --build working/build/renderer-live --target live-render-route-probe live-render-channel-test -j2
xvfb-run -a -s '-screen 0 1800x1000x24' python3 tools/test-live-render-routes.py working/build/renderer-live --mode campaign --observe-world-summon --world-seconds 15
```

## Native observation policy

Repeated validated descriptors with identical known layout, masks, caps, backbuffer
count and source key preserve the producer's generation. GetSurfaceDesc is an
observation; it must not invent a write during a pending copy. Changed descriptors
still advance generation and uncertain layouts still invalidate pixels. Successful
SetColorKey/SetClipper and real pixel mutations retain their existing guards.
No wire revision, original operation arguments, resource limits or recovery limits
change. This is a native policy, not recovered driver equivalence.

The fresh [mutation matrix](opengl-world-summon-mutations-20261006.json) compares
411 complete independent native frames across six cases. Identical nested descriptor
observations permit 201 RGB16 frames; a changed descriptor refuses the pending copy.
A nested successful copy that overwrites the outer copy's source must also refuse
the outer operation. Existing mixed RGB16, unsupported-copy and recreation cases
retain their results. The fresh [palette/DC regression](opengl-world-summon-palette-dc-20261006.json)
compares another 334 complete frames, for 745 total across eight cases.

## Actual interleaving remains unsupported

Exploratory strict tutorial tests confirmed the original summon but sometimes
exhausted the native recovery budget. One exploratory optimized run continued to
600 frames, but source changes during observation invalidated its freshness check;
it is not current-code validation. A nonoptimized exploratory run also exhausted
queued-byte capacity. None of these justify raising limits or suppressing GAP.

The bounded `blit_commit_refused` diagnostic records 19 hexadecimal fields: target,
thread, source, prepared, before/after epoch, before/after source generation,
before/after target generation, fill, bootstrap, direct, original result,
source/target generation origin, outer caller, last source-copy caller and owner.
Origins: 1 drop, 2 initial identity, 3 descriptor key, 4 descriptor metadata,
5 clipper, 6 key setter, 7 Unlock snapshot, 8 successful copy/fill, 9 DC handoff,
10 attachment, 11 flip, 12 palette dependency, 13 lifetime backlink retirement.
Caller/owner fields describe the last successful copy; consult origin before
attributing a current generation to those fields. Logs remain bounded and add no
observer COM call or reference.

A retained source-stable exploratory trace in
`working/experiments/opengl-render/run-_c3ol9rr/lock-capture/lifecycle.log` identifies
outer return PC `0x58c05d` and last source-copy PC `0x58c9af`, both already mapped
under `RI.copies` in the rendering inventory. It shows a changed target generation
with copy origin 8 during the outer hook admission/commit interval, on the same
thread. Other attempts show changed source generations with copy origin 8.
This confirms successful copy interleaving at the hook boundary, not the driver's
exact pixel-read timing or a semantic cursor ownership mapping. The final immutable
observation below records its own outcome and conflict fields.

Pending: define ordered ownership for these original primary copies, validate
reentrant source and destination mutations, repeat active tutorial continuation,
and separately compare animated actors/effects and full World frames. Conservative
refusal remains required until those contracts are resolved.

## Final source-stable observation

The [final diagnostic record](opengl-world-summon-observation-20261006.json)
confirms the original summon, initial forced World reader recovery and 108 further
native frames before conservative fallback at frame410. The native summon count
was not compared after fallback. A prepared successful copy saw its source
generation change with origin8: outer return PC `0x58c488` on thread452,
last source-copy return PC `0x58c05d` on thread408. These return PCs are already
in `RI.copies`; the exact driver read order and higher-level caller remain unknown.
Terminal consumer resources are zero. The original
process remained alive, preferences were unchanged, source fingerprints were stable,
and all2,927 immutable files passed verification before and after the run. This is
a passing refusal observation, not a passing strict native summon test.

Committed history `057f07d..01a66c2` was separately reviewed against exact
parent/current file and behavior receipts, with no unresolved accounting gaps.
Historical execution evidence and fingerprints are preserved.
