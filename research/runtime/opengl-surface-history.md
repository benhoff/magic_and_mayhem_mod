# Bounded RGB surface history

The subsequent [indexed palette extension](opengl-indexed-palettes.md) adds
8-bit histories and palette hooks. Findings below describe the initial RGB chunk.

## Confirmed implementation

`runtime/render/surface_history.h` extends the opt-in x86 bridge beyond one
checkpoint. `--capture-history` implies `--capture-draws` and enables a separate
`history-0001.bin`; existing single-draw capture files remain available.

The first eligible successful draw seeds the history. Subsequent eligible RGB
Blt/BltFast calls retain replay surface IDs instead of reuploading full snapshots.
Before each COPY, two CHECK records compare the current source/destination with
the original engine; another CHECK validates the original result. PRESENT
records mark completed recorded copies, including offscreen draws, and are replay
inspection points rather than claims about the game's actual display timing.

Surfaces first encountered in an eligible copy are initialized from its
checkpoint, not assumed to be freshly allocated or cleared. Canonical IUnknown
identity and intercepted QueryInterface aliases share a session ID. Observer
QI references are balanced through saved Release methods, bypassing application
hooks. No extra references keep objects alive. A forwarded final Release (zero
remaining references) retires all recorded aliases and emits DESTROY. If an
object address is reused later, it gets a fresh ID; IDs are never recycled.
The canonical interface must already use an intercepted surface vtable. An
uncovered interface/alias invalidates capture instead of broadening vtable hooks
into an unknown layout.

Successful full-surface Lock calls store returned pixel pointer, pitch and
format. Before a writable Unlock, the bridge copies tightly packed pixels from
the application's still-held lock, including negative pitch and padding. Only
a successful original Unlock emits UPDATE. A subsequent independent nonblocking
read-only snapshot emits CHECK. A failed Unlock keeps the pending lock state for
a retry; failed Lock and successful read-only locks emit no UPDATE. Surface2
Unlock uses its pixel-pointer argument; Surface4/7 uses a null full-surface RECT.
Rectangular application locks and other flags/layouts are currently rejected.

The x86 stdcall hooks use surface vtable slots 0 QueryInterface, 2 Release,
5 Blt, 7 BltFast, 11 Flip, 25 Lock and 32 Unlock. Coverage guards also forward
slots 6 BltBatch, 17 GetDC, 27 Restore, 28 SetClipper and 31 SetPalette. Signatures
were checked against the installed primary header
`/usr/include/wine/windows/ddraw.h`. Saved original methods are used for observer
operations; runtime surface/interface pointers are launch-local tokens.

## Bounds and gaps

A history stops after 16 successful recorded copies/CPU updates, after every
tracked object is released, or on process detach. Unreleased replay resources
are destroyed in the footer at a bounded stop/detach; these footer DESTROYs do
not claim game COM releases. The recorder permits 16 distinct surface lifetimes,
16 aliases per lifetime, 240 records before footer/GAP, and 64 MiB. It does not
reuse lifetime slots after Release.

Overlapping/reentrant observed calls invalidate the history. Successful partial
locks, outstanding locks at termination/final Release, unsupported canonical
interfaces, indexed formats, flips, and unsupported/uncapturable draws also
invalidate it. Successful tracked GetDC, Restore, SetClipper, SetPalette and
BltBatch calls invalidate coverage. Original calls still run with unchanged
arguments, HRESULT/reference counts and last-error state, including on failure.
An explicit GAP record makes native/Qt replay refuse the file. An unobserved
native memory change that survives until the next CHECK causes a replay mismatch
rather than being accepted as an UPDATE.

This is a bounded history of the supported observed paths, **not** complete
DirectDraw emulation or proof of whole-frame/game coverage. Other surface APIs,
DirectDraw RestoreAllSurfaces, pre-seed overlapping calls, external writers,
shared-vtable discovery limits and missed interfaces are not exhaustively
covered. Do not use a successful synthetic history to assert real-game fidelity.
Indexed palette-object updates and swap/flip semantics are still pending.

## Evidence and procedure

`./tools/test-render-history.py` uses freestanding PE32 fake Surface2/Surface4
objects in Wine, shared canonical identity/refcounts, padded negative-pitch
source rows, and a destination with an untouched border. It checks ordered
copies, both writable Unlock ABIs, successful read-only locks, failed Lock,
failed Unlock/retry, final Release, alias retirement, address reuse and bounded
termination. Separate fixtures reject partial locks, flips and reentrancy, and
an out-of-band native write fails GPU comparison. Additional fixtures cover
Restore, SetPalette, GetDC, an outstanding lock at process detach, and clean
detach with still-live COM objects. Valid histories also pass Qt framebuffer
readback; Qt refuses a GAP history. No game/media is consumed.

```bash
./tools/test-render-history.py
./tools/test-render-bridge.py
ctest --test-dir working/build/renderer --output-on-failure
```

Evidence from this change:

- `working/tests/render-history/run-_sl7klrn/report.json`: 13 x86 Wine fixtures;
  five valid native/Qt replays and eight expected capture/replay failures. The
  report records DLL and fixture executable hashes.
- `working/tests/render/run-xf5ntt_i/report.json`: default single-draw capture,
  opaque/keyed CPU/OpenGL comparisons, and Qt presentation still pass.
- Six renderer CTests and seven Qt shell regression tests pass.
- Production bridge builds as PE32 i386, SHA-256
  `b9db494a257e6dbdcf579b64ca9763dc923d66af4cafb641bf10bd4568e00c40`.

Confidence: confirmed synthetic x86/Wine and Mesa behavior; actual game and
physical-GPU behavior remain unvalidated. No balance or game binary rewrite is
part of this feature; existing guarded import staging is unchanged.

The real-game command is prepared but not launched automatically:

```bash
./tools/run-qt-shell.sh --capture-history
```

Click **Launch game** when ready, exit before reading files, then inspect the
recorded history through the native renderer:

```bash
./tools/run-qt-shell.sh --commands CAPTURE_DIRECTORY/history-0001.bin
```

Only complete, gap-free histories with matching CHECKs open a replay viewport.
The source bound for eligible copies remains 256x256 and destination 2048x2048;
clipping, scaling, self-copy, destination keys and effects remain unsupported.
Capture introduces observer locks and readbacks, so timings are not benchmarks.

Protocol: [surface commands](../formats/render-surface-commands.md).
