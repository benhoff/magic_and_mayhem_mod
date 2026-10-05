# Owned constant-fill command routing

2026-10-05. Native policy `NR.owned-session-fill` removes the startup fill
refusal observed in the [first real-game shadow run](opengl-real-game-shadow.md).
Original drawing, results and LastError preservation remain in the existing
hook/tracker path. This does not replace original rendering.

## Implementation

`game_session_blit_begin` now gives an admitted fill only a destination identity.
An initial full-surface fill creates synthetic zero storage; it does not emit a
before-CHECK pretending those zeros came from the driver. Subsequent fills retain
and check the existing destination representation before the update.

After a successful original call and existing epoch/generation checks,
`game_session_blit_end` emits one existing v1 UPDATE for the fill rectangle.
Its tightly packed native pixels are a rectangle-sized prefix of the constant
buffer that `game_fill_before` constructed from the captured `dwFillColor`.
Every pixel in that buffer has the same native value, so partial rectangles need
no row-pitch copy or new allocation. CHECK output is never rendering input.
There is no synthetic source-surface identity, new opcode or wire-version change.

The native renderer executes its existing admitted UPDATE, then CHECK and any
eligible PRESENT. This uploads native fill pixels to the GPU; it does not add
a GPU clear shader or eliminate producer pixel copies. Existing geometry,
format, clipper, flags, palette, ownership, record/byte/pixel and completion
limits still apply. Failed original fills emit no update. Initial partial fills
cannot establish a complete surface; unsupported cases still refuse.

## Validation

```sh
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-session-fills.py working/build/live-render-channel
python3 tools/test-render-bootstrap.py --build working/build/fill-bootstrap \
  --case fill-session --case fill-update --case fill24 --case fill32 \
  --case fill-failed --case fill-partial
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-live-render-game.py working/build/live-render-channel
```

The [synthetic execution record](opengl-owned-session-fills.json) covers eight
PE32/Wine sessions: indexed8, RGB16/24/32, failed fill/retry, rejected initial
partial fill, later nonzero-origin rectangular fill and the prior fill-session
regression. Every PRESENT is compared with independently maintained original
fixture pixels. Explicit GPU native-byte CHECK replay also compares the final
native surface, including unused high bits in RGB32. The original fixture
deliberately mutates its fill-effects argument after consuming it.

The live default consumer has zero ordinary native/RGBA readbacks, zero viewport
image uploads and zero retained surfaces at END. Diagnostic native-byte replay
and framebuffer capture are explicit test reads. The existing incremental
consumer and live-channel CTests also pass. Synthetic evidence uses no original
game artifacts and does not establish original-driver pixel equivalence.

## Real-game result and next boundary

The [new original startup observation](opengl-real-game-shadow-fills.json)
publishes fill CREATE/UPDATE/CHECK records instead of failing before the stream
starts. The original main menu remains visible and owned 800x600 frames continue.
All 2,927 original files verify unchanged before and after the experiment.

The session still terminates before any PRESENT with file GAP reason 3 (tracked
surface invalidation); the separate mapped-channel reason is GAP (2).
The trace logs the gap immediately before `dc_acquired`. In the existing
`game_dc_acquired` path, acquiring an application DC invalidates the session
surface before that diagnostic is logged. This identifies the next ownership
boundary; it does not recover the game's font rasterizer or prove an independent
pixel comparison. ReleaseDC can restore an owned snapshot, but this terminal
ordered session has no DC checkpoint/resume route.

Next: account for the bounded application GetDC/ReleaseDC lifecycle and its
successful owned checkpoint in the ordered stream, with explicit failure and
ownership tests. Complete native startup presentation, independent driver-pixel
comparison, gameplay coverage, continuous sessions and replacement remain pending.
Historical evidence records retain their hashes; shared hook/fixture edits leave
affected older results stale.
