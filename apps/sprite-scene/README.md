# Native ANI/SPR scene preview

Bounded native scene application, separate from the Qt game shell and live hooks.
Loads version-5 ANI and version-4 SPR through AssetFile, uses the recovered forward
controller for explicit sequences, and composes mask/RGB565 uploads in OpenGL.
The original executable and Wine are only needed by research comparisons, not
by this preview application.

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
