# Primary presentation from game-owned RGB blits

Implemented 2026-10-03 after [offscreen propagation](opengl-game-owned-blits.md).
This connects reconstructed primary pixels to the existing Qt/OpenGL frame stream.
The game was not launched for implementation or testing.

## Confirmed implementation boundary

A successfully committed full writable Lock/Unlock checkpoint now records whether
its returned descriptor explicitly includes DDSD_CAPS (`1`) and
DDSCAPS_PRIMARYSURFACE (`0x200`). The primary bit alone, without the descriptor's
caps-valid flag, does not establish identity. New checkpoints replace this
provenance; aliases resolve through successful application QueryInterface calls.
Conflicting aliases, final Release, capture epochs and new CreateSurface objects
retain the previous conservative invalidation rules. The subsequent
[bootstrap chunk](opengl-primary-bootstrap.md) also accepts explicit application
descriptors as shape and identity provenance, without inventing initial pixels.

After a supported successful Blt/BltFast, the tracker verifies source/destination
generations, retains the reconstructed complete destination, and publishes RGBA
only if the destination has this observed primary identity. Native source-key
transparency and untouched borders are preserved before conversion. Offscreen
blits continue to produce offline evidence without live frames. Failed,
unsupported, untracked or invalidated operations publish no new frame.

The existing direct-primary Unlock publication now uses the same owned-pixel
publisher. It takes no surface locks or COM references. Original calls, HRESULT,
arguments and LastError are preserved. Publication uses the existing capture guard
and odd/even shared-memory sequence protocol: pixels and dimensions finish before
the new frame counter becomes visible to the reader. Qt receives these frames
through its existing timer and OpenGL viewport; no Wine-thread GL context is used.
The stream and snapshot formats remain unchanged.

Lifecycle diagnostics add `blit_presented` and `blit_presentation_skipped`.
The latter means a complete primary copy could not be published on this attempt,
for example because another producer held the presentation guard; it does not
change the original draw or turn an offscreen surface into a primary.

## Bounds and next work

This is still bounded capture, not uninterrupted replacement rendering. The
existing 16 Lock snapshots, 16 blit operations and memory/file budgets apply.
After a failed or unsupported draw, or after the budget expires, Qt keeps its
last complete frame; its counter does not advance. The subsequent [owned Flip chunk](opengl-owned-flips.md) handles observed
two-buffer RGB chains. Indexed palette handling and continuous frame boundaries
remain separate work.

The subsequent [bootstrap chunk](opengl-primary-bootstrap.md) initializes a
never-Locked destination from a complete opaque overwrite and observed application
metadata. Sources up to 2048x2048 are supported within the existing budgets;
800x600 initialization and incremental updates have synthetic coverage. Actual
game compatibility remains unvalidated.

## Offline validation

`tools/test-render-lock-blits.py --primary` marks a synthetic destination as
primary in the fake engine's returned Lock descriptor. The fake engine maintains
independent backing pixels and poisons its exposed lock storage during Unlock.
The fixture snapshots the frame stream after every seed and draw, records their
order and dumps native original-operation pixels independently of the tracker.
Python verifies every frame's counter, completed sequence, dimensions, pitch,
format, status and RGBA bytes against those independent pixels. Failed,
unsupported, offscreen and nested-invalidated operations must leave the previous
frame bytes and counter unchanged. Every accepted command still matches the
independent CPU oracle and native OpenGL replay.

The final shared stream is passed to the actual Qt viewport under Xvfb and Mesa
software rendering. `--stream-test` now samples source-image pixel centers across
the framebuffer and checks any letterbox bars, instead of assuming a fixed
red/green/blue/white fixture. This validates arbitrary reconstructed frame colors,
orientation, dimensions and RGB conversion in the existing presentation path.

`--primary --unknown-primary-caps` omits the returned DDSD_CAPS flag while leaving
the primary caps bit set. Propagation can still succeed, but no primary frame may
publish. Source and destination aliases, failed draw/retry, keyed copies,
reseeded pixels, subrectangles, 16/24/32-bit conversion, nested writes and the
operation limit are tested without running the game.

Confidence: confirmed scoped synthetic x86 -> owned native pixels -> RGBA stream
-> Qt/OpenGL framebuffer behavior; not a claim of real-game renderer equivalence.

Evidence: eight primary fixtures in
`working/tests/render-lock-blits/run-29ni7v3p/report.json`, plus nested update,
reseeding, budget and destination-alias fixtures in
`working/tests/render-lock-blits/run-hzjfochu/report.json` (12 distinct primary
cases). Missing caps provenance rejects publication in
`working/tests/render-lock-blits/run-5o43fs7i/report.json`; that run also checks
Qt readback of independently generated patterned 800x600 and portrait 129x257
streams. Those large images test the viewport, not large-source blit support.
Four offscreen regression cases remain unpublished in
`working/tests/render-lock-blits/run-ypv8na7t/report.json`.

All 14 Qt/renderer CTests passed, including frame-stream guards, viewport
presentation, native command replay, blit reference comparisons and persistent
surfaces. Tests use synthetic Wine surfaces and Xvfb/Mesa, not the game loop.

All 14 existing Lock/Unlock lifecycle fixtures also passed:
`working/tests/render-lock-lifecycle/run-s3wdcsxs/report.json`, including direct
primary publication, old Unlock ABI, negative pitch, retries and indexed/offscreen
nonpublication.

The existing Wine-to-Qt bridge regression passed at
`working/tests/render/run-t099o_bz/report.json`. The production DLL rebuilt as
PE32 i386; hashes are recorded in `working/build/render/manifest.json`.
