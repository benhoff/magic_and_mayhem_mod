# Native menu, loading, HUD and startup canvas sequence

The selected No-CD800x600 Quick Battle run now reproduces every sampled completed
canvas from menu startup through the first16 World returns. The final marker-complete run is pinned in
[native-canvas-producers-live-20261008.json](native-canvas-producers-live-20261008.json).
Its49 signature-checked producer sites yielded **1,062 matched completion
checkpoints /474,236,912 compared RGB565 pixels /zero differences**. All16 raw
startup queues were captured from queue1, including actual previously unsupported
kinds20 (`0x14`) in queues3–4,17 (`0x11`) in5–6 and16 (`0x10`) in7–8.
The startup-policy labels and the actual consumer kinds remain independent:
closed low-level producer inputs also record their selected drawing branches.

The Qt consumer retains process-local logical canvas generations. Create starts
undefined; bind and clip preserve state, fills define only their actual area,
copy uses the named source and destination, release removes the generation.
Completed native CPU canvases are mirrored to retained OpenGL surfaces, read
back for exact comparison, and800x600 completions are presented through the Qt
GPU viewport. Original oracle files never enter the consumer; it receives only
closed producer requests and separately pinned encoded source assets. This is
CPU composition plus a checked GPU mirror, not a claim that every operation has
been implemented as an OpenGL drawing command. Original drawing remains active.
The independent Worldv2 comparison/session policy keeps its separate seed and
integration boundaries; this producer viewer does not promote that path.

Confirmed source contracts (high confidence for the pinned selected windows):

- `58bf40`: ECX is the source wrapper; the first stack argument is destination.
  The final `46b9fc` call presents draw canvas6a49e0 into6a4fe0. It does not import
  presentation pixels into the first World canvas. Earlier evidence is retained;
  its first direction interpretation is superseded by the original disassembly.
- `58d320`: stack filename, original JPEG decoder destination lock, pitch,
  extent and format. A600x36 source is cropped to598x36 progress storage.
- `581ec0`: tinted font coverage, stack Y/R/G/B,64 float coverage values,
  channel quantization and truncating interpolation, full horizontal rejection
  and vertical cropping. It is not a general sprite dispatcher.
- `58ddf0`, `58d8c0`, `58e2c0`: inclusive asymmetric two-pixel bevel,
  asymmetric highlight-corner ownership and half-destination whitewash.
- `54b2d0`: owned PCX bytes plus effective RGB palette; selected unpadded rows.
- `58d1a0`: origin BMP file source, including the cursor atlas and HUD images.
  GDI clips49x23 Control Limit source into48x22 storage. JPEG/BMP/PCX output is
  produced from encoded source inputs; no original destination import is used.
- `596cb8` and `597086`: direct-word menu/HUD rasters; selected effective indexed,
  shaded, blend, wave and projected-shadow inputs are separately owned.
- `58c360`, `58c4a0`, `58c6a0`, `58cf20`: opaque rectangle copies.
  `58c140`, `58c8a0`, `58ca90`, `58cd50`: keyed copies.
  `58cbc0` has source/destination rectangles; unequal extents are refused.
  Deferred58d5b0 requests are observed when58d660 executes these copy branches.
- `58ed80`: menu transition halves each packed RGB565 word with7bef mask.
  Capturing this writer closes the large black/green residual after repeated
  presentation calls; it is not an invented initialization clear.
- `553a40`: orientation0 minimap terrain from source-cell colors and visibility,
  wrapped around its grid center and stepped through alternating diamond rows.
  Hidden cells read retained native auxiliary history, not sampled original pixels.
- `5527a0`, `5536c0`, `553850`: camera corner outlines, palette-colored cell
  markers and visible creature markers. The adapter resolves point/color requests
  from original source geometry/visibility; native storage composes those points.
  Full native generation of source geometry and visibility is a separate boundary.

The wire format is documented in
[canvas-producers-v1.md](../formats/canvas-producers-v1.md). Captures are bounded
by16 queues,128 MiB input,65536 records,128 tracked wrappers and1 GiB oracle
storage. Native admission also limits dimensions and retained pixel capacity.
No extra original Lock is introduced to sample a destination: checkpoints use
successful original lock pointers and known dirty generations. Primary/front
buffer driver presentation, unsampled/unlocked outputs, allocation/restore/error
paths, format changes, minimap rotations, stretched copies
and unknown producers remain gaps. RGB-add545c10, in-memory DIB58d240,
positioned BMP58d280 and clipped-word589c90 were instrumented but not observed
in this scenario; their instrumentation does not establish pixel equivalence.

Three newly executed copy entries58c6a0/58cd50/58cf20 are absent from the pinned
Ghidra discovery inventory. Their explicit16-byte recovered entry ranges retain
the unassigned-byte inventory and are justified only by signature-pinned private
original execution with a COM success adapter. They do not classify whole
functions or claim all copy recovery branches. Independent private original
fixtures also compare bevel, highlight, whitewash, fade and minimap with/without
hidden cells; the lock/unlock fixture adapters are explicit and the recovered
writers execute unchanged. Whole original executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

Reproduce in an isolated pinned source installation:

```sh
cmake -S compat/legacy/canvas-producers -B working/build/canvas-producers
cmake --build working/build/canvas-producers --target mnm-canvas-producers-live
xvfb-run -a -s '-screen 0 1280x1024x24' python3 tools/capture-scene-game.py \
  --canvas-producers --samples 16 --source-game <pinned-source-installation> \
  --producer-live working/build/canvas-producers/mnm-canvas-producers-live
python3 tools/test-native-canvas-producers.py --capture-report <capture>/report.json
python3 tools/test-canvas-producers.py
```

Capture preparation
checks the source executable hash and scripts all binary changes. The immutable
original manifest is verified before and after original-consuming experiments.
Hash-bound prospective coverage declarations are passed via `--claims` for
registered validation. The offline driver freezes native sources, builds and
runs the unit/original fixtures, gives replay only owned source assets and the
input stream, then independently compares every tight565 oracle. Historical
negative captures and their exact source/output fingerprints remain preserved;
budget failures, incomplete streams, missing producers and source drift never
become passing evidence by editing old hashes.

## 2026-10-10 producer extension

The historical 16-return run above retains its original source fingerprints.
The current increment separately validates [higher text layout](font-line-layout.md),
[source DIB/BMP cropping](native-canvas-image-production.md), and
[odd/padded RGB565/RGB555 physical fade spans](canvas-fade-span.md). Fade projection
uses the original contiguous span rather than a per-row odd-tail rule. A fresh
four-return original-active live shadow checks the shared native canvas pipeline;
it does not refresh the historical 16-return evidence or suppress original text,
image, HUD, cursor or panel bodies. Broader live callers and original wrapper/DC
failure/lifetime remain pending.

The [final current-source shadow](native-canvas-producers-final-shadow-20261010.json)
passes 954 completed checkpoints/422875312 pixels,
2002 glyph calls, zero differences and exact GPU mirrors.
