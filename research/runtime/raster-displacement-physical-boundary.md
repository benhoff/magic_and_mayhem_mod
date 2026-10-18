# Displacement samples at the physical canvas boundary

A current camera-stress acquisition ends normally but fails producer acceptance:
seven `raster/splice14` calls forward. The source-only refusal retains a 61x47
indexed frame and prepared origin (730,570), on an 800x600 physical plane. Its
RLE rows are valid and contiguous. This failed run receives no acceptance credit.

Original `0x5968a4` selects a displacement from the original table at `0x656640`,
reads a DWORD at destination plus twice that displacement, and stores its low
WORD. Its ordinary horizontal clip tests the shape width. It does not reserve
the displacement distance at the bottom physical allocation boundary. The
table's original non-drawing initializer is `0x57dbd0`.

The captured frame's source row29 contains an opaque pixel at x60. At the
observed origin this writes physical (790,599), whose low-WORD source sample
has only nine valid positive displacement positions left. Larger offsets sample
past the owned allocation. The native kernel's first pass must refuse before
any write; physically readable neighboring bytes do not extend ownership.

`tools/test-raster-displacement-boundary.py` compares the exact captured shape,
original table initialization and original wrapper against the production
splice over all 16 phases, both formats and DF states. Padded owned observation
storage permits original reads to be observed safely. It remains outside the
native plane. Moved-up and moved-left controls distinguish physical-edge
refusal from a general frame/phase rejection. Native admission and original-once
fallback are counted separately. This is a controlled boundary comparison,
not accepted live camera coverage or an extension of native pixel ownership.

## Confirmed original comparison

The original initializer produces `[8,11,13,15,15,15,13,11,8,5,3,1,1,1,3,5]`
in the recorded x87 context. Seven phase positions exceed the nine-WORD sample
margin. The 192-case comparison passes: 164 native calls, 28 original-once
fallbacks, 28 direct atomic refusals, and 92,565,504 compared physical WORDs.
Both interior controls admit every phase. The complete existing raster suite
also passes 9,323 native cases. No production admission check was weakened.

This explains an ownership boundary consistent with the captured refusal. The
seven live forwards do not themselves record every runtime sine phase, so the
controlled comparison is separate from attribution of each individual call.
Original reads are observed in explicitly padded fixture storage; real-game
neighboring bytes and complete original invalid-memory behavior remain unknown.
The camera-stress attempt remains failed. Standard declared campaign acceptance
and the five missing natural callers are independent.

## Finite native CPU policy follow-up

The [finite CPU displacement repair](finite-cpu-displacement.md) replaces original-once forwarding for the unowned low-WORD sample with retaining that destination, matching the existing public native World policy. The strict recovered API and its atomic refusals remain available. New captured-shape controls pass192 native calls, zero forwards,28 deliberate boundary-policy cases/60 retained WORDs and92,565,504 checked physical WORDs. Every owned sample still matches original execution; no original unowned-memory equivalence is claimed. Earlier fallback/camera records above remain immutable and historical. New live observations distinguish a passing journey from positive execution of the finite branch.

## Current strict-control contract

`NP.raster-displacement-physical-boundary` now scopes the retained strict API:28 atomic refusals and two admitted interior controls across the captured phases/formats/DF states. The current production comparator separately checks192 finite native calls and60 retained WORDs against original-defined pixels plus the explicit policy. It no longer claims current original-once forwarding for those unowned taps. [Fresh strict-control evidence](finite-cpu-displacement-renewed-strict-20261018.json) binds this clarified contract before execution; previous scope and original-once records remain historical.
