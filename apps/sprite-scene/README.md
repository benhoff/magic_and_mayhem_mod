# Native ANI/SPR scene preview

Bounded native scene application, separate from the Qt game shell and live hooks.
Loads version-5 ANI and version-4 SPR through AssetFile, uses the recovered forward
controller for explicit sequences, and composes mask/RGB565 uploads in OpenGL.
The original executable and Wine are only needed by research comparisons, not
by this preview application.

Body and child assets now have immutable native resource recipes and semantic
IDs. The preview submits its existing ordered display queue to the shared
[native scene service](../../renderer/scenes/README.md), which retains its canvas
and uploads. ANI scheduling, attachment placement, depth sorting and visibility
remain in the existing scene adapter. The manager outlives that adapter and keeps
its decoded resources resident; rendering does not advance the controller.

```bash
cmake -S apps/sprite-scene -B working/build/sprite-scene
cmake --build working/build/sprite-scene --parallel 4
ctest --test-dir working/build/sprite-scene --output-on-failure
working/build/sprite-scene/mnm-sprite-scene-preview \
  --root working/game-clean --ani 'Creatures\redcap.ani' \
  --sequences 0,4 --ticks 128 --interval 100 --loop
```

The interval is a preview clock. `--loop` explicitly restarts stopped sequences;
neither is a recovered gameplay policy. `--export-dir NEW_DIRECTORY` writes
bounded native/PNG frames instead of opening the window (maximum 64 steps).
The scene holds at most 24 uploads and four actors; no clipping or gameplay
event handling is performed.

Each actor has numeric group/facing controls when the paired ANI contains a
compatible group of eight. Group selection restarts; facing selection retains
animation progress and timing. These are structural groups, not verified named
gameplay actions. Stopped actors can be restarted by selecting a group again.
`--facing-change 7,0,7` schedules actor 0's change to facing 7 before tick 7,
for a repeatable window/export run. See
[selection contract](../../research/runtime/animation-direction-selection.md).

Installed validation: `python3 tools/test-sprite-scene.py`.
See [contract, export usage and evidence](../../research/runtime/native-animation-scene.md).

Visible bodies now include their ANI sprite displacement before SPR origins
are subtracted. Up to two explicit child ANI/SPR pairs can be composed through
the parent's attachment points:

```bash
working/build/sprite-scene/mnm-sprite-scene-preview \
  --root working/game-clean --ani 'Creatures\redcap.ani' --sequences 0,4 \
  --layer 'Creatures\bat.ani,0,1' --layer 'Creatures\eye.ani,0,2' \
  --tile-size 2 --placement-view 1 --ticks 32 --loop
```

These example assets demonstrate placement; they are not a recovered RedCap
attachment recipe. `--layer ANI,sequence,slot` accepts slots 1/2 and cannot
contain commas in the path. Default `--tile-size 1` and `--placement-view 0`
can be changed to footprint 2 and raw view 0..3. Children use independent
players and the shared 24-upload cache. Visible bodies and children now use
the recovered signed depth key and exact queue sort; world submission, occlusion,
lighting and complete child action/lifecycle production remain outside this preview.

Reproduce layered validation with `python3 tools/test-animation-layers.py`.
See [placement, limitations and evidence](../../research/runtime/animation-placement-attachments.md).

One recovered attachment recipe can replace the explicit example layers:

```bash
working/build/sprite-scene/mnm-sprite-scene-preview \
  --root working/game-clean --ani 'Creatures\redcap.ani' --sequences 0 \
  --attachment-mode-one --attachment-facing 0 --ticks 32 --loop
```

This reads installed effect entry 36 (effects2.ani, sequence 40 + admission
facing) and uses the first attachment point. Admission facing is independent
of the body controls. `--attachment-health` defaults to 1; zero hides the child.
`--attachment-remove-at 8 --attachment-reenter-at 16` exercises explicit
removal/reentry before those ticks. These are lifecycle fixtures, not recovered
gameplay triggers. The recipe child is not restarted by `--loop`; explicit
layers and the parent retain their prior preview restart policy. Select the
recipe or explicit layers, and use a NORMAL-printer recipe configuration.

Run `python3 tools/test-attachment-recipe-scene.py`; see
[contract, evidence and limitations](../../research/runtime/native-attachment-recipe.md).


Queue ordering is now applied before drawing. Default world inputs are synthetic:
actor i uses `(x=32*i,y=0,height=0,priority=0)`, body bias 6, first child bias 8
and second child bias 9. The second bias and explicit-layer roles are preview
policies; the selected original body/mode-one path supplies the 6/8 relationship.
Screen anchors and ANI/SPR offsets do not become world-depth coordinates.

Use explicit queue inputs and overlapping anchors to inspect ordering:

```bash
working/build/sprite-scene/mnm-sprite-scene-preview \
  --root working/game-clean --ani Creatures/redcap.ani --sequences 0,4 \
  --queue-position 0,0,499,0 --queue-position 0,0,500,0 \
  --placement-view 1 --overlap --ticks 16
```

Repeat `--queue-position x,y,height,priority` once per actor; values are signed
32-bit except height must be nonnegative. `--overlap` places pixel anchors at
(256,190); it does not change depth inputs. Exports include sorted actor/asset/
frame IDs and signed keys in each frame's `draw_queue`. The same view value
selects the recovered placement adjustment and queue rotation, without claiming
world projection recovery.

Reproduce queue/complete-frame comparisons after building the preview:

```bash
python3 tools/test-sprite-queue.py
python3 tools/test-sprite-queue-scene.py --reference working/tests/sprite-queue/<run-directory>/report.json
```

The first command prints its run directory. See
[queue contract](../../research/runtime/sprite-queue-order.md) and
[scene evidence](../../research/runtime/native-sprite-queue-scene.md).


The recovered SPR bitmask visibility pass is available as an explicit option:

```bash
working/build/sprite-scene/mnm-sprite-scene-preview \
  --root working/game-clean --ani Creatures/redcap.ani --sequences 0,4 \
  --layer Creatures/bat.ani,0,1 --layer Creatures/eye.ani,0,2 \
  --overlap --visibility --ticks 16
```

`--visibility` clears a fresh recovered coverage grid per frame and applies the
original reverse pass after sorting. Hidden records remain in exported
`draw_queue` with kind -2 and are skipped by drawing; other preview draws use
normal kind 0. The first queue entry is intentionally unprocessed. Missing SPR
auxiliary planes contribute no visibility shape. This does not generate masks
from pixel transparency or recover original activation/terrain submission.

`--visibility-expanded` requires `--visibility` and selects original 800/632
projection/bounds instead of 640/512; it does not resize the preview canvas.
There are no terrain-owner links in these creature/example-layer previews.
The model's resolved owner flags are validated independently.

Reproduce installed complete-frame checks against original visibility results:

```bash
python3 tools/test-sprite-visibility.py
python3 tools/test-sprite-visibility-scene.py \
  --queue-reference working/tests/sprite-queue/<queue-run>/report.json \
  --visibility-reference working/tests/sprite-visibility/<visibility-run>/report.json
```

Use the run directories printed by the respective original comparison tools.
See [visibility contract](../../research/runtime/sprite-visibility.md) and
[native evidence](../../research/runtime/native-sprite-visibility-scene.md).
