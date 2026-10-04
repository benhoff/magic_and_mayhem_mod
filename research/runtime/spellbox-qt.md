# Native Spellbox / Portmanteau preview

SpellboxWidget presents a caller-supplied inventory and prepares a local spell
loadout. It loads the original BMP background and both SPR sheets through the
bounded native menu loaders. Region Entry's Portmanteau icon opens it;
standalone --spellbox opens the same widget with explicitly labelled sample data.
No game process, campaign inventory reader or engine command is involved.

## Model and local behavior

Inventory has an opaque owner ID, item IDs/names/quantity, explicit item artwork
indices, three optional supplied Spell records per item, and talisman IDs with
an Alignment and optional assigned item ID. A spell's ID/name/artwork are supplied;
the widget does not derive spell recipes from item or sprite numbers.

Native limits are 30 item records, 21 talismans and seven per alignment, quantity
1..999, 128-character single-line IDs/names, item frame -1 or 0..22 and spell frame
-1 or 0..94. -1 preserves a text fallback. Duplicate item/talisman IDs, invalid
alignments, dangling assignments, missing assigned spells, and assignments
exceeding quantity are rejected before model mutation. These are preview policies;
original inventory capacities and stack rules remain unverified.

Select an item and talisman, then Assign, or drag an item to a talisman. Assignment
replaces that slot's previous item and releases its copy; a missing supplied spell
or exhausted item prevents it. Remove or dragging a filled talisman to an item
shelf returns its copy locally. Shelf counts show unassigned copies. A drag back
to a shelf uses the source talisman, regardless of the destination item cell;
empty shelf areas are not drop targets. Cross-widget/external/stale drag inputs
are rejected by source ownership and a model revision token. Single-frame cells
retain QPushButton focus, keyboard and accessible labels rather than treating
adjacent SPR frames as hover states.

Preview emits typed owner/talisman/item/spell IDs for the selected item and
alignment without changing assignments. OK emits the complete typed loadout,
including empty slots, and accepts it locally; Cancel/Escape restores the last
accepted snapshot. Enter activates the focused cell/action once, with key repeat
suppressed. Supplied text remains literal. No engine inventory is consumed.

## Artwork and navigation boundaries

Original decoded colour and alpha masks are preserved, including opaque black
and signed SPR origins. The 800x600 canvas scales with 4:3 letterboxing. Native
cell placement follows visible tray/shelf areas: talisman columns X=198/104/10,
Y=7+84*row; items X=290+83*column, Y=93+102*row, six per shelf. Cells use 84x86.
An authored command row and selection details occupy the carved header.
Selection outlines, quantity labels, system fonts, Assign/Remove/Preview/OK/Cancel
controls and stack handling are native presentation. No original runtime
screenshot equivalence has been verified. Original navigation tooltips are
preserved in the input file; their icon/callback bindings and effect frames are
not reconstructed.

MenuPreview supplies six illustrative item records and nine empty talismans.
Its sample spell names/IDs/recipes and illustrated frame choices are authored
preview data; they are not recovered gameplay mappings. Region close/accept
restores the caller's region/difficulty and focuses its Spellbox icon. Accepted
local assignments survive reopening; Cancel discards subsequent edits. A failed
asset load retains the caller, model and previous art. Standalone close returns
to Main Menu. Preview and acceptance are reported as pending adapter intent.

## Validation

The synthetic qt-spellbox check covers supplied IDs and literal labels, preview
signals, assignment/replacement/removal, quantity exhaustion, unsupported
alignment spells, accepted/cancelled snapshots, invalid-model/asset rollback,
maximum grids, sprite origins/colour, keyboard repeat/focus, scaled geometry and
Region/standalone navigation. All 21 targeted Qt menu/help/startup checks pass.

An installed-input inspector in working/tests/spellbox-preview exercised drag
assignment/removal under a virtual X display, rendered all 23 item and 95 talisman
frames through explicit inspection models, and checked Region return. Captures
empty.png, assigned.png and full.png were visually inspected. Installed --spellbox
and --region-entry smoke checks and conflicting menu flag rejection are checked.
Original manifest verification passes for 2,927 files before and after inspection.
These are offline/native preview checks; original runtime behavior, spell recipe
bindings, campaign persistence and engine-thread dispatch remain pending.

```bash
./tools/run-qt-shell.sh --spellbox
./tools/run-qt-shell.sh --region-entry
ctest --test-dir working/build/qt-shell -R '^qt-spellbox$' --output-on-failure
```

[Installed asset evidence](../formats/spellbox-assets.md).
