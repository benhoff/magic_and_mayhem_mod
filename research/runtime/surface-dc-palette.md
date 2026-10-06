# Indexed Surface2 bitmap reload and separate DC clipping

## Independent execution and scope

Two serialized reserved Wine 11.16 Surface2 runs exercise real GetDC,
StretchDIBits, GetDIBColorTable, GetPixel, ReleaseDC and native Lock readback
on 8×6 offscreen system-memory targets under normal cooperative mode.
No original game instructions execute. Immutable manifests pass before and
after both successful runs. A follow-up attempt refused before setup because
another Wine session was active; it started successfully after that session ended.

The [first capture](surface-dc-palette-capture-20261006.json) retains 360 cases:
six previously captured original loader submissions, indexed8/RGB565/RGB32
targets, absent/grayscale/reversed-grayscale/3:3:2-cube destination palettes,
and ten combinations of DirectDraw and application GDI clipping. Sources include
1/4/8-bit palettes, 24-bit pixels, sequential-offset-gap and installed cursor
inputs. The original helper addresses identify only the already captured input
boundary. Large source images are cropped to the small target in these cases.

The [identity capture](surface-dc-palette-identity-capture-20261006.json) retains
150 indexed-target cases from four owned inputs: cube and gray palettes with
duplicate black entries, an identity gray palette, and 24-bit bin-boundary colors.
It adds an exact-source destination palette. Inputs are stored alongside the
independent driver outputs. No native output or expected pixel enters either
probe. Each record retains the actual DC color table, initial/clipped/cleared
DC GetPixel frames, selected clip box/region complexity, operation results and
post-ReleaseDC Lock words. RGB32 unused high bytes are excluded from RGB
comparison; raw DC samples remain COLORREF (0x00BBGGRR).

## Confirmed clipping and palette observations

Attached DirectDraw clippers with one/two regions, missing lists or empty lists
do not constrain these GetDC bitmap writes. GetDC succeeds and its clip box
remains the complete target. These results differ from the previously recorded
Blt/BltFast clipper contract, which remains a separate implementation.

Explicit SelectClipRgn application regions do constrain the write: one region,
two disjoint regions, an empty region and overlapping rectangles forming a union.
An empty region leaves all native bytes unchanged. The combined case shows
that an application region still applies when a DirectDraw clipper is attached.
StretchDIBits returns the source height even for the empty region; GdiFlush
succeeds. Clearing the application region reveals unchanged borders and writes.
GetPixel returns the same full-frame values before and after clearing the region
in this Wine implementation; none is CLR_INVALID. This measured result differs
from Microsoft's [GetPixel clipping documentation](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getpixel)
and is scoped to this driver. [SelectClipRgn](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-selectcliprgn)
describes copying the supplied application region and clearing it with a null
handle; those API mechanics inform probe setup, not portable pixel guarantees.

For installed palettes, the actual 256-entry DC table matches the supplied RGB
entries. Translation from an indexed source uses full RGB precision and the
first entry with minimum squared RGB distance. A 24-bit source instead matches
nearest color after rounding each channel to its 5-bit-bin center:
`(channel & 0xf8) + 4`. That model fits every captured pixel, including dedicated
boundary inputs. A lookup-table mechanism is an inference from these outputs;
no driver instructions were reconstructed and no Windows-wide rule is claimed.

When all 256 source and destination palette colors match, 8-bit source indices
are preserved, including index255 when it duplicates index0's black color.
When translating to a different palette, duplicate-color ties choose the first
index. Color equality alone must not erase source index identity.

The 60 absent-palette cases are retained as observations. Their default DC table
is environment state, not a supplied palette; converted writes and RGB colors
are excluded from input-derived equivalence. Untouched regions, operation
admission and descriptor/clip-box observations are checked independently.

## Native implementation and evidence

`GlBlitter::reloadDib` now accepts indexed8 destinations whose complete palette
has been explicitly installed. `setPalette` mirrors successful RGB updates in
owned CPU state, separate from native pixel storage. Partial initialization
refuses reload; partial updates after full initialization affect subsequent
translation. Palette metadata stays with its surface when pixel storage swaps.
Identical full 8-bit source palettes preserve indices; other translations use
full RGB or the measured 24-bit bin-center rule. Per-call color caching bounds
repeated nearest-color searches to source palette entries or 32³ bins.

The optional owned DC-region argument is independent of `setClipper` state:
nullopt writes the cropped source rectangle, an empty vector writes nothing,
and up to32 bounded rectangles write their union. Overlap writes identical pixels.
Negative, empty/reversed or out-of-target rectangles and excess region counts
refuse before any writes. The native API intentionally limits geometry rather
than claiming every GDI region admission branch. Only written pixels become
known; clipped holes and larger borders remain undefined after invalidation.
Source sampling and full read/CPU/GPU presentation refuse unknown pixels.
No native COM/DC lease or lock lifecycle is introduced.

- [First offline comparison](surface-dc-palette-offline-20261006.json): 300
  explicit-palette/RGB cases match14,400 native words and14,400 full DC RGB
  values, plus14,400 clipped DC samples. The60 absent-palette conversions are
  excluded. Ten mutation/provenance tests pass.
- [Identity offline comparison](surface-dc-palette-identity-offline-20261006.json):
  all150 cases match7,200 native words and7,200 RGB values, plus clipped samples.
  Five tests check identity, tie order, boundary/clipping and color-table mutation.
- [Current native comparison](native-dc-palette-20261006.json): 450 cases match
  43,200 independent native/DC pixels;13,995 defined-pixel checks and9,510
  refusals cover clipped validity and palette initialization. Partial palette
  updates and storage-swap ownership also pass native policy checks.
- [RGB565/32 compatibility](native-ddraw-after-dc-palette-20261006.json): all72
  cases/2,693,856 independent pixels,24 scripted cursor compositions/253 refusals,
  and1,456 original dispatch projections/12 exact buffers/813 budget stops pass.
- [RGB24/32 compatibility](native-cursor-after-dc-palette-20261006.json):36 cases,
  1,346,928 independent GDI RGB comparisons,12 compositions/137 refusals pass.
- [Focused regressions](dc-palette-regressions-20261006.json): nine renderer tests
  and15 offline mutation tests pass.

The [initial native result](native-dc-palette-initial-20261006.json) preserves
its earlier source hashes and300-case outcome. It predates identity preservation
and the extra update/swap tests; it does not claim current-source validation.
Earlier surface reports also retain their historical fingerprints and scopes.

## Reproduction and remaining work

```sh
python3 tools/check-surface-dc-palette.py
python3 tools/check-surface-dc-palette-identity.py
python3 -B tests/test-surface-dc-palette.py
python3 -B tests/test-surface-dc-palette-identity.py
cmake --build working/build/ddraw-dib --target mnm-renderer
xvfb-run -a python3 tools/check-native-dc-palette.py --identity \
  --report working/tests/new-native-dc-palette.json
```

Offline checks need only Python and retained fixtures. Native checks need Qt/GL
and no Wine or original installation. Each collector uses new report/corpus
paths, a disposable prefix and the shared single-Wine reservation.

Default/absent palette conversion, flag-bearing palette semantics, logical GDI
palette selection, shared COM palette identity, GetDC palette/clip changes across
lease reuse, arbitrary/out-of-bounds regions, origins/stretch/other formats,
real original lost-draw/Restore composition, Windows hardware and live/wire
recovery remain pending. Indexed cursor recovery is not claimed by the existing
RGB scripted compositions. The runtime CPU frame adapter's earlier RGB565 floor
policy remains a separate pending bridge-color contract.
