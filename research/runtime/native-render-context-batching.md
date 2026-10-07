# Bounded OpenGL context batching and private state cache

2026-10-07. Native submission policy, separate from recovered game behavior.

`GlBlitter::batch` retains its private context across synchronous renderer
operations, permits nesting and restores the previous context and surface on
success or exception. Its callback must not dispatch Qt events, invoke presentation
callbacks or modify foreign GL state. Standalone operations retain their previous
context-restoration behavior. `CommandConsumer::submit` batches consecutive
non-PRESENT commands within its existing bounded submission; each PRESENT executes
outside that scope so callbacks can synchronously paint or fail in the caller's
context. No command order, work budget, admission, producer ACK or GPU lease/fence
changes. There is no rollback of preceding accepted commands.

The private context retains its VAO/FBO, constant raster state and pack/unpack
alignment. Attachment and viewport changes are cached; texture deletion invalidates
the attachment cache, including GPU lease destruction and scratch texture cleanup.
Shader uniform locations are resolved at construction, fixed samplers are set
once, and program/scissor changes are emitted only when required. These caches
never describe the Qt viewport context. All core pixel/key/clip/palette conversion
and ordered overlap paths remain in place.

Release builds defer ordinary GL error queries to the outermost batch return.
Unbatched operations, resource allocation, explicit readback and Debug builds
retain immediate checks. Framebuffer completeness is checked when its attachment
changes. A driver failure can therefore be reported at a Release batch boundary
after earlier operations have committed; cleanup still refuses the consumer
session. Input/sequence/geometry/resource checks remain per command.

## Headless performance

The same retained 15-frame RGB565 startup prefix (3,987 commands, 3,898 uploads)
ran before and after the change on the NVIDIA TITAN RTX, OpenGL 3.3 core, driver
610.57.04. Each mode used two warmup replays and twelve retained replays. After
the initial checkpoint, serial frame completion was:

| Metric | Before | After |
| --- | ---: | ---: |
| Median completion | 10.785 ms | 0.774 ms |
| p95 completion | 18.031 ms | 1.161 ms |
| Maximum completion | 24.781 ms | 2.432 ms |
| Queued completed-output throughput | 96.9 frames/sec | 1,061.6 frames/sec |

This is about 13.9 times lower median service time and 11.0 times higher
throughput for this workload. Both modes/drivers produce the same final RGBA
hash as the historical baseline, and NVIDIA/Mesa independently match the golden
keyed-copy/update RGB565 fixture. Commands, uploads and copies are unchanged; no
work is skipped. Mesa llvmpipe median completion changes from 8.821 to 8.618 ms
and throughput from 110.0 to 131.8 frames/sec. Host load is uncontrolled; these
measurements establish the combined change, not the contribution of each cache.
CPU samples show context switching no longer dominates this headless replay; the
faster process increases the relative weight of initialization and cleanup in
the full-process CPU profile. Inclusive caller percentages overlap.

The [execution record](native-render-batching-profile-20261007.json) retains
before/after raw timings, source and executable fingerprints and CPU summaries.
Input is exactly the previous [headless baseline](native-render-headless-profile-20261007.json)'s
compressed captured prefix; the new replay consumes no original media. The
[profile tool](../../tools/native-render-profile/README.md) reproduces the current
renderer with the same timing policy. Serial queries/fences include host feeding
and cross-context sampling; queued throughput drains at replay end. Neither
metric measures live game FPS, queue age or input-to-screen latency. EGL device
context costs do not establish desktop GLX costs. Battles, other hardware,
compositor/vsync/scanout and GPU loss/recovery remain unvalidated.

Original comparison evidence that fingerprints shared renderer sources remains
historical. Regression checks only validate their stated retained fixtures; this
policy does not promote whole-engine equivalence or live replacement.

## Regression validation

The [final regression record](native-render-batching-regressions-20261007.json)
passes all sixteen Release renderer CTests and the Debug incremental-consumer
test on Mesa/Xvfb. Release and Debug each cover 32 partitioned sessions, 354
independent complete-frame comparisons and 30 existing failure cases. New checks
verify nested context retention, caller context/framebuffer restoration, absent
context restoration, partial commit after rejection, and a caught injected GL
error at the batch boundary. Both ordinary native readbacks and viewport CPU
image uploads remain zero. The timer-driven Qt replay service also passes Skip,
Verify, poisoned CHECK and GAP cases.

The suite retains its offline surface/key/overlap/clip/palette/ownership/format
fixtures and existing wire tests; no original executable is run. The Debug
check keeps per-operation diagnostics active; the Release check exercises the
deferred driver-error boundary. No sanitizer run is claimed by this result.

Reproduce with:

```sh
cmake -S renderer -B working/build/render-batching -DCMAKE_BUILD_TYPE=Release
cmake --build working/build/render-batching --parallel 4
ctest --test-dir working/build/render-batching --output-on-failure
cmake -S renderer -B working/build/render-batching-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/render-batching-debug --target render-command-consumer-test --parallel 4
ctest --test-dir working/build/render-batching-debug -R opengl-incremental-consumer --output-on-failure
python3 tools/test-incremental-render-commands.py working/build/render-batching
```

The production Release `mnm-qt-shell` is rebuilt with the changed renderer. Its
separate [CLI execution record](native-render-batching-qt-cli-20261007.json)
records Skip and Verify synthetic replay smoke checks. Hardware desktop/widget
behavior and physical input-to-screen latency remain separate milestones.

The [fresh hosting guards](native-render-batching-hosting-20261007.json) also pass
against the rebuilt renderer: 70 changing PRESENTs in three bounded polls with
zero synchronous reader paints, input delivery between polls, complete final-frame
drain beyond 4,096 records, windowed/fullscreen high DPI and terminal GAP
direct-input fallback. This renews the existing synthetic responsiveness and
fallback policies whose older records include shared renderer hashes; old records
remain immutable. Reproduce under isolated Xvfb with
`python3 tools/test-native-command-responsiveness.py working/build/qt-shell`.
