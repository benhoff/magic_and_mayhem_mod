# x86 capture to persistent OpenGL command replay

## Finding and confidence

Confirmed with synthetic Wine PE32 surfaces: the bridge exports a completed
ordered checkpoint session, native OpenGL replay matches the original fake
DirectDraw draw byte-for-byte, and Qt displays that replay with correct pixel
orientation/colors. The original draw is still forwarded with unchanged API
arguments, HRESULT and last-error behavior. The producer reuses accepted
snapshots; it introduces no further locks, COM queries or object references.

The independent command tests cover rectangular CPU-style uploads, opaque and
exact-keyed copies, partial palette updates, presentation and destruction across
8/16/24/32-bit formats. Each sequence uses a separate Python native-pixel oracle
and RGB conversion to check GPU output. Destroyed/unknown/reused IDs, sequence
gaps, missing END, malformed payloads and poisoned CHECK outputs are rejected.

This establishes synthetic protocol/replay behavior with software Mesa, not
real-game hook coverage or physical-GPU compatibility. Game-created surfaces,
canonical interface aliases, Release, CPU Lock/Unlock writes and palette-object
mutations are not continuously tracked yet. CREATE/DESTROY in the bridge's
checkpoint file are replay resources, not observed COM lifecycle events. The
original engine remains responsible for all in-game drawing.

## Reproduce

```bash
cmake -S renderer -B working/build/renderer
cmake --build working/build/renderer --parallel 4
ctest --test-dir working/build/renderer --output-on-failure
./tools/test-render-bridge.py
```

The Wine test generates fresh opaque and keyed command sessions, checks native
pixels against the original synthetic draw, and checks both sessions through Qt
framebuffer readback. It does not consume `original/` or launch a map/game.

Preview any completed checkpoint session without launching Wine:

```bash
./tools/run-qt-shell.sh --commands CAPTURE_DIRECTORY/commands-0001.bin
```

Export independently generated native pixels and a PNG (new paths only):

```bash
QT_QPA_PLATFORM=xcb LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a \
  working/build/renderer/mnm-render-commands \
  CAPTURE_DIRECTORY/commands-0001.bin \
  --output OUTPUT_DIRECTORY/native.bin --preview OUTPUT_DIRECTORY/frame.png
```

To collect real-game evidence later, the command remains ready:
`./tools/run-qt-shell.sh --capture-draws`; click **Launch game** when ready.
Exit the game before reading its capture files. Read the existing draw-capture
procedure for eligible operations and bounded-capture restrictions.

## Evidence from this change

- `working/tests/render/run-3ep9heb_/report.json`: fresh x86 Wine opaque/keyed
  captures, one GPU copy and one successful native CHECK per session, two initial
  uploads, zero remaining surfaces, and both Qt command previews checked.
- `working/tests/render-commands/run-yk89idbm/report.json`: four format sequences,
  96 copies, 100 native CHECKs, eight presentations, independent RGBA hashes,
  and rejection of 25 malformed/mismatching sessions, including live-surface
  and record-count limits.
- All six renderer CTests and seven Qt shell regression tests passed. Production
  bridge built as PE32 i386 with SHA-256
  `232250a66f2d719fe8ab8b6fae399aa260535e037dbadf2bc5016a87a1db9ed1`.

## Next integration

Use this protocol for a bounded **continuous** history only after adding explicit
canonical surface identity/lifetime tracking and recording writable Lock/Unlock
updates, palette changes and unsupported-call gaps. Reentrant/concurrent or
unobserved operations must invalidate coverage rather than imply exact replay.
Validate each addition against synthetic interfaces before attempting real-game
render replacement. Flips require an explicit surface-content/swap model.

Protocol: [surface commands](../formats/render-surface-commands.md).
