# Selected SPR visibility planes

Recovered from NoCD helper `0x005013c0`, executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
These planes are distinct from the main per-pixel transparent/opaque run data.

| Frame member | Selected helper interpretation |
| --- | --- |
| +12 / +16 | Signed frame X/Y origins |
| +32 | Offset to a four-byte header followed by coverage DWORD rows |
| +36 | Offset to test DWORD rows; absent/outside frame disables the helper |
| Coverage header byte +0 | Retained; not read by selected helper (32 in observed planes) |
| +1 | Row count, unsigned byte |
| +2 / +3 | Unsigned X/Y amounts added to frame origins |
| +4 onward | Row-count little-endian DWORDs, used when draw kind is 0 or 33 |
| Test plane +0 onward | Row-count little-endian DWORDs, used to test full coverage |

Test and coverage rows can differ. Each stored DWORD is shifted right by the
projected column remainder and byte-swapped before accessing the coverage grid.
No pixel colour, palette, or native opaqueMask participates in this test.
Do not derive these masks from decoded pixels without recovering their producer.

Installed version-4 inventory: 59,407 frames; 38,592 have both offsets and
20,815 have neither. All present planes have enough bytes for their declared
rows; maximum observed row count is 24. This inventory is static format evidence,
not execution coverage of all installed frames.

The native SPR loader now retains both planes as independently owned opaque
`auxiliaryData` arrays, with containing-frame extent checks and allocation
budget charges. General asset storage remains independent of reconstruction.
`decodeSpriteVisibility` in `reconstruction/rendering` interprets the selected
header/rows and rejects truncated data. Missing test data yields no shape;
extra padding is retained by storage and ignored by the selected interpretation.

Evidence and confidence: header accesses and transforms are directly recovered
from the pinned helper; synthetic original/native comparisons and installed
scene cases are documented in [visibility contract](../runtime/sprite-visibility.md).
Confidence is high for this selected reader. The generation of masks, legacy
SPR conversion and complete meanings of header byte zero remain unverified.
