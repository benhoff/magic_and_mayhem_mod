# Atomic admission for remaining native pixel writers

This change keeps destination-dependent startup/HUD operations from partially
changing native history when a later source dependency is unavailable. RGB
addition checks all selected destination pixels first. Blend and displacement
rasters check selected destination/sample pixels and offsets before mutation.
Projected shadows check the full selected footprint before darkening it. Legacy
minimap terrain resolves every hidden-cell history coordinate before defining
any destination pixels. Source copying and image updates already preflight
their admitted inputs; panel and fade admission have separate evidence.

These are intentional native failure policies, separate from recovered original
success pixels. Opaque sprite writes still initialize undefined storage. Clipped
and transparent samples do not introduce dependencies on unused rows. Forward
wave offsets retain the existing left-to-right pre-draw sampling rule.

`other-writer-atomic-test` tests late undefined blend/add/wave/shadow dependencies,
late invalid wave offsets/positions, projected-shadow overflow, missing hidden
minimap history, and a projected history coordinate outside an otherwise large
enough old image. After refusal, only the originally missing pixel is supplied;
the complete native canvas must equal its pre-call state. Native Debug regression
tests and all saved original startup/HUD checkpoints provide separate success
pixel evidence. This does not test arbitrary driver failures, unsampled writers,
physical cursor latency or complete live rendering takeover.

The [current Debug CPU/GPU regression](other-producer-checkpoints-20261010.json)
passes 19 atomic writer refusals and two unused-dependency/opaque-initialization
positives, the existing native history/minimap fixtures, and all 954 saved
original startup/HUD completions / 422,875,312 RGB565 pixels. Every native GPU
mirror agrees; all GPU surfaces retire. The saved original journal stays pinned
separately from native source/compiler/build fingerprints. No original completion
files enter the native input directory or consumer.

Shared source edits leave older rendering/font/World evidence historical at its
recorded hashes. This new scoped replay does not promote those wider contracts
or the independently maintained live glyph hook. The refreshed image corpus
also passes 288 source-only placements / 5,387,712 WORDs and 288 atomic malformed
input refusals; odd/padded canvas fade refresh passes 972 original cases and ten
native refusals. See [image](canvas-image-producer-regression-20261010.json) and
[fade](canvas-fade-span-regression-20261010.json) reports.
