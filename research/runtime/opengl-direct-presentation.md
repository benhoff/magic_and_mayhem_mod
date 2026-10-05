# Direct GPU command-replay presentation

Native policy, reviewed 2026-10-05. This connects bounded command replay to a
Qt viewport without the PRESENT operation reading pixels to CPU memory. It does
not route live engine operations, suppress original drawing or establish a
physical-GPU performance result.

## Ownership and synchronization

`GlBlitter` accepts an explicit GUI-thread sharing context and verifies sharing
before allocating resources. `presentGpu` resolves native pixels/palettes through
the same integer conversion shader as image export, into an owned RGBA8UI texture.
`GpuFrame` is a GUI-thread lease of that texture and its producer context. A
surface retains only a weak lease: the displayed frame survives surface deletion
and renderer destruction, without a surface/context ownership cycle. While a
lease is retained, another presentation of that surface reuses and updates it;
leases represent the latest presentation, not immutable captured frames.

The producer flushes a completion fence. A sharing consumer waits on that fence,
samples with an integer texture shader and flushes a sampling-completion fence.
Before overwriting the reused texture, the producer waits for consumer sampling.
Only one serialized GUI-thread consumer is admitted per lease; a second live
consumer context is refused. Unshared or absent current
contexts refuse access; source-handle rejection remains the existing policy.
Renderer teardown releases its native surfaces even while leases retain the
context. Textures/fences are released in the retained producer context when the final
lease expires. Qt viewport destruction releases only its own upload texture;
renderer textures remain owned by the lease. Recreated, unshared viewport
contexts require a new renderer/frame binding; stale texture names are not used.

The viewport retains the existing aspect ratio, nearest-neighbor sampling,
orientation, input coordinate mapping and CPU-image path. Resizing changes the
viewport without uploading source pixels. GPU frames have no QImage backing.
A reported sharing error can be recovered by supplying a newly compatible frame.

## Command replay and scope

`replayCommandsGpu` executes a decoded complete bounded stream in a supplied
renderer, sends each PRESENT lease to a callback and preserves explicit CHECK
and CHECK_RGBA diagnostics. PRESENT has no native or RGBA readback. The existing
file/PNG exporter retains image readback. Callback failure cleans up replay-owned
surfaces, while a retained frame can still outlive those surfaces.

The shell's `--commands FILE` path creates the renderer sharing the initialized
viewport, displays each PRESENT synchronously and retains the last frame after
END/source destruction. Smoke mode separately renders an image reference and
reads the Qt framebuffer; these are diagnostic operations, outside ordinary
GPU presentation. No incremental transport/consumer or live game pipeline is
introduced by this milestone.

## Reproduction and evidence

Build `mnm-qt-shell`, `gpu-presentation-test` and renderer regression targets from
`apps/qt-shell`. Run:

```sh
python3 tools/test-gpu-presentation.py working/build/qt-shell \
  --sanitized-build working/build/qt-shell-gpu-sanitized
```

The optional sanitizer build uses `-fsanitize=address,undefined
-fno-omit-frame-pointer -fno-pie` and executable linker flags
`-fsanitize=address,undefined -no-pie`. Its GPU test uses leak detection disabled
for Qt/driver process caches. All fixtures are generated; original artifacts are
not consumed. Every run creates new logs/report under `working/tests/gpu-presentation`.

The test independently expands native colors and compares complete displayed
framebuffers for indexed8 and masked16/24/32 pixels, keyed/opaque copies,
palette edits, swaps, texture reuse, aspect-ratio resizing and CPU-path switching.
It checks source/renderer destruction, unrelated-context refusal, empty frames,
callback-failure cleanup and missing callbacks. Normal and high-DPI runs execute
in both ordinary and ASan/UBSan builds. The runner also exercises shell command
replay and refuses an incomplete GAP stream.

The immutable final companion `opengl-direct-presentation-v2.json` pins accepted results,
source/executable hashes and per-run logs. Confidence is high within these
synthetic native-integration checks on Mesa/Xvfb. Original scene equivalence,
continuous live transport, hardware-driver compatibility, context-loss recovery
and sustained performance remain separate work. Historical rendering evidence
retains its original fingerprints; shared-source changes may make it stale.

The initial `opengl-direct-presentation.json` remains historical. V2 reruns the
complete suite after the final teardown/single-consumer guards; its 416 frames
match with zero presentation readbacks/uploads. Last-lease texture deletion and
still-live native surface teardown are included. Ten existing renderer/export/Qt
viewport/input regressions pass after the final source changes.
