# Ten owned native sprite transformations

This batch adds independent, widget-free sprite preparation policies to the
partial native sprite-input/presentation work. It reuses the existing SPR pixel
preparer and the N27 immutable resource/session and CPU/GPU delivery services.
These are intentional native policies, not recovered original drawing or ANI
behavior. The menu/assets parent remains partial and no live route is added.

| `NP.sprite-transform.` component | Preparation policy |
| --- | --- |
| crop | Source-local nonempty rectangle; subtract its top-left from the origin |
| trim | Minimal covered-pixel rectangle; all-transparent input becomes canonical empty |
| mirror-x | Reverse columns; origin becomes `(width-1-originX, originY)` |
| mirror-y | Reverse rows; origin becomes `(originX, height-1-originY)` |
| rotate-cw | Pixel-index quarter turn: `(x,y)` becomes `(height-1-y,x)` |
| rotate-half | Pixel-index half turn: `(width-1-x,height-1-y)` |
| rotate-ccw | Pixel-index quarter turn: `(y,width-1-x)` |
| nearest-resize | Inverse floor sampling with independently floor-scaled signed origins |
| silhouette | Replace covered color words with one RGB565 value; preserve coverage |
| outer-outline | Add one pixel of padding and a four-neighbor dilation in uncovered pixels; preserve interior color and coverage |

Each operation consumes one selected immutable source frame and returns a new,
owned, single-frame direct RGB565 Sprite. Indexed sources resolve through their
embedded palette or a caller-supplied RGB565 table consumed during preparation.
No palette pointer survives. Direct sources refuse
palette overrides. All eight name bytes survive. Encoded source offsets, run
endpoints, trailing planes, legacy palette words and palette indices are cleared:
these are derived native pixels, not a serialized SPR or original outline path.
Opaque black remains covered. Masks normalize to0/1 independently of color.

Input and output dimensions are0..2048, with zero width iff zero height and at
most1Mi pixels per plane. Input dimensions/area are checked before preparation;
output extents/origins are checked before output allocation. Origin arithmetic
uses signed64-bit intermediates and refuses results outside signed32-bit range.
Mirrors/rotations use pixel-grid coordinates, including off-image signed anchor
positions. Resize scales origins with mathematical floor, including negative
origins, and samples source pixels with floor. These conventions are explicit
native policies and do not infer a legacy pivot, filter or timing convention.

Crop requires a positive in-bounds rectangle. Resize requires positive source
and destination extents. Other operations accept canonical empty frames and
return empty output with origin zero. Trim also canonicalizes all-transparent
nonempty frames. Outline leaves original covered pixels intact, including zero,
and colors uncovered four-neighbors inside the expanded extent; this includes
hole edges and excludes diagonal-only neighbors. Padding increases each origin
by one. An expansion exceeding extent or area budgets refuses atomically.

Inputs remain unchanged on success/refusal; output planes own their storage.
There is no source-image/file/GPU pointer in derived pixels. The library depends
on assets and native sprite rendering, with the application session appearing
only in tests. Prepared frames can be loaded into a native SpriteTable and drawn
after that table/session and the original source have been retired. Existing GPU
batch contracts still apply: logical/preparation/upload refusals precede drawing,
while physical GL failure during accepted drawing can leave a prefix.

Prospective source/contract fingerprints precede Release and ASan/UBSan controls.
A forward cell-mapping oracle and independent rational resize intervals validate
all ten geometries, masks and signed origins for indexed/direct/palette-override
fixtures. Derived immutable plans then pass independent clipped CPU/GPU pixel
checks, including extreme anchors, opaque zero, holes and retired producers.
Composition identities, exact budget admission, overflows, malformed planes,
empty frames and metadata normalization are bounded controls. The existing native
sprite-session control is a supplier regression, not renewed original evidence.
No original artifacts are consumed. No original filter, tinted/encoded-run
outline, Qt widget route, world/hardware/performance or live replacement credit
is granted. Historical evidence hashes and all status axes remain independent.
