# Qt menu SPR integration

## Asset evidence and native mappings

Read-only installed inspection on 2026-10-04 decoded complete sheets with the
native SPR inspector. Contact sheets and raw owned inspection JSON are retained
under `working/tests/menu-sprite-integration/`. No game process was launched.

| Sheet | Decoded content | Native widget association |
|---|---|---|
| Interface/CharacterScreen/800x600/sprites.spr | 12 RGB565 frames: coloured gems 0..2, grey gems 3..5, two adjustment triplets 6..8 and 9..11 | CFG SPRITEBAR pairs and STANDARDBUTTON triplets; original indices validated before mutation |
| Sprites/Buttons.spr | 21 RGB565 frames; first three triplets show face, book, portmanteau | Region Entry CFG maps Character to 0..2, Grimoire to 3..5, Spellbox to 6..8 |
| Interface/MultiplayerBattleSetup/MultiPBattle Screen buttons 800-600.spr | 63 RGB565 frames: eight colour-token triplets 0..23, boot triplet 24..26, twelve portrait triplets 27..62 | Single Player and host/guest lobby use explicit supplied colourIndex/portraitIndex; boot art reused for removal buttons |

Confidence is high for decoded pixels, dimensions, signed origins and CFG index
roles. Association of shared sheet filenames with Region/Single Player controls
and setup/lobby selection catalog is based on decoded visual content. It is a
native presentation policy, not recovered original file-binding/callback proof.
The preview colour samples select observed red/blue/green/gold triplets 1/2/0/6;
portrait samples cycle the first three triplets. Opaque IDs are never parsed as
asset names or indices. No original wizard identity or progression enum mapping
is asserted.

## Rendering and behavior

`loadMenuSprites` opens read-only through AssetStore and the native SPR loader,
with 8 MiB input/scanned limits, 128 frames, 128x128 frame dimensions, 1 MiPixel
and 4 MiB decoded limits. Signed origins are limited to +/-128. RGB565 expands
by bit replication; indexed frames use their decoded palette. The opaque mask
sets alpha, preserving opaque index/word zero as black. Owned QImages preserve
origins independently of input/file/decoder lifetime. Trailing auxiliary planes
are not treated as UI effects. Missing/empty triplets and invalid selections
fail before widget mutation; short/corrupt sheets retain the prior model/art.

SpriteButton retains QPushButton actions, text fallback, accessible names,
tooltips and keyboard activation. It draws frame 0 normally, frame 1 under the
pointer, frame 2 while pressed. Disabled controls use normal art at 55% opacity;
keyboard focus has a pale outline. These interaction policies are native and
have not been compared with original callbacks. Placement uses anchor minus
signed origin, scaled with each configured control rectangle. This preserves
normal/pressed frame offsets; it does not stretch every frame independently.

Character talisman rows use 350x50 at the CFG anchor, with active/grey frames
spaced 50 pixels apart for the installed maximum of seven. Each frame's signed
origin shifts its pixels within that row. Values remain available as QLabel
text/accessibility, although the row paints gems. Native repeated-icon counts
are bounded to 32. State changes update icons alongside existing purchase,
refund, accept/cancel behavior.

Supplied setup/lobby Players add portraitIndex (-1 or 0..11) and colourIndex
(-1 or 0..7). -1 retains text, and inactive slots retain native add/no-player
presentation. Selection bounds apply to inactive records too. Existing typed
requests carry selections; preview sample orchestration changes explicit indices
with sample labels/IDs. Host/guest/local-slot control permissions remain intact.

## Validation and remaining boundaries

Synthetic menu-sprite checks exercise RGB565/indexed conversion, transparency
versus opaque black, signed origins, ownership, missing/traversal inputs,
empty/out-of-range states, bounded origins/counts, normal/hover/pressed/disabled
art, scaling, semantic button actions and repeated active/grey gems. Widget
checks cover explicit selections, bounds, text fallback, short/corrupt sheet
rollback and existing models, navigation, purchase budgets and local permissions.

All 20 targeted menu/sprite-loader checks passed, with affected checks rerun
after final guard/test additions. Five installed-art captures (Character,
Region, Single Player, host lobby and guest lobby) were visually inspected.
All 20 installed preview modes passed smoke checks. All 2,927 original files verified unchanged before and after installed asset use.

Original SFT fonts, slider/radio artwork, other menu sprite controls, live
sprite-state equivalence, original catalogs and the engine command adapter
remain outstanding. This is native preview integration, not live replacement.
