# SFT font reader and selected text consumers

Reviewed 2026-10-04. Static evidence from the No-CD executable with SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence is high for selected reader arithmetic and local consumer behavior;
full font state lifecycle, code pages, rendered text equivalence and live native
replacement remain unverified.

## Reproducible evidence

```bash
python3 tools/export-sft-support.py --decompile
```

The exporter verifies the immutable manifest before/after, verifies executable
hash, saves selected disassembly, inventories all six installed fonts, rechecks
input hashes and optionally decompiles in a read-only Ghidra project. Ghidra
function creation, where needed, exists only in the discarded analysis session.
Recorded run: `working/decompiled/sft-support-pxjq5rm6/`; its report includes
artifact hashes. Retained summaries are in
[font evidence](../formats/sft-native-loading.json).

| Address | Selected responsibility |
| --- | --- |
| `0x004a5ae0` | Font file reader; thiscall object, path and conversion argument |
| `0x004a58e0` | Byte advance from contour state; optional profile update |
| `0x004a59e0` | Single-byte draw, cursor update and shared sprite draw call |
| `0x005575b0` | Font initialization caller; heading/body/tooltip/yellow paths |
| `0x00582440` | Referenced palette conversion; not validated here |
| `0x00581ec0` | Referenced shared sprite draw; separate SPR evidence exists |

The reader compares declared size at file offset four against bytes read, then
branches on versions two/three and palette presence. Version three with a
palette uses palette at +40, profiles at +808, offsets after
`profileCount × rowCount × 8`, and frame data after `glyphCount × 4` offsets.
Version two uses a different header; this native loader intentionally handles
only version three. No-palette installed evidence is absent, and decompilation
shows unusual use of the existing offset-table member in that branch; native
RGB565 support is a bounded synthetic policy, not a claim of original success.
The reader does not test the magic. Native signature and extent checks are
additional validation policies.

Relevant observed object members, for this build only:

| Offset | Role |
| ---: | --- |
| `+0x00` | Allocated file buffer |
| `+0x04` | Raw palette pointer, or zero |
| `+0x08` | Contour/profile pointer |
| `+0x0c` | Glyph-offset table pointer |
| `+0x10` | Glyph-frame base pointer |
| `+0x14` | Converted palette-list member |
| `+0x18` onward | Previous trailing profile values, one per profile row |
| `+0x218`, `+0x21c` | Draw cursor x/y |
| `+0x220` | Sum of header ascent/descent words |
| `+0x224` | Tab advance |
| `+0x230` | Tracking/extra spacing value |

These are recovered PE32 fields, not a native C++ ABI or cross-process channel.
When conversion is requested for a paletted font, the reader references the
palette converter and overwrites each in-memory glyph member at frame +28 with
a converted palette pointer. Native storage preserves on-disk indices and RGB;
no runtime pointer is copied into native data.

## Contours, byte mapping and draw behavior

The advance helper maps byte to profile ordinal `byte - 33`, then takes the
maximum (initially zero) of previous trailing minus current leading across rows.
When its update argument is nonzero it stores the current trailing profile.
Tracking and selected punctuation spacing are added. Quote-like byte values
receive an extra two; the other selected punctuation path depends on a global
and a string table whose complete semantics are not recovered here.

Space (32) and underscore (95) use the profile for `a` (97), while the draw
helper suppresses their glyph output. Tab (9) returns configured tab advance;
newline (10) and lower controls return zero. The single-byte draw helper calls
the advance helper with update enabled, advances cursor x, skips those special
bytes, and passes the selected frame to shared sprite drawing. Higher-level
line handling must be recovered separately; this helper alone does not establish
complete text layout or encoding.

The initializer chooses heading/body filenames containing 640 or 800 and loads
`sprites\toolTip text.sft` and `sprites\yellowtext.sft`. Reader callsites are
`0x005575f0`, `0x00557639`, `0x0055765c`, `0x00557680`. These corroborate the
font role and installed variants; native Qt widgets still require a separate
font rendering/layout integration.
