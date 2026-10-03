# Bounded surface-command sessions, version 1

This is a new instrumentation protocol, not an original game asset. Producer:
`runtime/render/surface_commands.h`. Consumer: `renderer/commands.cpp`.
All integers and native pixel values are unsigned little-endian. Coordinates
are nonnegative; rectangle right/bottom bounds are exclusive. Rows are tightly
packed in logical top-to-bottom order.

The 16-byte header contains `MNMCMD01`, uint32 version 1, uint32 header size 16.
Each record has three uint32 fields: opcode, sequence (starting at 1 with no
gaps), payload byte count. Payload follows immediately, without alignment.

| Opcode | Payload |
|---|---|
| 1 CREATE | uint32 ID, width, height, bits, red/green/blue masks; full native pixels |
| 2 UPDATE | uint32 ID, x, y, width, height; patch native pixels |
| 3 COPY | Ten uint32: source ID, destination ID, source left/top/right/bottom, destination x/y, key-enabled (0/1), exact native key |
| 4 PALETTE | uint32 ID, first index, count; count RGB triples (no flags or alpha) |
| 5 CHECK | uint32 ID; full expected native pixels, used only for comparison |
| 6 PRESENT | uint32 ID; resolve native pixels/palette into a Qt RGBA image |
| 7 DESTROY | uint32 ID |
| 11 SWAP | Two uint32 IDs; exchange native pixel storage, keep palettes attached to their IDs |
| 10 CHECK_RGBA | uint32 ID; full expected RGBA8888 pixels, comparison only |
| 8 END | Empty payload; all surfaces must have been destroyed and at least one PRESENT recorded |

IDs are nonzero session-local identifiers, not interface addresses or persistent
COM identities. Reusing an ID within a session, even after destruction, is an
error. The consumer maps them to its own process-local OpenGL surface handles.
Create initializes every pixel. Indexed surfaces initially have opaque black
palettes; explicit palette records replace entries. RGB surfaces require valid
nonoverlapping contiguous masks with at most eight bits per channel. Formats
are 8-bit indexed and 16/24/32-bit RGB. Unkeyed COPY must have key value zero.

The parser validates the complete session before initializing OpenGL. Bounds:
64 MiB per file, 4096 records, 64 live surfaces, 16,777,216 live pixels, and
2048x2048 per surface. Updates/copies must be entirely in bounds. Copies require
identical formats and distinct IDs. Unknown opcodes, invalid lengths, missing
END, trailing records/bytes, sequence gaps, stale IDs and invalid palettes fail.
Clipping, stretching, overlapping self-copies, fills, longer flip chains, effects and format
conversion are not represented in this version. No unsupported call is silently
translated into an apparently complete live stream.

CHECK reads the generated OpenGL pixels and compares exact native bytes; it
never uploads expected output. Mismatch exits with status 2 and writes no output
or preview. PRESENT captures the independently generated image. Offline replay
exports the last presented surface's native bytes and optional PNG, even though
all live surfaces are destroyed by END. Successful replay returns 0, errors 2.
Outputs use exclusive creation; existing files are never overwritten.

## Current bridge producer scope

With `--capture-draws`, a successfully written `blit-0001.bin` also produces
`commands-0001.bin` beside it. Existing snapshots supply CREATE for source ID 1
and destination ID 2, optional palettes, COPY, CHECK against original destination
after, PRESENT, both DESTROY records and END. It adds no observer locks or COM
calls. Both files are created exclusively; an interrupted command write cannot
produce a valid completed session. Source remains bounded to 256x256 by the
existing capture guard, regardless of the broader consumer limit.

These CREATE/DESTROY records delimit replay checkpoints. They do **not** assert
that the game created/released those COM objects at that point. UPDATE and
incremental palette records are supported and synthetically tested by the
consumer; the checkpoint producer does not emit them. The opt-in history producer below
additionally records native CPU writes and lifetime observations; indexed palette
mutation tracking is described in the bounded history section below. The file describes one accepted draw,
not every call preceding it or a full frame. Unsupported calls remain visible
only in the separate bounded `events.bin` inventory.

## Bounded history producer and GAP

`--capture-history` enables `runtime/render/surface_history.h`, producing
`history-0001.bin` separately from the checkpoint file. It uses the same version
1 header/records. Opcode **9 GAP** has one uint32 reason and always makes replay
fail (there is no recovery/END after it):

| Reason | Meaning |
|---|---|
| 1 | Overlapping/reentrant observed calls or busy termination |
| 2 | Recorder capacity/allocation limit |
| 3 | Unsupported/unbalanced application lock, changed lock layout, or locked termination/Release |
| 4 | Uncovered canonical identity or interface alias |
| 5 | Unsupported/missing palette, palette readback failure, or unexplained color change |
| 6 | Unsupported/failed draw coverage, readback, flip, restore, GDI/DC, clipping or palette operation |

CHECK mismatches also fail replay even if the file contains a valid END.
Earlier consumers reject GAP as an unknown opcode; current consumers report its
reason. The history begins at the first eligible indexed/RGB draw, lazily seeds later
surfaces from draw inputs, and emits full-surface CPU UPDATEs after successful
writable Unlocks. Subsequent copies check native input/output instead of
reuploading snapshots. Final COM Release emits DESTROY; bounded-stop/detach
cleanup may also destroy replay resources without claiming COM destruction.
The 16-operation recorder limit is stricter than the general consumer limits.
See [coverage and evidence](../runtime/opengl-surface-history.md).


CHECK_RGBA (opcode 10) is a uint32 live surface ID followed by width × height × 4
RGBA bytes in logical row order. The parser checks the exact payload length and
lifetime before starting GL. Replay resolves the surface's current native pixels
and palette/masks, compares all RGBA bytes, and reports `color_checks`. Expected
colors are never uploads. Earlier consumers reject opcode 10 as unsupported;
current writers retain the version 1 envelope with this additional opcode.

Indexed histories initialize palettes from original GetPalette/GetEntries,
fan out shared SetEntries ranges as PALETTE records, and emit full PALETTE on
successful surface reassignment. Palette changes can emit PRESENT without COPY;
they leave native indices intact. Per-entry flags are excluded, alpha stays 255.
Null detachment and unsupported palette capabilities invalidate history.
[Palette evidence and limits](../runtime/opengl-indexed-palettes.md).

SWAP (opcode 11) requires distinct live IDs with identical dimensions, bit depth
and RGB masks. It exchanges native textures without uploading pixels or drawing
a copy. Palettes, resource IDs and presentation textures stay attached to the
logical surfaces. Later CHECK/PRESENT resolves the exchanged storage. Unsupported
older consumers reject this new opcode. The envelope remains version 1.

Within an already seeded history, the bridge now emits SWAP for verified
primary/front + single backbuffer pairs. It checks both native inputs before
Flip and both original outputs afterward, then presents the front. Successful
unsupported flips still emit GAP 6; failed flips emit no SWAP. Discovery is
repeated per call and does not retain COM references across application calls.
See [flip scope and evidence](../runtime/opengl-double-buffer-flips.md).
