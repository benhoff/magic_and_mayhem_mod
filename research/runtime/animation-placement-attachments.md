# ANI sprite placement and attachment points

Selected No-CD contract, pinned to executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This is offline reconstruction and bounded native preview work. It does not
replace a live creature, attachment lifecycle or world-rendering pipeline.

## Recovered fields and routines

Version-5 ANI records are 44 bytes. For a displayed opcode-0 SPR record:

| Record offset | Interpretation in selected original callers |
| --- | --- |
| +4 | SPR frame index |
| +8 / +12 | Signed sprite displacement in pixels |
| +16..+23 | Eight retained name bytes; not used by these placement helpers |
| +24 | Retained opaque word; semantics not recovered here |
| +28 / +32 | First attachment point, signed pixel displacement |
| +36 / +40 | Second attachment point, signed pixel displacement |

The first attachment point is read by `0x00507250`; the second by
`0x005072c0`. Body descriptor helper `0x00507190` reads displayed record at
creature `+0xb4` and emits `(SPR-frame pointer, x, y)`. If the displayed pointer
is null, it starts sequence `288 + facing` before reading it; that fallback
selection is static evidence and is **not executed or implemented** in this
milestone. Overlay descriptor helper `0x00507330` reads record `+0x829`
(controller at `+0x825`) and emits the same descriptor.

All four helpers apply one adjustment based on type-record DWORD `+8`, reached
through creature pointer `+0xac`. Configuration loading at
`0x00502df0..0x00502e62` reads the `TileSizeXY` key and stores its clamped 1..2
value at type-record `+8`. This is the footprint size, not the creature type ID.

For footprint size 2, raw view global `0x006e9850` produces:

| View value | x adjustment | y adjustment |
| --- | ---: | ---: |
| 0 / other | 0 | 0 |
| 1 | +32 | -16 |
| 2 | 0 | -32 |
| 3 | -32 | -16 |

Other footprint values leave the coordinates unchanged. Arithmetic wraps in
32-bit x86 words; interpreting the result as signed preserves negative values.
`placement.cpp` uses unsigned arithmetic and a bit-preserving signed conversion
so boundary values never rely on undefined C++ signed overflow. No compass
labels, world rotation or camera projection are inferred from this table.

## How the selected draw caller uses them

The creature draw region calls the first attachment helper at `0x004fa9d0`.
It obtains child descriptor displacement from child displayed ANI `+8/+12`,
then queues the child using **creature base + attachment point + child offset**
(`0x004faaf0..0x004fab3c`). It does not add the parent's sprite displacement to
that attachment point. A separate path calls the second helper at
`0x004fb1e8`; the non-size-2 branch at `0x004fb1f3` instead adds half the
SPR height to the first attachment Y. The preview's explicitly selected second
point at footprint 1 does not reproduce that special consumer branch. These
callers establish selected attachment consumption, not a complete inventory
of attached creature/effect roles or every exceptional placement path.

Body placement is **creature base + body offset**. The sprite draw routine then
subtracts the SPR frame's own signed origins. ANI placement and SPR origins
are separate operations. The prior preview omitted ANI offsets; the current
preview applies them for every visible body frame, even without child layers.

The recovered helpers do not decide which asset, action or child controller
belongs to a creature. Gameplay event handling, equipment/effect admission,
attachment ownership, complete original sorting/depth/occlusion, terrain and
world projection remain outside this milestone.

## Native model and preview policies

`reconstruction/animation/placement.hpp` exposes `spriteOffset` and
`attachmentOffset`, independent of Qt or application widgets. They require a
sprite record; applying placement to a stop/control record is rejected.
`NoCdAnimationPlayer::displayedRecord` returns an owned snapshot of the selected
record, safe across switches, restarts and player destruction.

The application owns one primary ANI/SPR pair and up to two explicit child
pairs. Each actor has independent forward child players. Children attach to
one of the parent's two points and add their own unadjusted ANI sprite offset;
only the parent footprint adjustment is applied to the attachment point.
Children become invisible when the parent has no displayed sprite. Child
players continue ticking independently and follow the explicit `--loop`
preview restart policy. Selecting a body action group does not infer or select
child actions. These are bounded preview policies, not recovered gameplay rules.

Drawing order is actor order, then body, first configured layer, second
configured layer. It is an explicit composition order, not the original world
queue sort. One shared LRU cache is keyed by **asset and frame** and retains
at most 24 uploads / 50 total renderer handles. Equal frame indices in distinct
SPR assets cannot share the wrong pixels. Pixel budgets and fully in-bounds
rendering requirements remain enforced by the renderer; clipping is unsupported.
Anchor sums use widened arithmetic and reject values outside native signed
coordinate bounds instead of overflowing. This is a native safety policy;
the placement helper model itself retains the original 32-bit wrap behavior.

```bash
python3 tools/test-animation-placement.py
python3 tools/test-animation-layers.py
working/build/sprite-scene/mnm-sprite-scene-preview \
  --root working/game-clean --ani 'Creatures\redcap.ani' --sequences 0,4 \
  --layer 'Creatures\bat.ani,0,1' --layer 'Creatures\eye.ani,0,2' \
  --tile-size 2 --placement-view 1 --ticks 32 --loop
```

This example deliberately attaches independent installed assets to illustrate
both points. It does not assert that RedCap owns Bat/Eye attachments in the game.
The default footprint is 1 and view is 0; changing them adjusts placement only.
`--layer ANI,sequence,slot` may be repeated twice; slots are 1 or 2. Child SPR
names resolve as ANI sibling basenames through case-insensitive AssetFile paths.
Comma-containing layer paths require a future structured manifest interface.
`--export-dir NEW_DIRECTORY` retains per-frame body/layer draw anchors, selected
SPR indices, raw events and native/presentation hashes. The usual bounded
clock, facing-switch and export protection options remain available.

## Evidence and confidence

The helper runner verifies the original manifest before/after, privately maps
the unchanged PE32 image, and calls the four selected routines with bounded
fixture objects. The descriptor's displayed record is always present. Original
SPR pointer selection is checked using a fixture table; real sprite loading,
Win32 APIs and the null-display fallback are not executed. Original code bytes
are never patched. All version-5 installed ANI sprite records are visited;
seeded/random boundary fixtures additionally test signed wrapping.

The layered runner independently obtains original forward controller traces,
reads the displayed records by original relative indices, and composes checked
SPR rows using the recovered offsets. It verifies complete GPU RGB565 frames
and RGBA expansion. This is original helper/selection evidence plus an
independent whole-frame CPU oracle, **not original whole-scene rendering**.

High confidence applies to the selected field reads and adjustment arithmetic.
The selected attachment composition is backed by static callers and native
pixel comparisons; complete attachment roles, original layering rules and live
behavior remain unverified.

Final helper evidence: [retained report](animation-placement-attachments.json),
`working/tests/animation-placement/run-g39x6r2f`. Across 133 installed version-5
ANI files, 63,382 sprite records plus 133 synthetic boundary/random records
produce **6,097,440 matching helper results**. All four routines match for
footprints 0/1/2/3 and view values 0/1/2/3/4/0xffffffff.

Final layered evidence: [retained report](native-animation-layers.json),
`working/tests/animation-layers/run-35sc0xoc`: **198 frames, 396 body states,
792 layer states and 40 distinct SPR frames**, all matching. The six runs cover
all four size-2 views, explicit restart and a phase-preserving facing change.
Window smoke, third-layer rejection and resource cleanup also pass. The
updated body-only runner at `working/tests/sprite-scene/run-7qkol_33` passes
195 frames / 390 states / 30 distinct SPR frames with ANI displacement applied.

Native placement, snapshot ownership and layered scene fixtures pass both
normal and ASan/UBSan builds (leak detection disabled for this environment).
The original manifest passes before/after each original-consuming runner: all
2,927 immutable inputs remain unchanged. No live game replacement was attempted.
