# Requested native frame paint before physical capture

Two fresh NVIDIA recorded-session suites exposed physical captures of an older
native frame while their same-ordinal GPU readbacks matched the reference.
The concurrent run had7,953,826mismatching pixels; a repeat after the live
storage run stopped had992,633. Each includes the existing126GPU baseline
mismatch. All three recorded cohorts had physical differences, including the
corrected reference cohort. These failures remain retained, without changing
ordinals, references or RGB tolerance1.

A diagnostic comparison of11selected bad physical images to nearby owned
native GPU frames finds a complete match to the preceding frame at tolerance1
(one image matches two identical preceding images). These candidates diagnose
stale presentation; they are not used to reassign or admit frames. Original
comparison pixels never enter native rendering.

The presenter previously accepted the first `frameSwapped` signal after
supplying a GPU frame. It did not require that requested frame's successful
paint before accepting a swap. The new admission observes the existing
`GlViewport::framePaintTiming` callback for that supplied frame, ignores swaps
before successful paint and waits for a subsequent swap. The callback is cleared
before its local state expires. Geometry, full-frame capture and the existing
five-second timeout remain unchanged.

The executed presenter emits `painted_before_swap_frames` and
`ignored_prepaint_swaps`. The recorded comparator requires the complete paint
receipt; an older presenter without it cannot pass provenance admission.
This is a presentation-comparator policy, separate from original resource read
reservations and the native live application's menu/input lifecycle.

The post-change paint-only run still fails as recorded below. A widget paint
and swap do not establish all-context completion, scanout or an unobstructed
screen in arbitrary environments. Complete native replacement and47/21journey/
operation closure remain open.

The paint-only trial still has1,292mismatching pixels (including the unchanged
252baseline negative); this establishes that paint plus swap alone is
insufficient for physical identity. That source version and failure are retained.

The final policy adds an ordinal strip outside the compared game rectangle.
It is updated only after the requested native GPU frame paints. The host holds
that GPU frame and reads game plus strip in one X11 image, admitting only the
current marker. The marker is generated from native ordinal metadata; the
original reference is never inspected by this admission loop. Late captures
are retried by marker identity, with a five-second deadline. The same fixed
ordinal and all game pixels are then compared normally.

Raw strip bytes and a game-plus-strip SHA-256 retain the same-image receipt.
The parser rejects stale/relabelled markers, changed game bytes, corrupted
markers with refreshed hashes, missing identities and wrong extents. Five
focused controls cover these failures. The marker requires32diagnostic rows
below the viewport; complete screen containment is checked before execution.
See [physical generation format](../formats/drawing-physical-generation-v1.md).
Final full software and NVIDIA suites each compare1,270,115,312game pixels
with only the unchanged252pixel unmodified-reference negative (126GPU and
126physical at storage ordinal170). The other six profiles match, including
Quick Battle and corrected storage. All323+238+222recorded frames in each
suite have valid same-image ordinal receipts; no frame reassignment or
tolerance change is used.35focused Python methods, including five generation
receipt controls, pass. Source hashes remain stable.

A separate strict no-forward native campaign reaches Save then times out
waiting for native readiness at stage4. It records6,612native frames,6,581paints
and40,089optional native pixel calls with zero forwards, fallback or recovery.
This is retained as an unresolved storage/input lifecycle failure. The
comparison-host capture fix does not assert a fix for that native application
route. Original manifests verify and the private Wine prefix stops cleanly.
