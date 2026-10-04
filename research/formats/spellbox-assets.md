# Spellbox / Portmanteau installed assets

Read-only installed-input inspection, 2026-10-04. Original manifest verification
brackets this work; no executable or original file was modified.

| Installed relative path | Confirmed content |
|---|---|
| Interface/SpellBox/800x600/Portmanteau.bmp | Native BMP decoder returns an 800x600 background: three vertical coloured trays at the left, five horizontal item shelves at the right |
| Interface/SpellBox/800x600/Talisman.spr | Version 4, direct RGB565, 95 frames; frames 0/1/2 are visually empty blue/green/red talismans; remaining frames include illustrated talismans and small effect marks |
| Interface/SpellBox/800x600/mitems.spr | Version 4, direct RGB565, 23 distinct frame entries, with item artwork and signed origins |
| Interface/SpellBox/Spellboxtooltip.cfg | HEADER ValidConfig=TRUE; STR_04/05/06 name Law/Neutrality/Chaos talismans; STR_00..03 name Realm View, Grimoire, Character Improvement and Begin Battle |
| Interface/SpellBox/Spellboxtutorial.cfg | Four tutorial dialogs describe spell preparation, left talisman trays/right item shelves, dragging an item onto a talisman to create a spell and dragging it back to remove it; each item yields three spells depending on talisman type |

Equivalent 640x480 image/sprite files exist; the native widget uses the 800x600
set. Tooltip and tutorial files are text configuration, not control rectangles.
No dedicated SpellBox screen-layout CFG was found in this directory.

Decoded 800x600 item frames range from 33x34 to 82x83, with origin X -29..-1
and Y -27..0. Empty talismans are 80x86, 80x82 and 80x83 with origins
(-1,0), (-1,-4), (-1,-3). Illustrated frames generally fit that canvas; small
5..13 pixel marks retain larger negative origins around their talisman centre.
These are separate frames, not evidence of button state triplets. Frame counts
also agree with the prior [SPR inventory](spr-native-loading.json).

Confidence is high for decoded dimensions, frame contents and tutorial text.
The native blue=Law, green=Neutral, red=Chaos presentation and tray positions are
visual/native interpretations, not recovered engine indices. Item-to-spell
recipes, frame-ID bindings, inventory order, capacities, effect animation,
original fonts and navigation callbacks are not established by these files.
Caller-supplied IDs and explicit artwork indices must remain separate.
