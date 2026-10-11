# DRAW-03 bounded native scene composition

The native `DrawingScene` adapter composes a declared finite source history into
one complete RGB565 canvas. Native storage starts undefined. Fills, encoded
images, glyph coverage, sprite colours/effects, copies and minimap source cells
must define every pixel needed by later operations. There is no diagnostic clear,
original destination seed or oracle callback. World inputs bind actual source SPR
files through the existing strict resource catalogue; effective colours, clips,
shading and draw order are preserved. GPU World completions must equal separate
native CPU composition before they become the input to subsequent UI work.

The adapter lives in `compat/legacy/`, because its producer records are specific
to the No-CD build. Native renderer/resource services remain independent of the
wire format, widgets and hooks. `mnm-drawing-scene-replay` uses the adapter without
a QWidget session or installed game. It validates source asset hashes and produces
native checkpoints plus a GPU frame only after the declared boundary seals.

## Declared scene and evidence

The portable fixture is the first complete **pre-display-copy canvas** of the
retained 800x600 Quick Battle map2 startup. It consumes the unchanged contiguous
producer history through sequence20458: menu/loading initialization, World
queue1, later HUD copies, minimap terrain and marker/outline points, then cursor
composition. Its final independent original oracle is checkpoint920 on canvas2.
Checkpoint908 covers World completion;915/916,918/919 and920 cover successive
post-World layers. These are full800x600 canvases, including borders.

`tools/retain-drawing-scene.py` verifies the full historical stream and source/
output hashes against the immutable original comparison, then retains the exact
prefix,16 actual encoded source assets and six oracle planes. The immutable
retention catalogue pins that slicing operation and its tool sources. The
historical comparison and its source fingerprints are preserved. Portable replay
needs no `working/` installation, original executable or source media; it creates
only disposable native inputs under `working/tests/drawing-scene/`.

`tools/check-drawing-scene.py` verifies retention and original provenance,
source/frame identities and every selected complete output. Only producer bytes,
encoded assets, boundary identity and requested native checkpoint numbers enter
the native process. Oracle paths and bytes stay in the comparator. It checks
binary/input/source stability and reports full-word differences and pixel bounds.
A new native execution against these frozen original outputs is separate from a
new execution of the original game.

Fresh native execution reproduces all six complete checkpoints:2,880,000RGB565
pixels, zero differences. The final canvas includes all post-World layers through
the captured cursor copy. Eight portable guards and the native order/refusal/
cleanup suite pass. The new result is retained separately as
`draw03-native-comparison-20261010.json`; full presented-scene and live stages
remain open. An initial materialization created separate Interface/interface
directories and was correctly refused by the strict resolver; source packaging
now uses one canonical case-folded tree while keeping requested asset identities.

## Admission and side effects

Producer sequence gaps, checkpoint identity gaps, incomplete native history,
unknown inside-World producers, missing/mismatched resources, wrong queue return
identity/extent, incomplete queues and wrong final canvas/extent refuse. Errors
poison the scene; partially executed storage cannot be presented or resumed.
A new instance with complete source history is required. A sealed scene is
immutable. Resource/storage cleanup, source hashing, native canvas copies,
GPU writeback and complete readbacks are checked within this native contract.
A completed diagnostic checkpoint does not implicitly authorize presentation.
The explicit boundary must follow all layers in the declared frame.

Synthetic tests exercise ordered World/HUD/encoded-BMP/final-copy composition,
undefined startup, incomplete boundaries, unknown producers, sequence/identity
errors, missing assets, poisoning and cleanup. The baseline fixture verifies the
real startup layers through the final composed canvas.

## Reproduce

```sh
cmake -S compat/legacy/drawing-scene -B working/build/draw03 -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/draw03 --target mnm-drawing-scene-replay drawing-scene-test -j4
ctest --test-dir working/build/draw03 -R '^native-drawing-scene$' --output-on-failure
python3 -B tests/test-drawing-scene.py
python3 tools/check-drawing-scene.py --headless \
  --executable working/build/draw03/mnm-drawing-scene-replay \
  --report working/tests/draw03/new-comparison.json
```

For a scope-bound result, prepare claims before execution with
`tools/draft-coverage-claims.py --behavior NR.drawing-scene
--scenario DRAW03.startup-composed-canvas --output <new-claims.json>` and pass
`--claims <new-claims.json>` to the checker. Report paths must be new.
`--require-complete` remains nonzero: this does not close the broad DRAW-03 gate.

## Remaining boundaries

The independent oracle precedes the final display copy. It does not prove physical
DirectDraw presentation/Flip, driver failures or a complete displayed-session
journey. Native GPU presentation readiness is separate from original presentation
equivalence. Original consumer non-pixel state, arbitrary readbacks and lifetime/
error semantics are not recovered by this adapter. Minimap visibility and marker
geometry are owned captured inputs; their native source-state generation is
pending. Cursor pixels do not establish picking/hotspot behavior. Alternate draw
kinds, camera motion, active battles, general tooltips/windows, transitions and
unbounded delivery remain pending. No live observation or bypass is introduced.

Confidence is high within the pinned pixel-composition scope, and provisional
outside it. The full DRAW-03 milestone remains open until a declared complete
scene includes all required original side effects and independent presentation
comparison; omitted draws or approximations cannot satisfy that gate.
