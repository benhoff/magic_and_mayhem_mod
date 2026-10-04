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

Original reference checked on 2026-10-04: the installed tutorial explicitly
describes right-hand ingredient shelves, left-hand talisman trays, drag-to-create
and drag-back-to-remove. The original English manual, printed page 10 (PDF page
7), also describes a selected ingredient at the top of the box with all three
possible spells, and spell hover tooltips. Its screenshot confirms the shelf/tray
layout. Reference: [original manual](https://oldgamesdownload.com/wp-content/uploads/manuals/magic-and-mayhem_win_manual_en_t0c.pdf).
This is primary documentation/static screenshot evidence, not live mouse-event
observation. A desktop capture attempt failed due to unavailable X authorization.

The native mock now shows the selected ingredient and all three caller-supplied
spell artworks/tooltips in a header. Clicking an available ingredient picks up
artwork that follows the pointer until placed or deselected; ordinary held-button
dragging remains supported with ingredient drag artwork. Hovering a talisman
temporarily shows the resulting supplied spell without assigning it. Dropping
commits that spell artwork. Picking up a filled talisman permits moving its
existing copy to another alignment or returning it to a shelf, including empty
shelf background. Shelf quantity is conserved; exhausted shelf artwork is hidden.
Right-click or the first Escape puts down a carried ingredient without mutating
the draft; Escape otherwise restores the accepted draft and cancels the screen.
Hidden screens clear pointer overlays. Click-to-carry and its cancellation are
user-requested native behavior; the manual only confirms held-button dragging.

An available ingredient's shelf artwork is hidden immediately on left-button
press and remains hidden during held dragging. Release or completion/cancellation
of the native drag restores shelf presentation using the resulting inventory;
a committed last-copy assignment still leaves the shelf empty. This temporary
visual state does not reserve or consume inventory, and hiding the cell clears
it. The synthetic Spellbox check compares shelf pixels while held and after an
outside release, and verifies that the quantity and assignments are unchanged.

Assign/Remove remain keyboard-accessible alternatives. Missing supplied spells
or exhausted copies prevent assignment. Drag ownership and model revision tokens
reject external/cross-widget/stale sources. A hover changes presentation only;
Preview remains a separate typed semantic request. Native quantity/control
policies remain distinct from original engine contracts. Mana-cost rings are
not shown because the supplied model contains no recovered spell costs/max mana.

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
Selected ingredient/three-spell art occupies the carved header with an authored
command row underneath.
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
