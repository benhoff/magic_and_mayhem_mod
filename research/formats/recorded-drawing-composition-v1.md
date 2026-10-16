# Recorded native GPU composition input, version 1

Diagnostic application input for `mnm-drawing-present <owned> <new-output>
--recorded-composition`. This is native policy, not an original engine wire.
The backend receives native-derived RGB565 source snapshots and an input-only
transfer plan; original comparison pixels, oracle paths and expected results
stay in the external comparator.

`presentation.json` has exactly `schema: 1`, `width`, `height`, `sources`, `frames`.
Each source has integer `id`, contained `path`, SHA-256 `sha256`, `width`, `height`.
Source bytes are tight little-endian RGB565 with exact dimensions. Every source
must be used. Hashes are checked before execution, at upload and after execution.
The declaration is also checked after execution. Unknown fields and invalid
numeric types refuse, including fractions and booleans.

Frame indices are contiguous from 1. A `fill` frame has exactly `index`, `kind`,
`colour` (0..65535). A `copy` frame has exactly `index`, `kind`, `source_kind`,
`source`, `view_rect`, `source_rect`, `destination_rect`, `key`. `source_kind` is
`snapshot` (declared source ID) or `history` (strictly earlier presentation index).
Rectangles are half-open [left, top, right, bottom], positive and within their
owned extents. The source rectangle must lie inside its declared view; source
and destination extents must match. Key is null or a native RGB565 value.

The native GlBlitter performs opaque/keyed copies, preserves destination history
and freezes earlier presentation histories with GPU copies when referenced.
Source/history textures retire at their last declared use. The primary starts
undefined; a partial/keyed first write cannot invent valid background pixels.
Live storage is bounded to 64 surfaces/16,777,216 pixels, with at most 8192 sources
and frames, 2048x2048 extents, a 4 MiB declaration and 2 GiB source bytes.

Each completed frame is presented at 1:1 on an isolated X11 screen. The backend
retains a diagnostic full RGB565 GPU readback and a full RGBA screen witness after
Qt's buffer swap, with hashes and exact input frame index. Output creation is
exclusive; crop/scale, missing swaps, invalid GL state and leaked native surfaces
refuse. Expected original pixels never choose a source or ordering. Diagnostic
readbacks and source-image uploads are intentional comparison overhead, not a
normal-frame or 60 FPS claim. Xvfb witnesses do not establish hardware scanout or
that an original collector saw every global presentation route.
