# Packed ordered unlock updates

2026-10-05. Native policy `NR.owned-session-unlock-packed` addresses the
per-row UPDATE exhaustion found by the
[application DC startup observation](opengl-owned-session-dc.md).
Original drawing remains active; this is a bounded command bridge.

## Implementation

`game_session_unlock` keeps the initial complete checkpoint as CREATE. An
existing complete checkpoint or an admitted rectangle now emits exactly one
existing v1 UPDATE with destination x/y, width/height and tightly packed native
pixels. The partial base CHECK still precedes the update; the complete resulting
CHECK and any eligible PRESENT follow it.

Full-width regions already have contiguous normalized owned rows and serialize
directly without new allocation. Narrow regions copy only their admitted pixels
into temporary tight storage. The existing 64-MiB ownership limit counts the
pending after-snapshot, partial base, other owned/reserved storage and temporary
allocation together. Allocation or resource refusal stops the ordered stream;
temporary storage is released even when record or transport publication fails.
Forced scratch allocation/admission failure remains untested.

Native input still comes from the pre-original-Unlock snapshot. Original padded
or negative pitch was normalized by the existing capture code, and original
Unlock may poison the borrowed memory without changing command input. CHECK
bytes never supply render input. Existing ownership, generation/epoch, failed
Unlock retry, geometry, palette, record, byte and operation bounds remain in
place. No wire version, opcode or native GPU backend changes.

## Validation protocol

```sh
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-session-unlocks.py working/build/live-render-channel
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-owned-session.py --build working/build/fill-bootstrap
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-session-dc.py working/build/live-render-channel
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-session-fills.py working/build/live-render-channel
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-live-render-game.py working/build/live-render-channel
```

The dedicated fixture exercises DirectDraw interface generations, RGB16/24/32,
indexed palette state/change, padded and negative pitch, full repeated locks,
full-width bands, one-row and narrow nonzero-origin rectangles, chained updates,
failed Lock/Unlock retries and bounded completion. Independent fake-engine
storage is updated by the original fixture Unlock, which then poisons all exposed
bytes. Every UPDATE is compared with an independently cropped native checkpoint;
every admitted PRESENT is compared with the complete independent GPU frame.
Explicit CPU and native-byte CHECK replay also compare final native storage.

Missing base, readonly/discard locks, changed shape/argument, invalidation and
byte-budget exhaustion remain explicit refusals. The missing-base fixture never
starts an ordered session; shutdown reports interruption, not a completed empty
stream. Ordinary native/RGBA readbacks, viewport image uploads and terminal GPU
resources must remain zero. Diagnostic framebuffer/native-byte reads are explicit.

## Current execution and next boundary

The [current packed-unlock execution](opengl-owned-session-unlocks-verified.json)
passes 26 cases: 17 complete sessions and nine refusals, with 55 independent full
GPU frame comparisons. Complete cases also pass CPU and native-byte CHECK replay.
The [initial native result](opengl-owned-session-unlocks.json) retains its earlier
fingerprints; the legacy bounded-upload assertion was subsequently corrected and
the complete packed suite rerun under a new result. This preserves evidence
history without refreshing hashes.

The [legacy ordered regression](opengl-owned-session-unlock-legacy-regression.json)
passes all ten cases, including bounded completion, explicit byte-budget refusal,
restore/held ownership and indexed palette/flip behavior. Its shell/replay
executables were preserved from the earlier successful build because concurrent
menu CMake edits introduced duplicate unrelated targets. This validates the
producer/replay behavior, not those concurrent build changes. The old case label
`record-limit` now reaches the byte limit with packed rectangles; it does not
force the record cap. Both renderer CTests pass. Current DC and fill regressions
pass 18/eight cases (38/17 GPU frame comparisons), and the seven-session live
transport regression passes with normal GPU execution and the sanitized C writer.
The GPU consumer sanitizer build was not rerun.

The [original startup observation](opengl-real-game-shadow-unlocks.json) passes
immutable-input verification before and after execution. It now publishes only
39 commands (six CREATE, 14 UPDATE, 18 CHECK, one BLIT), well below the 4,062-record
admission limit that previously stopped startup. Published bytes are 24,594,964.

It reaches the separate 16-operation cutoff before any eligible primary PRESENT.
`session_finish_owned` refuses with mirror GAP reason 6 because no surface has
been presented; mapped-channel failure remains GAP reason 2. Original drawing
continues, but the bounded native session remains incomplete with zero native
presentations. Packing removes the record-exhaustion blocker; it does not prove
an original-game native frame, independent driver-pixel equivalence, continuous
sessions or rendering replacement.

Next: define a bounded startup completion policy that allows the first eligible
primary PRESENT while retaining independent command, byte, surface and ownership
limits. Validate this policy separately before rerunning original startup. Historical evidence
retains its hashes; changed shared headers leave older validation stale unless
separately rerun under a new evidence ID.

## Continuous delta increment

The [continuous owned unlock delta policy](opengl-owned-unlock-deltas.md) retains
matching baselines privately during full writable locks and publishes only the
changed storage-pixel envelope after a successful Unlock. Unchanged unlocks retain
PRESENT and ownership/counter handling. The bounded policy and partial regions
above remain unchanged; historical evidence retains its original source hashes.
