# Qt menu BMP/JPEG integration

## Scope and evidence

All existing menu widgets share `menu_assets.cpp`. Their original JPEG
backgrounds, including Region Entry illustrations, now pass through
`mnm-jpeg-loader`; the Mini Menu BMP panel passes through `mnm-bmp-loader`.
The application no longer calls QImage's codec entry point directly for menus.
The JPEG service still uses its private Qt codec backend.

`loadMenuImage` opens inputs through the bounded read-only AssetStore. It chooses
BMP/JPEG explicitly by extension, sets an 8 MiB input cap and limits decoded
width/height/pixels/bytes to the expected image size (maximum 1600x1200). It
requires exact dimensions and copies tightly packed RGB into an owned QImage,
including unaligned RGB rows. No raw decoder-result storage survives by alias.
Missing, corrupt, oversized, mismatched or unsupported inputs fail the widget's
existing transactional load, preserving previous models/artwork.

Character Screen additionally loads the three STATBAR Text BMPs from its CFG
and WizardFace0/1/2.JPG. Installed 800x600 bar images are 700x20. Direct decoded
inspection shows matching coloured and grey halves, each 350x20; the native
widget clips the coloured texture proportionally over the grey texture. It
retains QProgressBar value/accessibility semantics. Portraits are 400x300 JPEGs
with blue backgrounds; explicit supplied indices select faces, with -1 retaining
text fallback. Face progression is not inferred from stats or character IDs.
See [Character Screen policies](character-screen-qt.md).

Confidence is high for configured filenames, installed dimensions and decoded
asset appearance; bar clipping, JPEG blue-key threshold and portrait placement
are native presentation choices. Original transparency, placement and live
callback equivalence have not been validated. SPR-backed buttons, talismans,
setup/lobby portraits/colours and SFT typography remain separate work. BMP/JPEG
loaders alone do not decode those controls. Grimoire/Spellbox/Realm Viewer image
assets belong to screens not yet implemented in the menu preview.

## Validation

Synthetic `qt-menu-images` checks cover RGB order, BMP row padding/orientation,
owned QImage lifetime, both loaders, expected-size limits, rejected traversal,
missing/corrupt/unsupported images and mismatched codecs. Character Screen checks
cover coloured/grey bar pixels, explicit face selection, blue backdrop removal,
invalid selection and corrupt texture/portrait load rollback, alongside existing
budget/navigation checks. Existing menu tests cover all other background routes.

All 22 targeted Qt/menu and BMP/JPEG loader checks passed. All 20 installed-asset
preview modes passed smoke checks. The updated Character Screen capture was
visually inspected at `working/tests/menu-image-integration/character.png`;
original faces and bar textures appear in the configured frame and stat rows.
Synthetic tests also verify selected face identity and scaled/letterboxed image
geometry. All 2,927 original files verified unchanged before and after inspection.
The independent loader corpus evidence remains in
[BMP loading](../formats/bmp-native-loading.md) and
[JPEG loading](../formats/jpeg-native-loading.md).
