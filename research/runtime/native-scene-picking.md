# Native scene mouse picking (NS14)

Reviewed 2026-10-05. `NP.scene-picking` is an intentional native diagnostic
policy. It does not recover original mouse bindings, terrain rays or pointer
ownership, and it changes no balance rules or original artifacts.

## Contract

Left-click selects the foremost opaque creature pixel in the **presented queue**.
The picker walks that queue backwards, using signed sprite origins, frame extents
and decoded opacity masks. Transparent holes reveal earlier draws; opaque terrain
occludes creatures. RGB/index zero can still be opaque. Equal depth is resolved
by the actual presented ordering, without a second sort. Empty/background clicks
clear transient selection. The full unsigned native slot/generation identity is
validated by the existing Orders controller; old-frame identities cannot select or
order a replacement after release/reuse. Selection does not advance the simulation.
The selection-outline decoration is not part of the hit mask.

Right-click ignores creature bodies and picks the foremost opaque **terrain** draw.
All body/overlay records for a tile carry its explicitly declared standing cell.
Diagnostic tiles retain the caller's standing layer, independently of fine height;
ordinary MAP tiles declare `(x,y,z+1)` when that layer exists. An opaque terrain
draw without a declared cell blocks picking through to hidden terrain. A miss, an
unsupported top tile or no selection queues nothing. Picking a visible tile's art
chooses its parent cell; this is not geometric floor-only or original ray picking.
Reachability, footprints and occupancy remain decisions of the native planner.

Composition checks declared cell bounds and its XY agreement with the tile. The
cell is retained through sorting and exported as optional `cell` metadata. The
interactive view caches the last presented queue, and both mouse and numeric-field
orders use the existing semantic controller path. Successful terrain picking updates
the target fields and queues a move; Step applies it. Save preserves pending moves.
Pick/selection metadata and canvas state are not checkpoint fields.

The QWidget canvas owns no world, sprite data or adapter. It displays a fixed
512x256 logical image without margins/scaling, maps fractional event coordinates
by flooring, and ignores clicks outside the viewport or unsupported buttons. Native
picking operates on owned decoded sprites and at most 12,320 draws, with safe signed
coordinate arithmetic, frame/mask extent checks and binary coverage checks. Error
handling stays behind the application action callbacks.

## Validation

```bash
python3 tools/test-scene-picking.py BUILD/world-picking-test NEW_OUTPUT
```

Both normal and ASan/UBSan component runs compare 4,718,592 picks against an
independent forward pixel-ownership oracle over complete viewports, overlapping
actors/terrain, transparent holes, equal depth and clipping at canvas edges.
Additional checks cover terrain under actors, opaque unsupported-cell occlusion,
invalid frames/masks, budget/coordinate limits, explicit layers in four actual
compositions and malformed standing-cell refusal. A synthetic CPU/OpenGL frame is
presented by the production canvas; actual Qt left/right/middle mouse events verify
fractional/bounded dispatch, full-generation selection, selected-only queuing,
no implicit tick, misses/no selection, stale-frame release/reuse and target fields.
A saved mouse order resumes in a fresh process to exactly the uninterrupted bytes.
Inputs in these component tests are owned synthetic data.

Updated scene executables also pass the existing multi-creature composition runner
in normal and sanitizer builds: 144 complete CPU/OpenGL pixel comparisons and
twelve exact JSON/RGB565/PNG/checkpoint continuations per build, over three terrain
profiles and four views. Its installed SPR/TTD reads are bracketed by original
manifest verification. This checks preserved diagnostic composition after metadata
changes; it is not an original mouse-picking comparison. The normal suite passes
106 CTests and six focused sanitizer suites pass.

The accepted [record](native-scene-picking.json) pins fresh reports, current source
fingerprints and test/manifest logs. [Committed-history review](native-scene-picking-history-review.json)
and its [extension](native-scene-picking-history-extension.json) check exact intermediate receipts separately without asserting a past gate or new
runtime equivalence. Historical shared-source scene/control evidence retains its
fingerprints and may remain stale. Main-window callback wiring and ordinary-map
cell annotation are reviewed/compiled; installed whole-window mouse interaction
and original ray/selection equivalence are not claimed.

## Remaining boundaries

Original pointer/ray/input mappings, geometric floor-only picking, group selection,
stop/cancel controls, faction permissions, automatic playback, commander/summoned
creature gameplay, mixed profiles, lighting/visibility/attachments and live
replacement remain open.
