# Native SPR upload and OpenGL rendering

## Regression follow-up — 2026-10-10

The [drawing-family validation](drawing-family-validation.md) found that
`tools/test-sprite-render.py` retained the floor-scaled RGB565 expectation after
the [independently measured bit-replication policy](surface-dib-ddraw.md) landed.
The test now uses that driver-derived expansion. Its
[fresh installed-frame execution](native-sprite-bitrep-regression-20261010.json)
matches all 91 independent original RGB565 outputs and presentation hashes,
including 14 indexed and 77 direct frames, two empty frames and zero leaked
surfaces. The failed prior suite remains separate and unchanged. Rendering code,
producer admission and live replacement scope did not change; indexed embedded
palette policy, original layout/callers and supported-session behavior remain
bounded as described below.

## Historical implementation

Rendering asset chunk 3, reviewed 2026-10-04. The version-4 native SPR loader
now feeds owned OpenGL surfaces through a small sprite adapter and an offline
preview CLI. This milestone renders selected assets without Wine; it does not
replace live original loading or scene composition.

## Implemented contract

`renderer/sprites/sprite.hpp` defines `UploadedSpriteFrame`. Creation takes a
decoded native `Sprite` and frame index, validates dimensions, typed buffers,
palette selection and mask values, and uploads an RGB565 colour surface and an
indexed8 coverage surface. No input vectors, file handles or palette pointers
are retained. Decoded data may be released immediately. The renderer must
outlive the upload; construction, drawing and destruction use its GUI thread.
The noncopyable object destroys both surfaces on scope exit. Failure to
allocate the second surface releases the first.

Direct-colour words remain RGB565. Indexed colours use the selected embedded
RGB palette, quantized with red/blue `>>3` and green `>>2`. This is the explicit
unshaded RGB565 policy used by the isolated original drawing fixture. It does
not run or reproduce original palette-chain construction, lighting variants,
palette animation, RGB555 selection or auxiliary-plane effects.

`draw(destination, anchorX, anchorY)` computes the top-left position as
`anchor - signed frame origin` using 64-bit intermediate arithmetic, then
requires the entire frame to fit the destination. Origins are used for
placement, rather than discarded during upload. The destination must have
RGB565 masks `{0xf800, 0x07e0, 0x001f}`. Empty frames allocate nothing and drawing
them is a no-op, including for a nonexistent destination.

`GlBlitter::copy` additionally accepts an optional source-sized indexed8 mask
surface. The shader fetches mask pixels in source coordinates, discards zero
mask values, and retains the destination at those positions. Nonzero coverage
copies the native word, including zero. If a key is supplied too, both coverage
and key conditions apply. The mask must differ from the destination. Bad
handles, formats and dimensions are rejected before drawing. Ordinary copies
explicitly disable mask use; the existing command wire format is unchanged.

Both colour/mask textures stay resident. Draw calls use the copy shader without
native uploads or readbacks. Explicit `read` and `present` remain synchronization
points; presentation still reads RGBA back for Qt. Two surfaces and twice the
frame pixel count are charged to the existing 64-handle/16,777,216-pixel budget.
Frame dimensions remain bounded to 2048 each. Clipping, stretching, blending,
shading, live capture routing and animation scheduling remain outside scope.

## Build and preview

The small integration project links `mnm-sprite-loader` and `mnm-renderer`;
the underlying assets and renderer libraries remain independent of each other
and of application widgets. It can be added to a parent CMake project that
already defines either target.

```bash
cmake -S renderer/sprites -B working/build/sprite-render
cmake --build working/build/sprite-render --parallel 4
ctest --test-dir working/build/sprite-render --output-on-failure

xvfb-run -a env QT_QPA_PLATFORM=xcb LIBGL_ALWAYS_SOFTWARE=1 \
  working/build/sprite-render/mnm-sprite-preview \
  --root working/game-clean --prefix C:/MagicMayhem \
  --path 'c:\MagicMayhem\cReAtUrEs\rEdCaP.sPr' --frame 0 \
  --output working/redcap-native.565 --preview working/redcap-native.png
```

Use new output filenames. The CLI opens through `AssetStore`, closes the file
after decoding, uploads the selected frame, releases CPU decoded buffers, then
draws onto a `0x1234` background with a one-pixel border. The anchor is
`origin + 1` on each axis; preview dimensions are `frame dimensions + 2`.
Empty records produce an untouched 2x2 background. Frames exceeding 2046 in
either dimension or whose bordered anchor exceeds signed `int` are rejected.
Input follows the loader's normal budgets. Native output is packed little-endian
RGB565; PNG output comes from the renderer's RGB565 presentation shader. Both
outputs require new files. If writing a later output fails, earlier outputs may
remain; this tool does not promise an atomic multi-file transaction.

The CLI writes a JSON report to stdout with origin/anchor metadata, draw counts,
native and raw RGBA hashes, driver identity and final surface counts. `--manifest`
accepts an array of `{path, frame, output, preview}` instead of the single-path
options. Manifest input is bounded to 1 MiB/128 entries, and aggregate native
plus raw RGBA output to 32 MiB. Exit 0 means complete rendering; errors return 2.
The CLI has no original-manifest wrapper; use the experiment runner for
installed-asset validation.

## Validation and evidence

```bash
python3 tools/test-sprite-render.py
```

The runner verifies the immutable manifest before/after, also on failures,
and executes the pinned [original comparison](sprite-binary-comparison.md)
afresh. This requires the pinned MMSprite checkout, known clean/No-CD executables,
`g++ -m32`, ELF i386 execution and Xvfb. The reference executes unmodified No-CD
draw routines in private mappings; the game is not launched or patched.
Installed SPRs, source files and the preview executable are hash checked.
Requests alternate mixed-case relative and explicitly aliased Windows paths.
Original after-pixels are comparison inputs only and never sent to OpenGL.

The [retained report](native-sprite-rendering.json) records source/executable/
input/output hashes and graphics-driver identity. Generated decoded pixels and
PNGs remain under ignored `working/tests/sprite-render/`. The report records
91 installed frames: 77 direct RGB565, 12 nonempty indexed and two empty indexed
records. All 91 OpenGL destinations match fresh original draw bytes; all 91
presentation RGBA hashes match independent CPU expansion of those original
RGB565 words. This includes all 27 palette-free files and selected RedCap,
effects3 and terrain palette samples. Empty records emit no copy, and nonempty
draws emit exactly one copy without native uploads/readbacks. Final surface
count is zero. The original indexed palette builder is not executed.
The recorded run is `working/tests/sprite-render/run-5r551ygz/`, using Mesa
llvmpipe (OpenGL 4.6, Mesa 26.2.1). Its original-before/after logs verify 2,927
immutable originals; fresh original reference evidence is in
`working/tests/sprite-binary/run-k95nrsof/` with its own manifest checks.

All 11 CTests pass: four assets tests, six existing renderer tests and the new
sprite renderer test. The new test independently checks 64 seeded indexed/direct
draws with masks and signed origins, all four palette selections, opaque zero,
CPU/GPU RGB565 presentation, retained uploads after source data destruction,
out-of-bounds/extreme origins, empty records, malformed upload inputs,
second-allocation rollback, cropped source-mask coordinates, key/mask
conjunction, ordinary-copy mask reset, and stale/incompatible mask rejection.
The sprite fixture also passes AddressSanitizer/UndefinedBehaviorSanitizer;
leak checking is disabled for this environment. GPU resource counts are checked
explicitly; this does not establish a general leak profile.

Confidence is high within these tested operations and selected installed
samples. It does not establish every frame/effect, original clipping behavior,
palette/shading generation, fonts, ANI timing or whole-scene/live equivalence.
The next milestone should recover animation selection/timing and compose a
bounded native scene using these verified frames before considering live work
replacement. Gameplay balance remains unchanged.
