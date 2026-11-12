# Two minimap update acceptance rows

MU07 closes `journey.hud-minimap-tooltips.minimap-update` for the explicit
default and software profiles at actual 640x480 and 800x600. The unchanged
156-row inventory advances from **60 passed / 96 pending to 62 passed /
94 pending**. Every parent row and both pending final decisions are preserved.

The immutable result is
`rendering-next-two-results-20261107/closeout-analysis-04.json`.
This is fresh analysis of four retained real Campaign/storage executions.
It adds no original-game run, original-reference execution, native code,
capture, build or game fixture. Confidence is high within the finite scope
below; native execution retains its historical source versions.

## Correct source and presentation phase

The captured original sequence writes minimap terrain, camera corners, one or
more cell markers and creature markers to logical canvas 2. It then copies
that canvas to display canvas 1 **before** entering the World scene queue.
Scene drawing subsequently overwrites the minimap region on canvas 2. The
retained original World result planes describe that later scene result and
cannot be used as minimap expected output.

The analyzer independently computes the final directly determined RGB565
WORDs from the captured terrain triplets and owned overlay arguments, in
original call order. It checks view zero, map dimensions, center and origin,
format/stride, every cell call and its original return offset, the camera
return state and refreshed creature dimensions. Visible terrain and final
overlay writes supply expected values. Fog-hidden terrain requires an auxiliary
background; those unknown WORDs supply no independent output credit here.
Nonhidden terrain is required, so isolated marker stores cannot satisfy the
update check.

Each selected source packet/request must equal the accepted immutable packet
for its generation. The native display copy must be unique between the prior
and current World imports, use the live World handle incarnation, cover the
exact full source, and belong to a closed successful original application
caller interval. The independent history also verifies the complete copy
destination against the retained diagnostic.

Separate pixel lineage tracks the directly determined map WORDs through
copies, overwrites, swaps and presentations. It preserves display/UI contents
when a later World import replaces only the scene source. Every expected
map WORD must survive into the chosen viewport; sparse samples cannot pass.
Every corresponding scaled RGBA pixel is compared with the independently
computed WORD. The entire viewport body must also match the accepted prior
comparison hash and lease/command identity.

Three advancing source generations per domain must have meaningful changed
inputs and changed visible output. Redraw counts, unused flash fields and
unchanged pixels cannot qualify. This closes the finite map-update requirement;
whole markers/borders, camera interaction, picking, upstream simulation
visibility and hidden-background restoration remain separate.

The four domains each select source generations 3, 7 and 12. Together they
verify 10,716 displayed map pixels and 32 changed visible WORDs.

## Guards, negatives and applicability

Twenty-five actual-data guards cover source order and cardinality, mixed map
state, original finite results, wrong pixels, missing expected WORDs, unchanged
output, copy timing/results, handle reuse, unrelated scene generations, lost
ownership and missing domains. Existing UI/client controls check the selected
62-row progress pack. The register, source census, audit, exact review receipts,
normal staged hook and verified local evidence backups accompany the commit.

The first attempt assumed one cell-marker call, as in the earlier Quick
capture; Campaign contains multiple calls. The second attempted to use the
post-scene original plane. Its diagnostic found every directly known minimap
WORD overwritten by scene drawing, granting zero pixel credit. Both failed
reports and exact source preimages remain in SQLite. A corrected development
pass checked all four domains but its final archive write was refused by the
filesystem sandbox; its log and exact sources remain retained. The final
analysis records the corrected phase with the required archive access.

Only the new Python retained-data analysis receives current execution support.
The inherited eight native source differences remain visible; no historical
hash is refreshed and no native dependency is waived. All known cursor and
other failures remain. Current whole-player validation, arbitrary maps/views/
providers, the other 94 rows and both final decisions remain open. No remote
push, publication or private transfer is part of this batch.
