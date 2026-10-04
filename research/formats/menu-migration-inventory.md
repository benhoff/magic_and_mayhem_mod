# Menu migration inventory

Read-only inspection of installed plaintext layouts under
`working/game-nocd/Interface` and labels in
`working/game-nocd/CFG/interface screens text.cfg`, 2026-10-04.
Counts below are declared sections, not observed active controls. No game was
launched. Confidence is high for configuration contents, medium for migration
scope inferred from them. No action dispatcher or live state contract was
recovered during this inspection.

## Text/button candidates

| Screen / relative layout path | Declared controls | Functional boundary |
|---|---|---|
| QuickBattleMainMenu/Screen (Quick Battle Main Menu).cfg | 1 text, 4 text buttons: Create Multiplayer Game, Join Multiplayer Game, Create Single Player Game, Cancel | Best next standalone menu; child screens and network/game setup stay in the engine initially |
| MiniMenu/screen (Mini Menu).cfg | 8 text buttons across two documented modes | Campaign: Load Game, Save Game, Preferences, Quit Game, Cancel. Battle: Preferences, Quit Battle, Cancel. Mode selection, pause/resume, conditional availability and quit behavior need live recovery |
| BattleEnd/screen (Battle End - Victory).cfg and screen (Battle End - Defeat).cfg | Each has 21 text sections and 1 OK text button | Suitable text-only result widgets once the engine supplies rewards, points and other dynamic values |
| QuickBattleEnd/screen (Quick Battle End).cfg | 26 text sections, 4 picture boxes, 3 text buttons: Spectate, Continue, Quit | A functional approximation can replace pictures with text; still requires result/player data and action availability |

Quick Battle's button IDs are 76, 41, 77 and 11, with 800x600 rectangles
`200,200,600,250`, `200,275,600,325`, `200,350,600,400`, and
`280,475,520,525`. Its heading uses text ID 2. These are enough for a native
preview using the same CFG/string-table approach as MainMenuWidget.

MiniMenu's file comments explicitly separate realm-view and battle-mode
buttons. Several rectangles overlap across modes; displaying all eight at
once would be incorrect. Some battle entries have no Rect1. A mode-aware
widget using Rect2 and scaled 800x600 presentation is feasible, but the
configuration does not establish which buttons the engine enables at runtime.

Further inspection for the native Mini Menu confirms BMP panel assets:
`800x600/Realm View Mini Menu 800-600.BMP` decodes to 600x400, and the
640x480 counterpart decodes to 480x320. Confidence is high for decoded sizes;
centering the panel is a native preview policy pending live placement evidence.
See [implemented variants and validation](../runtime/mini-menu-qt.md).

## Standard Qt controls with additional data requirements

| Screen | Declared controls beyond text/buttons | Remaining functional requirement |
|---|---|---|
| LoadGame | 1 edit box, 1 list box | Save enumeration/selection and load/cancel dispatch |
| SaveGame | 1 edit box, 1 list box; Save/Cancel/Delete buttons | Naming, save/delete dispatch, overwrite policy and persistence |
| MapSelectionScreen | 1 list box; OK/Cancel buttons | Map choices and selected-map contract |
| BattleOptionsScreen (Preferences) | 12 radio buttons, 2 sliders; OK/Cancel buttons | Settings snapshot, radio grouping, slider semantics, apply/cancel and persistence |
| MultiplayerGameSelect | 1 list box; OK/Cancel buttons | Live session enumeration and selection |
| JoinMultiplayerScreen | 3 radio buttons, 1 edit box; OK/Cancel buttons | Legacy transport choices and network connection behavior |
| SetMultiplayerScreen | 3 radio buttons, 2 edit boxes; OK/Cancel buttons | Legacy transport choices and session creation behavior |
| RegionEntry | 4 radio buttons, 3 sprite-backed standard buttons; Enter Region/Cancel | Difficulty, region data and navigation semantics; icons can become labeled buttons after action mapping |

SinglePlayerBattle declares 17 sliders, 6 picture boxes, 4 standard buttons
and 40 text sections, in addition to Cancel/Start/Map. MultiplayerBattleSetup
also combines dynamic player/chat/setup controls. Both require substantially
more state than a navigation menu even if decorative assets are omitted.
CharacterScreen has stat/sprite bars and twelve standard buttons; its state
and character progression are additional dependencies.

EndGameSummary's file identifies itself as an example and uses zero rectangles
for its controls. It is not evidence of a ready usable screen layout.
ExperienceProcessing declares only one text section; this alone does not
describe its runtime flow. HighScores also needs further investigation rather
than readiness inferred from its filename.

## Implementation order and evidence boundaries

Suggested order for simple functional widgets: Quick Battle navigation,
mode-specific Mini Menu, then text result screens with explicit data models.
Extract reusable configuration/text/button presentation from MainMenuWidget
instead of extending its fixed six-action enum to unrelated screens. Keep
screen-specific actions and mode/state models in their own controllers.

Native previews and navigation among native widgets can be implemented now.
Original engine actions, pause/resume and return transitions still require the
observation/adapter steps in
[the main-menu assessment](../runtime/main-menu-qt-migration.md). This inventory
does not establish live replacements or change the coverage ledger milestone.
Original-manifest verification is performed before and after installed asset
inspection. No proprietary sprite/font decoding is required for a text/button
approximation.

Victory and Defeat now have [native result previews](../runtime/battle-results-qt.md)
with explicit supplied display data. Original result population/visibility and
continuation remain unconnected. Visual fidelity is documented separately from
layout and functional validation.

Quick Battle End now has [native result previews](../runtime/quick-battle-results-qt.md)
with four supplied player slots and exclusive Spectate/Continue states. Portraits
use text placeholders; original result data and action availability remain open.

Map Selection now has a [native list preview](../runtime/map-selection-qt.md)
with caller-supplied IDs, keyboard selection and OK/Cancel intent. Installed map
enumeration and original battle setup/return semantics remain unconnected.
