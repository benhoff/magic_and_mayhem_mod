# Qt input polling state, version 1

`MNMINK01` is a new bridge protocol, not an original game format. Producer:
`apps/qt-shell/input_state.cpp`. PE32 consumer: `runtime/render/input_polling.h`.
All fields are little-endian uint32 except the eight-byte magic. File size is
exactly 1088 bytes. Each OpenGL shell launch creates an exclusive file beside
its frame stream; `--input PATH` passes it to the disposable runner as
`MNM_RENDER_INPUT`.

| Offset | Meaning |
|---|---|
| 0 | `MNMINK01` |
| 8 | Version 1 |
| 12 | Total byte count 1088 |
| 16 | Sequence: odd while Qt writes, even when stable |
| 20 | Active: matching target, visible viewport and viewport keyboard focus |
| 24, 28 | Cursor x/y in displayed game-image/client coordinates |
| 32, 36 | Game-image width/height |
| 40–63 | Reserved, zero |
| 64 | 256 virtual-key words, four bytes each |

Each key word has bit 31 set while down. Bits 0–30 are a rising-edge generation,
starting at zero and wrapping modulo 2^31. Auto-repeat does not increment it.
Release and focus loss clear the down bit but retain the generation. This lets
the reader detect a press followed by release between polls. Mouse left/right/
middle use Windows VK 1/2/4. Common letters, digits, navigation, F1–F24,
punctuation and aggregate Shift/Ctrl/Alt are mapped from Qt events.

Qt publishes with aligned atomic word stores and a sequence guard, including a
50 ms heartbeat while the launch is active. The PE32 reader rejects odd/changed
sequences, inactive snapshots, dimensions outside 1–2048 or coordinates outside
the image. It measures sequence freshness locally using GetTickCount and falls
back after 500 ms without a new sequence. On first observation a previously
stale file can receive this initial 500 ms grace period; this is not a durable
input log or a timestamp-synchronized protocol.

GetAsyncKeyState consumes each key's generation once per bridge process, returns
bit 15 for down and bit 0 for a newly observed press. GetKeyState returns the
down bit. Unmapped keys, including side-specific modifiers and Caps/Num/Scroll
Lock, remain with Windows. Cursor
coordinates are converted through ClientToScreen on the application-observed
cooperative HWND. Rejected/unavailable snapshots call the original API.
The bridge is read-only with respect to this file. Normal shutdown publishes
inactive state; an interrupted writer is rejected by its sequence or lease.

See [input integration and evidence](../runtime/qt-input-forwarding.md).
