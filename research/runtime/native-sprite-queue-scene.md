# Native sprite queue scene integration

The Qt preview now collects visible body/child draws, computes selected NoCD
depth keys, applies the [exact original queue permutation](sprite-queue-order.md)
and draws in ascending queue order. Renderer surfaces, asset loading and widgets
remain independent of the build-specific model; the scene owns orchestration.
No Wine, live game or runtime hook was used.

## Inputs and policies

`SpriteScene::setQueueInput` supplies per-actor x/y/height/base priority and
three independent priority biases. Default synthetic inputs use x=32*actor,
y=0, height=0 and base priority=0. Body/first/second biases are 6/8/9.
The selected original body/mode-one path supports the 6/8 relationship;
explicit child roles, the second bias and synthetic actor placement remain
preview policies. Priority addition preserves 32-bit wrap. Negative height
and invalid view are rejected before queue input changes.

Screen placement is independent: `setAnchor` changes pixels; ANI displacement,
parent attachment points and each SPR origin determine draw placement.
`--queue-position x,y,height,priority` repeats once per actor, while `--overlap`
uses pixel anchors (256,190). The existing raw view selects both queue rotation
and the footprint adjustment. It does not recover world projection.

Visible records enter the queue in actor order, body then configured children.
The original unstable tie permutation therefore depends on that explicit
submission order. Complete original producer order is not inferred. Each draw
owns its actor/asset/frame IDs and coordinates; uploads retain asset ownership,
24-entry eviction and per-frame background clearing. Exported `draw_queue`
records sorted actor/asset/frame IDs and signed depth keys.

## Offline checks

After building `apps/sprite-scene`, run:

```bash
python3 tools/test-sprite-queue.py
python3 tools/test-sprite-queue-scene.py --reference working/tests/sprite-queue/<run-directory>/report.json
```

The queue comparison prints the run directory supplying the reference report.
The scene runner checks its pinned executable and source/input hashes, verifies
immutable input manifests before/after and runs only native Qt/OpenGL under
Xvfb/software rendering. Original code runs only in the earlier bounded queue
comparison, with unchanged instructions and no Win32 calls.

Twenty-four fixtures cover four views, ascending/reversed actor depths, equal
actor depths, body/child cross ties, heights around 499/500 and priority wrapping
across the signed boundary. Original builder/sorter results supply order and
keys; a separate CPU compositor supplies complete expected pixels.

The synthetic fixture composes four actors and two independent child assets,
with transparent masks, opaque zero and independent signed SPR origins.
Seventy-two complete frames match: two displayed frames plus one cleared frame
per fixture. Order IDs and keys, rejected input and zero final resource ownership
are checked. Installed fixtures overlap RedCap bodies (sequences 0/4), Bat and
Eye example layers (sequence 0) at one screen anchor; twenty-four initial complete
RGB565 frames and RGBA presentation hashes match the independent decoder/oracle.
They are explicit example layers, not recovered creature recipes.

[Machine-readable evidence](native-sprite-queue-scene.json) records the normal
and sanitized comparisons and artifact directories. All seven targeted CTests
pass normally and under ASan/UBSan (leak checking disabled in this environment).
The 24-fixture/72-frame original-order synthetic comparison also runs sanitized.
The existing 198-frame installed placement/layer regression remains a separate
compatibility check.

Confidence is high for selected key/sort behavior and these bounded native
frames. The CPU pixel oracle uses original queue orders, not the original
whole-scene draw routine. Original visibility pass `0x005015f0`, lighting,
terrain, all producer priorities/submission order, live clock and full native
world rendering remain unimplemented/unverified.


## Subsequent visibility milestone

The previously unimplemented visibility helper/reverse pass now has selected
offline original/native evidence and an explicit native preview option. See
[visibility reconstruction](sprite-visibility.md) and
[installed frame comparison](native-sprite-visibility-scene.md). Original terrain
role production and activation/world ownership remain separate; the queue
comparisons above describe the earlier pass-disabled milestone.
