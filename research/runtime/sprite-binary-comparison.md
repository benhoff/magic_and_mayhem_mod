# MMSprite versus the installed sprite loader

Reviewed 2026-10-04. This follow-up compares the pinned MMSprite reader with
selected original routines. It corrects the earlier assumption that all SPRs
without embedded palettes need an external palette: the installed version-4
files in this group carry direct 16-bit colour pixels.

## Builds and method

No-CD executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Clean executable SHA-256:
`124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214`.
MMSprite revision: `7d9bd05a2fb203d81bca11b808dc08b31fbd8c41`;
[source lock](../formats/mmsprite-source-lock.json).

```bash
python3 tools/compare-mmsprite-binary.py
```

Requires a host capable of executing ELF i386 programs and `g++ -m32`, plus
the clean and No-CD working installations and pinned external source. Managed
sandboxes may restrict 32-bit syscalls; the recorded run used approved execution
outside that restriction. The runner hash-checks both executable builds and
upstream tracked source, creates an unchanged No-CD snapshot in `working/`,
exports selected disassembly/direct caller evidence, and compiles the isolated
[reference harness](../../tests/sprite-binary-reference.cpp).

The harness maps PE sections privately at their preferred image base and calls
the **unmodified** original No-CD word drawing, indexed drawing, and word
conversion routines. It supplies a bounded destination, clipping globals and
coordinates adjusted for frame origins. Indexed samples use a fixture palette
chain containing one unshaded RGB565 table derived from the embedded palette;
the original palette allocator/shading builder is not executed. Mapped source
buffers and globals are private process memory; game files remain unchanged.
Each sample process has a five-second deadline.

The independent checked row model reads colours as bytes for indexed SPRs or
little-endian words for palette-free SPRs. It checks row width and frame
extents. Its output is compared byte-for-byte with the complete original
destination, including transparent runs and a one-pixel untouched border.
MMSprite RGBA samples are composited onto the same background and quantized
to RGB565 before comparison, so precision differences alone cannot account
for a reported mismatch.

Original-manifest verification runs before/after, including failures. The
runner rechecks executable, upstream, and asset hashes. Full evidence lives
under `working/tests/sprite-binary/`; the retained
[report](sprite-binary-comparison.json) contains input/output hashes, addresses,
compile command, artifact hashes and explicit validation boundaries.
Recorded evidence: `working/tests/sprite-binary/run-v23rzsaf/`; both original
verification logs report all 2,927 files verified. Static excerpts confirm
the clean build's version-4 check, -1 dispatch and word conversion as well;
only the No-CD drawing/conversion code was executed.

## Results

| Scope | Recorded result |
| --- | --- |
| Installed SPR metadata | 181 files; all frame-table addresses equal MMSprite's sequential frame addresses |
| Palette-free version-4 SPRs | All 27 files sampled; 77 frames drawn by original code; every MMSprite grayscale result differs |
| Indexed version-4 samples | 12 nonempty frames match with the supplied unshaded RGB565 palette, including all four palettes in the selected terrain file |
| Empty indexed records | Two original draws leave the complete destination unchanged; Python MMSprite raises `InvalidDimensionsException` |
| Checked row model | All 91 sampled original destinations match exactly |
| RGB565-to-RGB555 word conversion | All 77 palette-free samples match the conversion model after executing original conversion code |
| Live game / full loader / palette builder | Not executed by this harness |

For example, timer frame 91 is 40 by 40: 1,320 destination words differ between
MMSprite and original drawing. Buttons frame 0 differs at 1,131 words. RedCap
frames 0, 175 and 349 match after RGB565 quantization under the unshaded fixture
palette. These are isolated routine comparisons, not screenshots or full
gameplay rendering equivalence.

## Confirmed handling differences

### Palette-free means direct colour in this installed version-4 set

At No-CD `0x57e8e8`, frame member `+0x1c == 0xffffffff` selects the direct-word
draw dispatch. Backend setup selects `0x597086` for a non-MMX direct-word
implementation (`0x484561`). That routine advances coloured source pixels by
two bytes and copies WORDs (`0x5971e8`, `0x597208`, `0x59722f`), while transparent
runs advance only the destination. This is not an 8-bit external-palette path.
All 1,204 records in the installed palette-free version-4 files have this -1
member, according to the prior metadata inventory.

No-CD `0x57d560` optionally converts the stored word plane in place using
`((pixel >> 1) & 0x7fe0) | (pixel & 0x1f)`, the RGB565-to-RGB555 mapping.
It begins at the first row's pixel offset (`frame + 0x2c`) and ends at the first
nonzero offset in `frame + 0x20`, then `+0x24`, otherwise frame size. It skips
frames with zero width or height. The clean counterpart at `0x564360` has the
same conversion logic. This also confirms that those eight frame bytes skipped
by MMSprite have a role in the original implementation.

The pixel-format flag used by load/conversion is populated from the surface
mask query at No-CD `0x58d540`/caller `0x4e4253`. Original palette packing at
`0x582783` also branches between RGB555 and RGB565 masks. Static packing plus
isolated word conversion strongly supports RGB565 stored colour, with optional
RGB555 presentation; a live display-mode transition remains untested.

MMSprite creates a grayscale palette when the header palette count is zero and
consumes one byte per coloured pixel. Thus even its correctly shaped output
uses the wrong colour interpretation and source-byte progression. The C and
JavaScript implementations also use byte palette indices/grayscale here;
the reference project is not a faithful decoder for these direct-colour files.

Confidence: high for the installed sample mismatch, word-width behavior and
conversion expression; supported by static code and execution of the actual
original routines.

### Empty frames are drawing no-ops

No-CD direct drawing at `0x597097`/`0x5970a1` returns without drawing when either
dimension is zero. The selected indexed routine returns without writing for
the two zero-by-zero installed records exercised here. Its entry checks allow
execution when either dimension is nonzero; do not generalize the two-record
test to arbitrary one-zero malformed frames.

Python rejects zero dimensions. JavaScript coerces them to one when parsing.
A native loader should preserve the installed empty record and draw nothing,
instead of failing the whole sprite or inventing a one-by-one image.

### Version handling is stricter in the selected original loader

No-CD loader `0x57d310` reads a 24-byte header and compares its version field
with exactly 4 at `0x57d400`. On mismatch it displays "Incorrect Sprite Version"
and returns failure. Clean loader `0x564110` has the same comparison at
`0x564200`. MMSprite accepts and samples the seven installed version-2 SPRs,
including LOGO; its ability to decode them does not imply these loaders accept
them. Another original compatibility path or unused legacy assets remain
possible; complete call coverage has not been established.

The inspected loader also caps embedded palettes at four (`0x57d4ce`) and
requires a nonzero palette-build argument when palettes are present. MMSprite
does not implement that original initialization contract. Native asset parsing
and palette/shading initialization should remain separate operations.

### Palette indices become pointers, not fallback guesses

With palettes present, the original loader builds palette chains then replaces
each frame's `+0x1c` index with the corresponding object pointer
(`0x57d514`–`0x57d522`). The selected loop does not provide MMSprite's arbitrary
invalid-index-to-zero fallback. Palette-free frames retain -1, which draw
dispatch treats specially. All version-4 embedded-palette indices in this
installation are valid; the huge positive values found previously are in
version-2 files rejected by this loader.

At draw time, the indexed routine chooses a chain member and a 256-WORD colour
table according to draw arguments (`0x57e1db`–`0x57e20c`). MMSprite directly
expands raw RGB and ignores these runtime shade/variant tables. The fixture
comparison confirms the selected unshaded run decoding, not every lighting,
effect or palette variant.

### Origins, transparency and tables

Original drawing subtracts signed frame origins from requested positions
(`0x5970ae`, `0x5970f7`; indexed `0x57e217`/`0x57e21a`). MMSprite exposes origins
as metadata but its PNG exporter only writes the frame rectangle. Retaining
those coordinates in the native interface is required for placement.

Original row walkers stop according to frame width and advance transparent
runs without overwriting destination pixels. MMSprite derives delta lengths
from adjacent row offsets and exports transparency as alpha zero. These are
compatible for the indexed samples tested with an explicit mask, but differ
as APIs and are not interchangeable with alpha blending.

The binary uses the stored frame-offset table. Python walks frames sequentially
by their size. All installed SPR offsets agree, including version 2 under the
reader's four-byte adjustment. Offset-table handling therefore remains an
implementation difference, but is not an observed installed mismatch.

The selected original SPR loader checks complete file reads and exact version;
it does not reproduce MMSprite's signature/declared-size checks in its inspected
body. Keep safe native validation without claiming identical malformed-input
acceptance.

## Effect on the native asset work

MMSprite is a useful reference for ordinary indexed version-4 runs and metadata,
but cannot be the sole oracle for rendering assets. The native result needs
two colour representations: indexed bytes plus embedded palettes, or packed
RGB565 words; both retain a transparent-run mask, origins and empty records.
Rendering can use the existing indexed/16-bit surface support.

Start with embedded-palette RedCap as planned, but include the original draw
comparison as an oracle. Add direct-colour timer/Buttons as the next extension.
Treat grayscale as a diagnostic export policy only. Recover palette chains,
lighting/effects and font/ANI contracts independently before claiming complete
rendering equivalence. This work remains offline; no original live work is
replaced and no gameplay balance changes are made.
