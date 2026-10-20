# Indexed World run outline

The pinned No-CD57fe10 leaf used by primary kind7 has two distinct pixel
branches. With no horizontal clipping it copies only the first and last pixel
of every encoded opaque RLE run. A zero transparent skip preserves separate
adjacent run endpoints. Vertical clipping does not change that branch.
Horizontal clipping instead half-blends each surviving opaque pixel against
the destination. WOUT.original/native-20261020 confirms these branches in the declared
219-fixture matrix:13,898,234 exact RGB565 words, zero differences. The136
outline fixtures also pass through the separate renderer process. Additive and
kind8 regression pixels, worker/atlas paths, source identity and covered
displacement-boundary refusals repeat. Original manifest bookends pass.

Native preparation preserves endpoint bit2 alongside ordinary coverage bit1;
normal shaders still use nonzero coverage. The outline shader copies only
endpoint texels. The same two planes and atlas slots serve ordinary sprites.
Visual rebinding includes encoded run boundaries, which affect this pixel
operation and existing whole-run clipping. Owned World v1 tag9/backend14
requires indexed frame/palette inputs and selects its branch from signed
geometry and the exclusive clip. Unknown tags and backend mismatches refuse.

Source construction/lifetime, original return/register state, RGB555/other
masks, tint581200, live capture/admission and full replacement remain pending.
