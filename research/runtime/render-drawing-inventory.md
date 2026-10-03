# Drawing inventory and bounded replay

Recorded 2026-10-03. This is the next step after
[Qt/OpenGL frame presentation](opengl-presentation.md): identify a drawing
operation, retain its inputs and original output, and compare an independent
reference implementation. The engine still draws through DirectDraw. No game
session was launched during this work.

## Static evidence: high confidence for this executable

Pinned no-CD working executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
preferred image base `0x00400000`. Reproduce with:

```bash
./tools/export-render-support.py
```

The exporter verifies the hash before creating output, exports selected assembly
windows, finds direct call sites with nearby instructions, and hashes artifacts.
Addresses below refer only to this build. Initial output:
`working/decompiled/render-support-kwx3j0_o/`, including monitor API string bytes.
No function boundaries are inferred from a direct call's
surrounding instructions.

| Wrapper/site | Confirmed observation |
|---|---|
| `0x58a9b0`, `0x58aa90` | Fullscreen/windowed DirectDraw setup; one direct call site each |
| `0x58ad90` | Surface creation and surface-interface query; 57 direct call sites |
| `0x58b660` | Surface lock wrapper; 69 direct call sites |
| `0x58b69d`, `0x58b735` | Surface Lock at vtable byte offset `+0x64`, including retry |
| `0x58b7f4` | Reads the locked pixel pointer from `0x6f6904` for return |
| `0x58b806` | Conditional Unlock at `+0x80` before returning that pointer |
| `0x58bf40` | Whole-surface opaque-copy wrapper; 70 direct call sites |
| `0x58c03a`, `0x58c05a` | Blt `+0x14` / BltFast `+0x1c`, with WAIT flags |
| `0x58c140` | Whole-surface source-keyed copy wrapper; 17 direct call sites |
| `0x58c239`, `0x58c252` | Blt with KEYSRC, without/with WAIT |
| `0x58c276` | BltFast with SRCCOLORKEY, optional WAIT |

The surface wrapper accesses Surface2 at object `+0x08`, width/height at
`+0x10/+0x14`, and stores pitch at `+0x18`. Its 108-byte descriptor is at
`0x6f68e0`; pitch is `0x6f68f0` and pixel storage pointer `0x6f6904`.
Restoration code sets a source color key from object WORD `+0x2c` using
SetColorKey `+0x74` and flag 8. These fields are confirmed for the inspected
wrappers, not a complete object layout.

`0x58b270` loads USER32 monitor functions, including MonitorFromWindow,
GetMonitorInfoA and GetSystemMetrics. Strings reside in `0x5f19d8–0x5f1a63`.
Its indirect calls must not be mistaken for dynamically selected rasterizers.

The numerous lock callers and returned pixel pointer establish a CPU-visible
surface path alongside Blt/BltFast composition. **Inference:** replacing only
DirectDraw copies is unlikely to replace all rendering. Actual pixel-writing
loops, sprite asset formats, terrain drawing and gameplay frequency remain
unclassified. Static caller counts are not runtime draw counts.

## Implemented capture and replay

`runtime/render/` remains new instrumentation, separate from reconstructed
engine algorithms. Its PE32 bridge now forwards BltFast as well as Blt/Flip
and captures successful primary-surface presentation through all three.

Opt-in draw capture logs the first 2048 observed application calls to Blt,
BltFast, Flip, Lock, Unlock and CreateSurface. Each includes a caller return
address for correlation with static evidence. Observer locks are excluded.
Pointer tokens identify interfaces only within a run; allocator reuse and
interface aliases prevent treating them as persistent surface IDs.

The bridge additionally tries at most eight eligible small blits and writes
at most one capture file. It records full source and destination-before pixels,
releases its locks, forwards the original draw unchanged, and reads destination
pixels afterward. Supported operations are equal-format, unscaled, in-bounds
opaque or source-keyed copies without a clipper, effects or self-copy. Source
size is bounded to 256x256; destination to 2048x2048. Readback uses nonblocking,
read-only locks; failed snapshots skip evidence without changing API results.
Original HRESULTs and last-error state are preserved. Details and exact binary
layout are in [the format specification](../formats/render-draw-capture.md).

The native-pixel CPU reference in `tools/replay-render-capture.py` copies the
source rectangle, skipping pixels equal to an enabled source key. Color-space
key ranges are deferred; inspected wrappers set equal low/high values. It compares
the whole resulting destination against captured output, writes mismatch
coordinates and native values, and produces source/before/captured/replayed PPMs.
This establishes a small drawing contract for a future OpenGL implementation;
it does not replace game drawing yet.

## Prepared live procedure

```bash
./tools/run-qt-shell.sh --capture-draws
```

The shell opens without launching the game. Click **Launch game** when ready;
the launch log prints the new `draw-capture` directory. Exit the game, then run:

```bash
./tools/replay-render-capture.py working/experiments/opengl-render/run-XXXX/draw-capture
```

The first eligible operation may be a menu bitmap; it is not automatically a
creature sprite. No file means no completed eligible sample (unsupported path,
busy surfaces, exhausted attempts or failed file creation); the event inventory
can still narrow the active path. Capturing perturbs rendering and disk traffic;
these are correctness samples, not benchmarks. Concurrent surface modifications
are not globally frozen and may cause an honest mismatch.

Staging-only validation created
`working/experiments/opengl-render/run-1wpu8mon/` with a hash-checked PE32 DLL,
patched disposable executable and empty capture directory. The game was not
started. The shell command above creates a fresh experiment when launched.

Use the PPMs and caller addresses to classify the sample. The same small copy
contract is now implemented in the [native OpenGL renderer](opengl-blit-replay.md).
Run replay with `--backend opengl --headless` to compare the shader result
against both this reference and captured output. Selected CPU pixel-writing
callers and sprite decoding remain later work.
Input forwarding, full surface lifetime tracking, palette updates, clipping,
effects, animation and complete in-game equivalence remain separate work.

## Validation and confidence

`python3 tests/test-render-capture.py`: seven host tests cover all four native
pixel sizes, opaque/exact-key copies through both operation formats, subrect
strides, duplicate palette colors, full-destination mismatch reporting, preview
output and malformed/truncated evidence rejection.

`./tools/test-render-bridge.py`: isolated synthetic Wine run
`working/tests/render/run-mmyp99hs/` validates actual x86 hooks with opaque Blt
and source-keyed BltFast, independently expected pixels, negative source pitch,
padded destination rows, old Surface2 unlock ABI, original error/result values,
unsupported-flag forwarding, post-call busy-surface rejection and retry,
one-capture limit and exclusion of observer locks
from events. Replay matched both captures exactly. The existing mapped-frame
producer-to-Qt/OpenGL readback also passed. These are fake COM surfaces; actual
game/driver compatibility still requires live evidence.

All six existing Qt CTests pass. Production PE32 bridge build, staging hashes,
export artifact hashes and rejection of an unsupported executable also pass.

Primary API references:
[Blt](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-blt),
[BltFast](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-bltfast),
[GetColorKey](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-getcolorkey).
