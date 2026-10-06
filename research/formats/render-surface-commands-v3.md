# Native surface command stream version 3

2026-10-06. Version 3 extends the [native surface stream](render-surface-commands.md)
with owned clip state, bounded RGB565 Surface2-style copy policy and explicit
HRESULT diagnostics. Its schema and generated bindings are
`protocols/schemas/render_stream-v3.json`, `render_stream_v3.h` and
`render_stream_v3.py`. This is an offline native protocol, not an original game
ABI or a new live channel/producer version.

## Framing and ownership

The 16-byte header is `MNMCMD03`, little-endian version `3`, declared header size
`16`. Records retain the 12-byte `(operation, sequence, payload_length)` prefix.
Sequences start at one and are contiguous. Integers are little-endian; rectangle
coordinates use signed 32-bit two's-complement bytes with explicit host conversion.
There are no host pointers or shared C++ layouts. Generated `initial_header()`
constructs the header; the renderer decoder validates an entire variable-length
stream, rather than the generated fixed-header-file helper.

An offline writer exclusively owns its file; publication of that file is the
caller's responsibility. Incremental private fragments use the existing decoder
budgets (64 KiB appends, bounded retained record bytes); no new IPC publication,
cancellation, acknowledgement or live capability negotiation is implied. Bounded
replay retains the 64 MiB/4,096-record cap. Explicit streaming decoder mode retains
monotonic creation IDs, finite sequence/byte lifetime and resource budgets.

The native consumer owns created surfaces and their copied clip descriptors.
No shared COM clipper identities, HWNDs or raw pointer aliases are serialized.
Retiring a surface retires its clip state. END still requires all surfaces and
palettes retired and at least one presentation. Abort/admission/comparison failures
release session-owned resources, preserving unrelated renderer surfaces.

## Additional records

| Opcode | Payload | Admission and effect |
| --- | --- | --- |
| 16 SET_CLIPPER | `surface_id, mode, count`, then `count` RECTs | RGB565 destination, mode 0 detached / 1 explicit list / 2 attached missing list. Modes 0/2 require count zero; mode 1 with zero count means empty exclusion. At most 32 positive, in-bounds, disjoint regions. Supplied order is preserved. Validate before replacing owned state. |
| 17 SURFACE_COPY | 13 words: `source_id, destination_id, api, flags, busy_observations, source_RECT[4], destination_RECT[4]` | Live RGB565 surfaces; same-ID opaque copies freeze the source separately for each ordered clip piece on the GPU. API 0 Blt / 1 BltFast. Positive Blt geometry is one-to-one; invalid rectangles are legitimate semantic inputs. BltFast consumes destination left/top; right/bottom remain serialized input metadata. |
| 18 CHECK_SURFACE_RESULT | `surface_copy_sequence, expected_hresult` | Names the latest opcode 17, including one that failed. Explicit diagnostics only; it never determines copy admission or supplies pixels. Verify mode compares the actual result; Skip mode retains structural validation and skips the comparison. |

Clipper payload size is exactly `12 + count*16`; copy is exactly 52 bytes; result
check is exactly 8 bytes. Unknown modes/APIs, excess counts, overlapping/outside
regions, invalid IDs/formats, wrong sequences and unvalidated flag combinations
are rejected. Same-ID copies admit only zero or the selected API WAIT flag. Separate source/destination IDs retain their existing selected flags. The selected flag values are zero, the selected API's WAIT flag,
its missing-source-key flag, or `0x80000000`; broader key/effect combinations are
outside the captured policy. Busy observation bit 0 means source busy, bit 1
means destination busy; other bits are rejected. These are explicit caller/fixture
admission observations, **not** native Lock/Unlock or loss/restoration support.
The current injected producers do not emit any of these v3 records.

## Results, partial errors and compatibility

Opcode 17 returns the [bounded native backend policy](../runtime/native-surface-clipping.md).
An observed HRESULT error is a completed operation: retain any earlier clip-region
writes, record the HRESULT and continue the stream. It does not trigger consumer
failure or roll back pixels. Diagnostic CHECK records can therefore inspect a
partial failure. Host/GL exceptions and structural/diagnostic failures terminate
the session and clean up its owned resources; a failed offline replay publishes no
native output file.

Native COPY opcode 3 remains the already-in-bounds primitive and does not consult
clip descriptors. Only opcode 17 applies the Surface2-style policy. Other v2
records, including session-qualified palettes, retain their payload/ownership
rules in v3. Version 1 inline palette opcode 4 remains restricted to v1.
V1/v2 stream bytes and admission are unchanged, and their streams reject the new
opcodes. Older readers reject the new header; the new decoder accepts v1/v2/v3
with matching magic/version and rejects version changes. Live v3 negotiation and
original producer region observations remain pending.

## Validation

[Fresh offline integration](../runtime/native-clipping-stream-v3-final-20261006.json)
replays all 408 retained independent Wine Surface2 cases in one bounded stream:
408 actual HRESULT checks, 816 source/destination native pixel checks, including
two partial-write failures. CREATE records contain only the independently captured
before pixels; after pixels and HRESULTs occur only in subsequent diagnostic records.
The same stream passes uneven-fragment GPU replay in Skip mode, with zero ordinary
native/RGBA readbacks and complete resource retirement. The C++ failure/abort tests
retain an unrelated surface and verify ownership cleanup. An indexed palette v3
fixture checks v2 inheritance with independently specified RGBA bytes.

Sixteen invalid/mismatch streams are rejected, including malformed regions,
unknown IDs/APIs/busy bits, unvalidated flags, sequence/result errors, deliberately
incorrect pixels/HRESULTs, v1/v2 new-opcode attempts, truncation and trailing data.
Legacy command/consumer/palette regressions and protocol generation checks also
pass. These are native offline/headless results; no Wine/game/media execution,
Windows-driver equivalence, live v3 producer or replacement is claimed.

```sh
cmake -S renderer -B working/build/renderer -DBUILD_TESTING=ON
cmake --build working/build/renderer
xvfb-run -a python3 tools/check-native-clipping-stream.py
```

The tool writes a new per-run report or an explicit new `--report`, and is
registered as `opengl-clipping-stream` in CTest. No original media is required.
