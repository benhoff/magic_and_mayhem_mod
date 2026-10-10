# Owned four-orientation minimap terrain

`RS.minimap-terrain-raster` isolates the terrain pixel portion of No-CD
`0x553a40` from World batching, launchers and application widgets. The standalone
`renderer/minimap` library accepts owned RGB565 destination storage, source cell
colours/visibility and optional owned auxiliary history. It neither reads original
output pixels nor depends on game pointers, Qt or reconstructed simulation.

## Recovered boundary

The original routine uses object `+0x29` for 16-byte cell descriptors (state
pointer `+8`, RGB565 colour `+12`), `+0xc5` for orientation, `+0xe7` for destination
root, `+0xeb` for destination word stride, and `+0x6199/0x619d` for grid dimensions.
`+0x61a1/0x61a5` hold source centres; `+0x61a9` is the auxiliary surface wrapper,
with byte pitch at `+0x61c1`. Fog enable is `0x5e1404`; signed state bytes are
compared with `0x5e18f0`. Addresses are preferred VAs for the pinned No-CD build,
not persistent runtime pointers.

Fog/non-fog dispatch tables are `0x554810` and `0x554820`. Their four entries are
respectively `553aa0/553c72/553e75/55404f` and
`554250/5543a7/55451d/55468e`. Retain the surrounding undiscovered ranges; these
links describe terrain placement, not whole minimap or UI completeness.

For source row `r`, column `c`, even grid width `w`, destination coordinates
relative to the root are:

| Orientation | X | Y |
| --- | --- | --- |
| 0 | `c - 2*floor(r/2)` | `ceil(r/2) + floor(c/2)` |
| 1 | `-w + 2 + c + 2*floor(r/2)` | `w/2 - 1 + ceil(r/2) - floor(c/2)` |
| 2 | `1 - c + 2*floor(r/2)` | `w - 1 - ceil(r/2) - floor(c/2)` |
| 3 | `w - 1 - c - 2*floor(r/2)` | `w/2 - ceil(r/2) + floor(c/2)` |

Source starts at wrapped `(centerX - w/2 + (orientation & 1),
centerY - floor(h/2))`. Each loop increments source columns/rows with wrapping;
it does not rotate an already composed bitmap. Hidden cells read the same
placement relative to auxiliary root `(w-1, 0)`, independently of destination
root and stride. Visible cells read descriptor colours. Prior auxiliary pixels
must be owned native history in a future live adapter.

The original routine invalidates object `+0xff` to `-2`, locks the auxiliary
surface and unlocks once after drawing. The comparison fixture checks those
effects and otherwise unchanged object/cell/auxiliary storage. The native raster
library implements pixels only. A future bypass adapter must preserve these
effects and separately recover actual Lock/Unlock failure semantics. No live
bypass or driver compatibility follows from the isolated comparison.

## Native admission policy

Widths are even `2..256`, heights `1..256`, orientations `0..3`, and centres lie
inside the source grid. Destinations and history use owned positive word strides;
all addressed words must lie inside logical extents. Visibility is a closed
boolean plane. Missing/aliased history, invalid planes, unsupported inputs or
out-of-bounds placement refuse before modifying any destination word. These
refusals and atomicity are intentional native policy; the original routine has
no equivalent clipping/safety contract. Odd widths, arbitrary unnormalized
centres, negative pitches and partial clipping remain unsupported.

## Validation and reproduction

The isolated reference maps the hash-pinned original PE privately and replaces
only the signature-checked `0x58b660` Lock boundary with an owned auxiliary pointer.
The terrain routine and dispatch tables execute unchanged. Unlock is a synthetic
COM success boundary. Original files remain read-only; the runner checks the
2,927-file manifest before and after execution. Native inputs are generated
source colours, visibility and separate prior-history patterns, never original
completion pixels. Compare complete destination allocations including untouched
regions and seven padding words per row.

```sh
python3 tools/draft-coverage-claims.py --behavior RS.minimap-terrain-raster \
  --output working/tests/minimap-new-claims.json
python3 tools/test-minimap-terrain.py --claims working/tests/minimap-new-claims.json
```

The runner builds normal and ASan/UBSan native fixtures, checks atomic refusals,
and compares four rotations, all small-grid centres, larger boundary centres,
odd/even heights, and fog disabled/mixed/all hidden/enabled-all visible. Full
execution reports and raw outputs are retained in new `working/tests/minimap-terrain`
directories. Evidence and confidence apply only to the recorded corpus.

Existing live producer wire/capture/replay still admits orientation 0. This
standalone module does not alter that protocol, historical evidence, launchers
or batch integration. Marker/camera-outline geometry, changing HUD/tooltip state,
resource lifecycle, sustained gameplay and actual drawing suppression remain
separate work. Independent offline comparisons precede that integration.

## Recorded result — 2026-10-09

[The source/scope-bound comparison](native-minimap-terrain-comparison-20261009.json)
passes all **1,360 cases / 35,486,800 destination words**, with zero differences
between the actual original routine, normal native implementation and ASan/UBSan
implementation. Fifteen invalid-input/history/bounds refusals preserve complete
destination storage. Original source descriptors and auxiliary storage stay
unchanged; the object changes only at `+0xff`, and each call unlocks once.
All nine declared implementation/test/build dependencies remain stable and both
original manifest checks pass. Confidence is high within this isolated corpus.
The initial standalone CMake path failure is retained at
`working/tests/minimap-terrain/run-r4fsttyt/report.json`; it is not pixel evidence.
No existing producer, batch or launcher source was edited for this increment.
