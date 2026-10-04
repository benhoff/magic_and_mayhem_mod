# Bounded native ANI/SPR scene preview

Reviewed 2026-10-04. `apps/sprite-scene` composes selected native SPR frames using
the [recovered forward ANI player](animation-forward-contract.md), persistent
[mask/RGB565 uploads](native-sprite-rendering.md) and a Qt preview window. The
runtime preview does not require Wine or an original executable. This is a
bounded application milestone, separate from whole-scene or live replacement.

## Ownership and scene policies

The native asset and renderer services remain independent of build-specific
reconstruction and widgets. `SpriteScene` is application orchestration: it owns
decoded SPR data, independent forward players and a renderer-backed upload
cache. Widget code handles image presentation, pause/resume and presentation
interval controls; original addresses do not enter widgets or shared channels.
Input ANI/SPR files are closed after decoding. Players copy their selected
sequences; the scene retains SPR data to create additional uploads after cache
misses. The renderer outlives the scene, and all GL work uses its GUI thread.

The scene uses a **512x256 synthetic checkerboard**, **one to four actors**,
explicit numeric ANI sequences and fixed preview anchors. Actors draw in
selection order. Their signed SPR origins are subtracted from those anchors.
ANI metadata is preserved, but additional coordinate/attachment fields are not
applied. This layout is not recovered map projection, depth sorting or placement.
There is no terrain loading, camera, clipping, lighting, particle/event effect,
reverse playback or automatic action/direction selection.

The cache holds at most **24 uploaded frames**, with least-recently-used
eviction. Each owns colour and coverage surfaces, giving at most 48 upload
handles plus the background/canvas handles. Existing renderer pixel budgets
still apply; two large sprite surfaces may exhaust them before the handle limit.
Each presentation copies the retained background before drawing current actors,
so stopped/changed frames do not leave trails. Resident frame copies require no
native upload/readback. Presentation and exported native pixels synchronize
explicitly through the existing renderer APIs.

Each preview step makes one controller tick call per actor. A stopped sequence
normally remains hidden, matching the selected forward contract. `--loop` is an
explicit preview policy that restarts a stopped sequence on the following step;
it is not an inferred original gameplay transition. Event arguments are recorded
without executing gameplay, sound or attachment effects.

The window's **10..1000 ms presentation interval** schedules preview steps.
This adjustable wall-clock pace is independent of any claimed original game
clock. Runs are bounded to **1..512 steps**; the timer stops at completion.
Pause/resume affects only this preview timer. `--smoke-test` closes the window
after its bounded run, and reports startup/tick/presentation failures as exit 2.
No automatic original/CPU rendering fallback is supplied.

## Build and run

```bash
cmake -S apps/sprite-scene -B working/build/sprite-scene
cmake --build working/build/sprite-scene --parallel 4
ctest --test-dir working/build/sprite-scene --output-on-failure

working/build/sprite-scene/mnm-sprite-scene-preview \
  --root working/game-clean --ani 'Creatures\redcap.ani' \
  --sequences 0,4 --ticks 128 --interval 100 --loop
```

Default paired SPR selection uses the ANI file's raw filename as a sibling
basename. It requires a printable, NUL-terminated name without path separators,
colon or dot-only components; `--sprite` supplies an explicit asset-path override.
This is a native application resolution policy. Numeric sequence indices and
every selected type-0 sprite index are validated against their owned inputs.
Mixed-case Windows paths and the explicit `C:/MagicMayhem` alias are supported.

To export synchronously instead of opening a window:

```bash
xvfb-run -a env QT_QPA_PLATFORM=xcb LIBGL_ALWAYS_SOFTWARE=1 \
  working/build/sprite-scene/mnm-sprite-scene-preview \
  --root working/game-clean --ani 'Creatures\redcap.ani' \
  --sequences 0,4 --ticks 32 --loop \
  --export-dir working/NEW_scene_export
```

The export directory must be new and its parent must exist. Exports are bounded
to **64 steps plus the initial frame**. Each writes canonical little-endian
RGB565 `.565` pixels and an OpenGL-presented PNG, followed by `report.json` with
selected sprites/events, anchors, native/RGBA hashes, upload/copy counts and
final surface count. Outputs are new-only. Errors return 2 and may leave earlier
outputs in the newly created directory; multi-file export is not atomic. The
CLI itself has no original-manifest wrapper, so use the experiment runner for
installed validation.

## Recorded validation

```bash
python3 tools/test-sprite-scene.py
```

The runner uses a single private Xvfb/Mesa session for all child processes,
verifies immutable originals before/after (also on failures), hash-checks
installed ANI/SPR and the known No-CD executable, and records source/preview
hashes. It privately executes the unmodified original forward controller using
the same harness as the 347-trace comparison. Original state/oracle values are
comparison inputs only; they do not drive native playback or become GPU inputs.

[Retained report](native-animation-scene.json), generated in
`working/tests/sprite-scene/run-7rxolzb5/`:

- Two explicit RedCap sequences, stop and preview-restart policies, 65 displayed
  frames per run: **130 scene frames / 260 actor states**.
- All **260 selected SPR indices and events** match independently obtained
  original controller states, with restart timing applied separately as a
  preview policy.
- All **130 native destinations and presentations** match an independent
  checked SPR row/palette decoder, signed-origin composition and CPU RGB565
  expansion. **23 distinct sprite frames** were exercised.
- A Qt window completed a three-step timer smoke run. An existing export
  directory was rejected and its report hash remained unchanged.
- Native scene surfaces return to zero at scope exit; original-manifest logs
  verify all 2,927 immutable inputs before/after.

The current 14 CTests pass, including five assets tests, six renderer tests,
the sprite-upload fixture, the forward-model fixture and the new scene fixture.
The scene fixture compares **80 four-actor frames** using **128 distinct synthetic
sprites**, checks the 24-upload/50-handle cache boundary during eviction,
stop/restart selection, complete background clearing and zero final resources.
ANI parser, forward model and scene fixtures also pass ASan/UBSan, with leak
checking disabled for this environment. Pause/resume clicks and real-display
hardware drivers remain manual coverage.

Confidence is high for the selected forward controller traces and these
explicit preview composition policies. The pixel oracle is independent CPU
reconstruction, not original whole-scene rendering. Earlier original sprite
draw comparisons remain separately recorded; this milestone does not establish
complete animation metadata, action selection, world scheduling or live scene
equivalence. Recover action/direction mappings, phase-preserving switches and
additional placement fields before replacing original creature presentation.
