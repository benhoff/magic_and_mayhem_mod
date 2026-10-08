# Native rendering commit boundaries

This note records the intermediate source split of the 2026-10-07 renderer work.
It is source accounting, not additional original execution evidence.

1. Portable C word rasterizer and host test: complete borrowed-input preflight,
   signed origins, opaque zero, guard/padding preservation and bounded refusal.
   Runtime hooks, original comparisons and live replacement are committed later.
2. Native World frame decoding, integer RGB565 composition and retained canvas
   history: independently buildable previews and synthetic renderer tests.
   Runtime producers, channel delivery, live Qt presentation and preserved
   original comparison evidence are committed in the integration chunk.
3. Runtime adapters and live viewer: opt-in word routes, owned World capture and
   bounded publication, resource catalogue, Qt session, launcher integration,
   preserved original/live evidence and the complete coverage register.

Intermediate partial registrations deliberately do not claim original comparison,
integration or replacement. Later registrations retain each evidence source hash;
older renderer/UI comparisons remain historical when shared source changes.
Original startup canvas ownership, missed queue history, complete dispatch, HUD,
input and whole-scene bypass remain separate pending boundaries. No gameplay
balance changes or original media edits are part of this split.
