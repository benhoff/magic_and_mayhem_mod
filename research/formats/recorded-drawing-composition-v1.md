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

## Version 2: display extents

Schema 2 retains the same root fields and source contract. Every frame additionally
has exact integer `width` and `height` (1..2048). Copies validate their destination
against that frame; history sources validate against the saved frame's own
extent. A size change destroys the old primary and creates undefined storage.
Every frame additionally declares `reset_colour`, either null or an integer
RGB565 colour. A non-null reset is permitted only at an actual extent change
after the first frame; it defines the newly created primary before that frame
is drawn. Without a reset, a full fill or covering opaque copy must establish
the new complete image before readback/presentation. A reset without resize,
on the initial frame, or outside RGB565 refuses. The storage comparison
explicitly selects black at its action-bound display rebuilds; this is native
policy, not an original allocator guarantee. Saved histories remain owned until their declared last use.
Each output records its actual dimensions. Schema 1 retains fixed root dimensions.
The plan builder selects version 2 only when every binding declares output extents.


## Portable reference catalogs, schema 3

This is the external comparator catalog, separate from the native composition
plan versions above. Schema3 retains the schema2 action-bound storage boundary
and introduces `retention_evidence`, `reference_profile` and `witnesses`. The
retention ID selects a registered immutable static record; the whole catalog
must equal that record. Existing schema1/2 catalogs retain their original IDs.
A new retention cannot overwrite an existing evidence ID or fixture directory.

`reference_profile` contains a name (`unmodified_upload` or `stale_upload_crop`)
and six Boolean acquisition flags for crop, viewport, same-current rebind,
release/restore, destination witness and XCB witness. Recompute this value from
the independently registered original report. Rebind/viewport variants are not
admitted here. Crop requires both destination and XCB witnesses. A corrected
original cannot be admitted through a legacy schema that omits this provenance.

Each `witnesses` entry has its contiguous ordinal and a `files` map from original
basename to the existing gzip artifact contract. Retain original source RGB565,
default framebuffer metadata/pixels and each XCB packet. Names, hashes, extents,
frame/call identities, destination state, packet geometry and policy enabled
flags are checked against the immutable original acquisition. The external
comparator restores these files outside native inputs, repeats source/native,
source/destination, native/destination and destination/physical comparisons,
and fails on any difference. Final GPU/physical comparisons retain RGB tolerance1
and exclude alpha; RGB565 source/default witnesses compare exact words. No
frame omission, per-profile tolerance or generic journey acceptance exception.
