# Scene visibility and effects — 2026-11-07

This batch investigates the six remaining scene rows: visibility/occlusion,
attachment placement and combat output for default and software players.
Animation, lighting, auxiliary drawing and blending retain their existing
proofs. This work does not change engine code or gameplay balance.

The visibility analyzer checks changing opaque scene occlusion using retained
ordinary Campaign/storage executions at actual 640×480 and 800×600. It keeps
the complete named creature frame, effective palette bytes, draw anchor, clip,
source canvas incarnation and map/camera fixed across adjacent generations.
Only points switching between that opaque body and a later opaque raster with
a different source WORD qualify. Animation, motion, equal-colour overlaps,
blends, shadows, displacement and rectangle overwrites cannot supply this
proof. Original fog/coverage-grid activation and terrain-owner mutation remain
separate recovered contracts; this is finite downstream occlusion delivery.

Every qualifying point is checked against the independently reconstructed
original World. The exact request descriptors bind source order, original
entry, consumed AX and source checksum. The ordinary native command history
then maps the points through owned imports, copies and presentation to the
actual swapped viewport. Both adjacent generations must have live imports,
complete independently matched viewports and every selected point visible in
that viewport. Native pixels never choose the source witnesses. Addresses are
session-local observations, not identities assumed stable across launches.

Results are retained as immutable SQLite references in
`rendering-scene-effects-results-20261107/`. The analyzer freezes its source
closure and refuses partial admission if any domain fails. Failed field-name
and first-entry camera preparations remain retained. First entry has its own
accepted loading/preparation contract and earns no new credit here.

## Attachment placement remains separate

The existing [placement contract](animation-placement-attachments.md) and
[configured recipe](native-attachment-recipe.md) prove bounded helper math and
offline composition. The first child uses creature base plus the parent's ANI
attachment point plus the child's displacement; it does not include the
parent body's displacement. Footprint-two view adjustments and the separate
second-point/special half-height caller must keep their existing distinctions.

The retained ordinary rendering stream owns frame bytes, palettes, raster
anchors and order, but does not carry the parent/child relationship, displayed
parent/child ANI records or the creature base that produced those anchors.
Neither matching asset names nor nearby sprites establish that relationship.
A valid next observation must bind those raw inputs to a source generation at
the actual original producer, then check the resulting queue and native output
across the relevant views. Original helper sites `0x507250`/`0x5072c0` and caller
ranges `0x4fa9d0`, `0x4faaf0..0x4fab3c`, `0x4fb1e8` are existing evidence leads,
not newly observed calls. No synthetic preview closes either player row.

## Combat output remains separate

[Physical campaign input evidence](native-campaign-combat-input-20261020.md)
contains real summon, mana debit, melee and lethal transitions. Its exact
historical source phase and parent throughput failure remain intact. Later
campaign diagnostics also retain real damage and ordinary input, with their
explicit acquisition/pixel/nominal boundaries. They are not silently admitted
to the ordinary player cohort.

`runtime/menu/campaign_gameplay_observe.h` records actor snapshots and melee
returns with event ordinal, engine thread and tick. Its periodic snapshots do
not include an accepted source-generation identity or attachment pose. The
ordinary four-domain storage captures used here do not contain this gameplay
journal. A downstream effect raster cannot identify a spell, attacker, target,
damage or death on its own. The next combat observation needs those original
action/effect/state events and an exact source-generation join before complete
native-output and normal identity/lifetime admission. The native synthetic
combat sandbox supplies no credit for either player combat row.

Both final rendering decisions and all prior failures remain pending or
historical as recorded. No new game, original oracle execution, native build,
fixture or replacement is supplied by this retained-data analysis.

## Result and reproduction

`SV07.occlusion03` closes the two visibility rows: **2 of the 6 requested rows**.
All 88 accepted parent rows remain byte-for-byte unchanged; the selected inventory
is 90 passed / 66 pending, with both final decisions pending. The four domains supply
86, 209, 31 and 86 changing scene points respectively (default 800, software 800,
default 640, software 640). All 412 points map through both adjacent actual viewports:
824 native pixels, zero WORD/RGBA differences.

28 actual-data/adverse tests, 30 coverage UI tests and 9 client checks pass without
skips. Controls reject changing body frames/palettes/anchors/camera/imports,
blends/equal colours/rectangles, missing generations/points/leases, native pixel
differences, unreviewed analyzer drift and relabelled execution identities.
The current test-file correction/default selector receives fresh controls; the
source/WORD/viewport analyzer and all historical engine hashes are unchanged.

The first analyzer used the presentation's wrong generation-field name; the
second included first-entry camera preparation. Both refusals and exact source
bytes remain retained. A guard first treated an unknown writer's KeyError as an
unexpected exception, then consistently admitted both refusal exception types.
An initial client guard named a nonexistent Python file; the final command
executes the actual JavaScript checks. These are analyzer/control failures,
with no original startup or pixel-equivalence credit.

```sh
python3 tools/check-scene-effects.py \
  --report research/runtime/rendering-scene-effects-results-20261107/NEW-ANALYSIS.json
python3 tools/join-scene-effects.py \
  --analysis research/runtime/rendering-scene-effects-results-20261107/NEW-ANALYSIS.json \
  --report research/runtime/rendering-scene-effects-results-20261107/NEW-ADMISSION.json
MNM_SCENE_EFFECTS_REPORT=research/runtime/rendering-scene-effects-results-20261107/analysis-03.json \
  python3 -B tests/test-scene-effects.py
```

Run producers through the working-storage guard. Generated reports and exact
preimages use the SQLite writer; no expanded research JSON is introduced.
The analysis retains original engine and renderer source phases rather than
refreshing their hashes. Existing native identity/normal-retirement admissions
remain attached through the preserved primary-case/parent records.

The full coverage audit reports zero errors and no warnings for the new
behavior/evidence IDs. The incoming `ed5b85b1d..cdd60f0e3` range has two commits
and 897 parent/file transitions. All 382 unique SQLite payloads used by that
review are verified; 815 missing generated-artifact transitions reduce to
409 exact pairs and receive retrospective pending receipts. The corrected
range review has no unresolved gaps. Those receipts establish artifact custody
and accounting, not a historical gate pass or new diagnostic validation. The
original 2,927-file manifest also verifies before and after this batch.
