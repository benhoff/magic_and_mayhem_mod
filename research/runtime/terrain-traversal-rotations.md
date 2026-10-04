# Four-orientation terrain world traversal

This extends the [orientation-zero scene milestone](terrain-world-traversal.md)
to all four selected camera traversals, preserving their distinct loops and
shared cell ownership. Native `--world --view 0..3` now uses these recovered
contracts. Map initialization, lighting and entities remain separate.

Pinned No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The references execute unchanged instructions in a private PE32 mapping without
Win32/import calls, Wine, a game session or executable patching.

## Confirmed contracts

| View | Traversal | Starting map offset | Forward cell axis | Next row axis | Layer bound |
| --- | --- | --- | --- | --- | --- |
| 0 | `0x004f83f0` | -diagonal,-diagonal | +X | +Y | Header layer count; mode-zero cut inclusive |
| 1 | `0x004fbe00` | +diagonal,-diagonal | +Y | -X | Receiver +0x61, exclusive |
| 2 | `0x004fc3f0` | +diagonal,+diagonal | -X | -Y | Receiver +0x61, exclusive |
| 3 | `0x004fc930` | -diagonal,+diagonal | -Y | +X | Receiver +0x61, exclusive |

Correction to earlier static notes: orientation one reloads EBX from receiver
+0x5d at `0x004fbead` before computing its starting coordinates. It uses the
diagonal field; the previously suspected literal +1/-1 start was incorrect.

All four call origin helper `0x004f8230`, with base
`X=f11+trunc(f51/2)`, `Y=f15+16*(f41-2*diagonal)+trunc(f55/2)+f4d`.
For `S=f45+f49`, `D=f45-f49`, and `H=uint32(S)>>1`, corrections are:

| View | X correction | Y correction |
| --- | --- | --- |
| 0 | -D | -H |
| 1 | -S | trunc(D/2) |
| 2 | +D | +H |
| 3 | +S | -trunc(D/2) |

Native arithmetic checks overflow; original invalid/fault/overflow behavior is
not claimed. The same formulas appear in the selected audio origin helper, but
rendering reconstruction does not depend on audio services.

Anchors remain `originX+32*(column-step-row-step)` and
`originY+16*(column-step+row-step-layer)`. Starting map coordinates normalize by
one extent with no priority adjustment. Forward cell/row wraps add the respective
X/Y priority adjustment regardless of direction. Resetting the primary axis for
a new row removes its wrap priority. Traversal flag gates remain WORD +10 bit
0x4000 and WORD +8 bit 0x80 outside the supplied special layer.

Views 1..3 skip whole steps against left-64/top-48 using **floor division plus
one** when an anchor is on or outside the margin. After these skips, a row whose
starting anchor reaches right/bottom is rejected. An admitted row runs to its
span without a further right/bottom limit; viewport clipping happens later.
View zero retains its floor-only skips and far-boundary run limits. Replacing
these loops with a common geometric bounding-box test changes queues.

## Physical cell selection and boundary behavior

Recovered traversal retains both a physical cell pointer and logical coordinates
used by the terrain producer's depth/light calculations. Native visits now store
an owned physical cell index separately from those coordinate annotations.
The flag callback reads the physical cell, and the application loads its actual
MAP definition and raw flag WORDs. No legacy host pointer is retained.

Orientation two's row reset uses a strict `> width` comparison at
`0x004fc8bb`. Equality leaves logical column==width and selects a carried cell
in the next row or layer. Its next negative-X step resolves the carry normally.
This can revisit a physical cell with different logical depth coordinates.
The preview preserves the selected cell and depth rather than replacing equality
with a modulo operation. Repeated visits share producer flag changes and one
visibility owner; otherwise hidden-draw flags would depend on which visit's
private copy happened to be inspected.

In views 1/3, large clipping skips use row-offset tables for counts below the
map height, but the fallback multiplies by **height** at/above that threshold,
rather than by width. Rectangular grids can therefore select cells different
from a rotated-coordinate-only model. Executed synthetic comparisons cover
those selections. Primary rollback/remaining-step table accesses have no such
fallback; native reconstruction rejects a lookup outside the recovered rows.

The owned implementation rejects a physical cell outside the allocated grid or
logical coordinates outside the inspected domain (column==width is admitted).
An exploratory unguarded original reference faulted at a last-layer carry.
The final harness preflights native safety bounds before original execution and
reports rejected cases separately. These are native safety policies, not
original malformed-domain equivalence. The zero-light original fixture includes
one boundary byte for the carried column==width lookup; actual lighting access
and palette construction remain unvalidated.

## Validation and confidence

[Machine-readable comparison report](terrain-traversal-rotations.json) records
18,432 generated cases; 18,428 safe original/native comparisons match 3,089,824
queue records and complete cell flag mutations. View counts are 4,096 / 5,119 /
4,095 / 5,118. Four native-domain rejections occur before original execution.
There are 946,449 visits with nonzero wrap priority and 309,472 margin anchors.
Physical/coordinate alias evidence includes 202 view-one, 1,219 view-two and
22 view-three visits. Variations include rectangular grids, all camera-origin
corrections, cuts/modes/special layers, wrapping, viewport margins and large
negative origins. These counts extend, rather than add to, the earlier view-zero
reference fixture corpus.

The normal and sanitized traversal/queue/visibility CTests pass. Native fixtures
check all four starts/forward directions, differing layer bounds, retained
column==width carry, repeated physical indices and eight rejected domains.

Installed comparisons use CFsec01, CFsec02 and CFsec38: centered, wrapping,
panned and explicit boundary-carry cameras, all views, visibility on/off, normal
and ASan/UBSan builds. Each compares original traversal/ordinary producer/sort/
visibility queues, final physical owner flags, independently decoded MAP fields,
complete independently composed clipped RGB565 pixels and RGBA hashes. Shared
physical owners are checked on repeated visits. Surface count returns to zero.
There are 96 cases / 192 native images, 39,368 hidden draws and 22,762 partially
clipped draws across both builds. Repeated physical visits occur in 16 image runs
(96 repeated visits), with shared final owner flags verified.
Artifacts: `working/tests/world-terrain/run-y6ubmrse/`.

```bash
python3 tools/test-world-terrain-preview.py NORMAL --sanitized SANITIZED
```

The source snapshot `working/tests/terrain-rotations-source-22y9rb2t/`
is retained to isolate concurrent work: baseline
`4a108a2a29e9afaaa00de9784f49d0375ae8e5d6` plus this chunk's code overlay.
Build its `apps/terrain-preview` and pass `--source-root` to reproduce the run. Original manifests
pass before and after. LeakSanitizer is disabled for managed execution; prebuilt
Qt/driver libraries and the unchanged original PE are uninstrumented.

Confidence is high for safe selected four-orientation traversal, ordinary terrain
queues and the tested unshaded native scenes. Installed references disable object
references and creature processing, restoring that ignored flag for owner checks.
Native bytes remain exact and raw flags are not post-load normalized. Native
clipping/composition uses an independent CPU pixel oracle; this is not evidence
of original whole-world pixel rendering or clipping equivalence.

Remaining work: camera initialization and updates, map initialization/surface
admission, lighting/palette chains, water/overlays/entities, picking and live
routing. Full engine scene production remains broader than ordinary terrain.
