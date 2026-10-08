# Native complete World canvas comparison

Scope: No-CD executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
preferred image base `0x00400000`, selected single-player Quick Battle World
consumer output. This increment implements owned effective raster input capture,
strict asset binding and native OpenGL RGB565 composition. Continuous native
presentation and whole-consumer bypass remain outstanding. The World canvas is
observed immediately after its consumer returns, before later HUD/presentation
work; it is not the complete game window or a supported-session result.

## Recovered inputs and raster contracts

The observer at `0x5002a0` retains the original queue consumer. Its ECX queue
contains base/count/capacity at offsets 0/8/12 and 36-byte records. Primitive
entry hooks record the consumer's effective anchors, clipping, selected colours
and order while the original routines run. A return hook closes the observation
and writes the original canvas to a separate oracle. Native rendering accepts
the [owned wire format](../formats/world-frame-v1.md) and read-only pinned SPR
assets; it never accepts original destination pixels as rendering input.

| Captured route | Original entry/slot | Recovered scope |
| --- | --- | --- |
| Black | `0x595677` | Coverage writes zero; horizontal clipping returns 1 without pixels |
| Generic indexed | `0x5947b2` / `0x59521a`, slot `0x6c4e3c` | Selected WORD colour tables, including internal horizontal clipping |
| Terrain | `0x595b47` / `0x59603e`, slot `0x689b7c` | Selected DWORD colour-table entries, low WORD output, internal horizontal clipping |
| Generic clipped | `0x57de00` | Selected indexed RGB565 clipping/fallback requests |
| Half / quarter-destination / quarter-source | `0x57ec90` / `0x57f0f0` / `0x57f5f0` | Masked integer RGB565 composition |
| Wave | `0x5806f0` | Observed nonnegative offset tables, admitted amplitude 1..8; horizontal clipping uses the original fixed triangle |
| Direct word | `0x597086` | Observed direct RGB565 input; not exercised by the indexed live corpus |
| Distortion | `0x596490` / `0x5968a4`, slot `0x689b78` | Observed sixteen-row rightward displacement; horizontal clipping returns 1 |
| Projected shadow | `0x57e540` | Odd-height/alternating source-row coverage and destination darkening |

Palette-chain nodes have next/shift/neutral DWORDs at offsets 0/4/8. After
walking the requested ordinal, the selected table is neutral plus arithmetic
right-shifted shade, with the original negative correction when shift is nonzero.
Generic tables are 512 bytes; terrain tables are 1024 bytes. Confusing these
layouts produced the first failed comparison. Shade -127 selects black.
Native inputs own the resulting 256 WORDs, without reconstructing the live
palette generator or assuming palette addresses remain stable across launches.

Let `H(v)=(v>>1)&0x7bef`. Half is `H(source)+H(destination)`.
Quarter-source is `H(destination)+H(H(destination))+
((source>>2)&(0x7bef>>1)&0x7bef)`; quarter-destination swaps source/destination.
GPU integer textures preserve these values exactly. Coverage is separate from
colour, so opaque index zero and opaque black still draw. Displacement freezes
the preceding native destination on the GPU and samples rightward by the owned
offset for each destination row; no original pixels or CPU readback supply it.

Projected shadow starts at `anchorY-originY+floor(height/2)`. After vertical
clipping, it selects every other source row, shifts horizontally by half the
last retained source-row limit and moves left one pixel per output row. Covered
destination pixels become `H(destination)`. Its original route refuses
horizontal clipping as a whole. Native CPU asset preparation builds this derived
coverage mask; the GPU performs destination darkening.

The consumer's first dispatch table at `0x501184` has selected links for kinds
0/31 (ordinary indexed), 1 (shadow), 2 (compound sprite), 3 (quarter-source),
4 (half), 5 (quarter-destination), 6 (selected distortion) and 22 (wave amplitude
8). Kind 33 selects terrain before the table; -2 is hidden. Three computed
dispatch tables, other values, alternate consumer modes, readbacks and consumer
side effects remain incomplete. The adapter refuses unknown queue kinds rather
than silently declaring a partial image complete. These links describe selected
branches and do not recover entire functions or dispatch tables.

## Reproduction and validation

```bash
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/capture-scene-game.py --world-frames \
  --samples 4 --skip-queues 120 --interval 10

xvfb-run -a env LIBGL_ALWAYS_SOFTWARE=1 QT_QPA_PLATFORM=xcb \
  python3 tools/test-world-frames.py --capture-report CAPTURE_REPORT
```

The comparison tool builds `mnm-world-frame-preview`, exercises native wire,
palette, resource-cache and scene tests, then runs 18 independent original/native
fixtures across six raster modes and three clip cases. It binds every captured
frame by normalized encoded identity to pinned installed assets. Native output
is compared with independently executed unmodified original raster routines and
with every WORD of the separately captured live consumer output. Isolated
execution uses owned effective colour/offset inputs; live output supplies the
independent check of observation completeness. The private reference process
does not execute the whole queue consumer.

The private original copy reference calls generic entry `0x57e1b0`. The pinned
Ghidra inventory does not assign this entry to a discovered function. The
comparison therefore verifies its original eight-byte signature
`83ec1453558bc156` and registers only `0x57e1b0..0x57e1b8` as an explicit
recovered entry range. This retains the inventory gap and does not establish
a complete function boundary or an exclusion from further recovery.

Immutable results are registered separately as the
[World capture](native-world-frame-capture-20261007.json) and
[whole-canvas comparison](native-world-frame-comparison-anchored-20261007.json).
Their assertions and source hashes, rather than this protocol's existence,
establish the achieved scope. Earlier unsuccessful captures/comparisons remain
under their original working run paths. One intermediate trace matched native
and isolated original execution but missed 32,948 live border pixels because it
incorrectly omitted the terrain backend's internal clipping branch. The capture
was corrected; that failed result cannot stand in for a complete-frame pass.

Each run retains input/source/binary hashes, native PNG/WORD output, separate
original output and reports under `working/tests/world-frames/run-*`. Tools
verify all 2,927 immutable original files before and after each experiment.
The native CLI's `--background` is explicit owned policy; baseline comparisons
use zero and cannot read `.before.565` or `.after.565` as native inputs.

## Boundaries and next milestone

This is finite closed-file World rendering. Hooks assume the original engine
thread, fixed selected backends and pinned preferred image base; reentry,
backend changes, exception unwinding, unload and sustained shutdown need their
own execution evidence. Input budgets, renderer budgets and unsupported routes
are refusals, not hidden fallbacks. The native CLI destroys its owned surfaces
after export. Current rendering tests use Xvfb/Mesa llvmpipe, not a physical GPU.

Next: publish owned inputs continuously with frame identity, resource lifetime,
backpressure and transitions; render them through the existing Qt GPU viewport;
compare complete frames in shadow mode before bypassing consumer raster work.
Retain consumer mutations and required picking/readback state when introducing
that bypass. Expand camera actions, active battles, HUD/minimap/text/cursor,
movies, menus, save/load, recovery and shutdown before a complete launcher mode.
Gameplay balance and simulation timing are unchanged.

Shared renderer and observer edits leave older linked evidence historical/stale.
Prior direct-word replacement and surface/scene records retain their original
hashes and statuses; these new tests do not renew every older comparison.
