# Owned drawing history and physical native presentation

This native test/application policy implements the DRAW-02 `session.json`
backend ABI. It does not recover new engine behavior or acquire original frames.
Run `tools/replay-drawing-presented-session.py OWNED_DIRECTORY NEW_OUTPUT_DIRECTORY`
with `MNM_DRAW_COMPOSITION` pointing to `mnm-drawing-session-replay` and
`MNM_DRAW_PRESENTER` pointing to `mnm-drawing-present`. Both binaries are built
from `apps/drawing-session/CMakeLists.txt`.

The declaration admits exactly the existing schema/session/input/action/frame/
effect fields. One `owned-state` input is a JSON plan with exactly:

```json
{
  "schema": 1,
  "kind": "mnm-drawing-history-v1",
  "session": "1",
  "assets": {"owned.bmp": "asset-input-id"},
  "segments": [{
    "stage": "menu",
    "producer": "producer-input-id",
    "frames": [{
      "frame_index": 0,
      "boundary": {"sequence": 3, "queues": 0, "canvas": 7, "width": 800, "height": 600}
    }]
  }]
}
```

The decimal session token preserves the complete unsigned64-bit identity.
Producer bindings have `operation-input` role and source-owned producer bytes;
asset bindings have `encoded-asset` role. Each generic frame names the state,
its producer and every declared asset. All bindings must be used, contained,
hash-checked and within aggregate/per-file budgets. Segment order, source/frame
mapping, extents, action boundaries and typed identities are checked. The existing
native composition runner additionally validates the producer protocol, complete
terminal boundaries, asset catalogue and actual drawing admission. Its16-queue
limit remains explicit; this adapter does not widen producer coverage.
Recovered asset requests retain their exact logical names; physical paths use
canonical separators and case. SPR catalogue names are additionally bound to the
same declared encoded bytes, so case aliases cannot create ambiguous directories
or unbound resources. Canonical path collisions and transport-name collisions
refuse admission.

The backend creates a private composition directory, executes the native renderer
and sends only its derived RGB565 planes to an application presentation host.
The host displays GPU leases through the existing sharp `GlViewport` at one
physical pixel per source pixel. It waits for a buffer swap and reads the X11
screen through `QScreen::grabWindow`, rather than reading the widget framebuffer
or treating GPU readiness as presentation. Every complete physical RGBA plane
must exactly match the renderer's RGB565 colour expansion. Output RGBA frames
come directly from the screen; RGB565 output quantizes that same observed plane
after exact RGBA equality. Original oracles and expected effects remain outside
both native processes. The viewport must report zero application QImage uploads
and both children must release all native surfaces.
The physical screen must contain every complete frame; valid-size padded grabs
outside its bounds cannot be admitted. Private tests explicitly request a2048x2048
Xvfb screen instead of relying on this system's640x480default.

The fixed `frame-000000.bin` output and `session-report.json` follow the independent
session comparator ABI. Actions bind captured history; replay does not execute
simulation/input actions or assert their semantics. Nonempty caller-effect
requests return unsupported64 until actual native effect contracts exist. Movie
stages and other unknown protocols likewise refuse. Success on synthetic histories
or historical original primary-plane references does not establish independent
original physical presentation, active battle rendering, the fourteen complete
journeys, real driver recovery or DRAW-02 completion. Those gates retain their
original denominator and require new appropriate execution evidence.

The optional `--display-conversion normalized-nearest` policy expands native-derived RGB565 words by integer nearest normalization into native RGB32 storage before GPU presentation. Default `bit-replicated` presentation remains available. Both fixed policies are independently checked across all 65,536 RGB565 words. Policy selection supplies no original output or measured colour table; exact full RGBA equality remains required.
