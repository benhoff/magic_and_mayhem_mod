# Persistent bounded surface command execution

Native policy, 2026-10-05. `CommandConsumer` keeps session-local surface IDs,
palettes and native handles across decoded-command batches. It uses the existing
version-1 command format and the direct GPU texture lease. This is native preview
integration, separate from live PE32 transport or original rendering replacement.

## Admission and lifecycle

Complete-file decoding and progressive execution share `detail::CommandState`.
Decoded commands retain their original sequence number. Both paths check sequence,
unique/nonzero IDs, stale resources, pixel formats, image/expected byte dimensions,
update/copy bounds, key/palette/swap admission and END conditions. Limits remain
64 MiB encoded-equivalent bytes, 4096 records, 64 live surfaces, 16,777,216 native
pixels and 2048x2048 per surface, across all batches in one session. The public
consumer does not trust a forged decoded structure simply because it has a
`SurfaceCommand` type. No incremental byte decoder or channel ABI is introduced.

Batch boundaries are not completion boundaries. Each accepted command commits
immediately; there is no batch rollback and previously delivered frames are not
retracted if a later command fails. Empty batches are allowed while active.
END requires explicit destruction of every owned surface and a prior PRESENT.
`finish()` confirms END; producer disconnection without END fails and cleans up.
`abort()` and active destruction release owned surfaces and mark interruption.
Admission, rendering or callback failure terminates the session as failed and
cleans up its owned surfaces. Unrelated renderer surfaces are not destroyed.
Closed consumers refuse new submissions; create a new consumer for a new session.
Recursive submissions/completion/abort from callbacks refuse. All execution and
teardown must stay on the renderer's GUI thread; the renderer outlives the consumer.

A retained GpuFrame can outlive consumer cleanup. The caller owns presentation
of that last frame and should clear it when a failed/incomplete session is no
longer appropriate. Producer/consumer texture synchronization remains the direct
presentation contract; no new texture or surface ownership scheme is inferred.
The bounded session quotas are not a claim of indefinite continuous gameplay.

## Explicit diagnostics

Ordinary GPU execution defaults to `CommandDiagnostics::Skip`. CHECK/CHECK_RGBA
records still require correct IDs and lengths, but are counted as skipped and
cause no pixel readback. Skipping does not assert pixel agreement. Verify mode
reads and compares independently generated native/RGBA output; expected bytes
are never rendering inputs. PRESENT itself does not read back in GPU mode.
Offline image export is explicit and retains the last native bytes and QImage.
Existing replay/export wrappers use Verify mode to preserve comparison behavior.

Results report successful commands, presentations, performed/skipped comparisons,
owned live surfaces/pixels and a snapshot of renderer-wide counters. Renderer
counters may include other clients of a supplied renderer; owned counts are
specific to this consumer. Failures/interruption do not masquerade as END.

## Qt service integration

The shell delegates `--commands FILE` to `command_replay.cpp`, which admits the
complete bounded file before replay, then feeds up to 32 decoded commands per
Qt timer event into one retained consumer. Surface state survives event-loop
returns. Each PRESENT uses the existing shared-texture viewport; the last frame
survives END. The timer is an offline replay policy, not recovered engine cadence.
Closing during replay aborts outstanding work through consumer destruction.

`--command-checks` enables diagnostic Verify mode. Normal replay reports skipped
checks and zero PRESENT/diagnostic readbacks. Smoke mode separately computes a
verified image reference and reads the displayed framebuffer, outside ordinary
execution. The standalone test shell compiles the same replay service; optional
production-shell checks exercise the actual CLI dispatch.

## Reproduction and evidence

```sh
cmake -S renderer -B working/build/incremental-renderer -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/incremental-renderer --parallel 4
ctest --test-dir working/build/incremental-renderer --output-on-failure
python3 tools/test-incremental-render-commands.py working/build/incremental-renderer \
  --sanitized-build working/build/incremental-renderer-sanitized
```

The sanitizer build uses C++ `-fsanitize=address,undefined
-fno-omit-frame-pointer -fno-pie` and executable linker
`-fsanitize=address,undefined -no-pie`; driver/Qt cache leak detection is disabled.
Every runner creates a new report/log directory under
`working/tests/incremental-render-commands`; no original artifacts are consumed.

Independent CPU native-color and composition fixtures cover all four formats,
updates, opaque/keyed copies, palette edits, swaps and new IDs after destruction.
Whole, one-at-a-time, fixed-seven and varied batches are compared in both Verify
and Skip modes. Each batch checks resource/counter state; each PRESENT compares
the complete Qt framebuffer. Failure checks cover bad IDs/sequences/geometry,
formats/pixels/checks, GAP, callback/reentrancy/null/trailing failures, missing END,
abort/destructor cleanup, fresh sessions, retained interrupted frames and quotas.
Qt service checks span four timer events and explicitly distinguish performed
and skipped comparisons, including poisoned/GAP refusal.

Confidence and status are limited to fresh synthetic native integration on
Mesa/Xvfb. Live transport, byte-fragment framing, channel overflow/resynchronization,
original command coverage/replacement, physical-GPU recovery/performance and
longer session policy remain separate. Historical direct-presentation and scene
reports retain their exact original hashes; shared-source changes can leave them
stale. The new companion evidence records current source and executable hashes.

The frozen [execution record](opengl-incremental-commands.json) passes 64 partitioned
sessions, 708 full-frame comparisons and 60 failure cases across normal and
ASan/UBSan builds. Ordinary consumer readbacks and viewport image uploads are
both zero. The same production Qt CLI passes Skip and Verify smoke runs; 40
PRESENT records span four timer events. Existing direct GPU presentation tests
also pass at ordinary and 2x display scale. No new original/live claim follows.

After fragmented decoder extraction, a separate [v2 native result](opengl-incremental-commands-v2.json)
reruns all 64 normal/sanitizer sessions, 708 full-frame comparisons, 60 failure
cases and production Qt Skip/Verify CLI checks against current sources. The old
record remains immutable; live transport evidence is separately scoped.
