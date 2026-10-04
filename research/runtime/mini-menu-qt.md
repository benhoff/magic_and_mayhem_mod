# Native campaign and battle Mini Menus

Implemented 2026-10-04 in `apps/qt-shell/mini_menu_widget.*`. These are
standalone native UI previews; original pause/resume, saves, preferences and
quit actions remain in the game and have no adapter in this milestone.

The caller selects `MiniMenuWidget::Mode::Campaign` or `Mode::Battle`.
Campaign shows configuration sections TEXTBUTTON_1 through _5: Load Game,
Save Game, Preferences, Quit Game, Cancel. Battle shows _6 through _8:
Preferences, Quit Battle, Cancel. Inactive controls are hidden and disabled,
and action dispatch additionally checks the active mode. Changing mode moves
focus to its first button. The widget emits semantic `actionRequested(Action)`;
QuitGame and QuitBattle are distinct intents. Escape emits Cancel and ignores
autorepeat. These are native policies, not recovered original key semantics.

Shared menu helpers load labels and Rect2 coordinates through bounded read-only
AssetStore access. Widget reloads retain the previous display on failure.
The installed background differs from the preceding menus: it is a BMP panel,
not a JPEG filling the screen. Read-only inspection confirms these image sizes:

| Installed path | Decoded size |
|---|---|
| Interface/MiniMenu/800x600/Realm View Mini Menu 800-600.BMP | 600x400 |
| Interface/MiniMenu/640x480/Realm View Mini Menu 640-480.BMP | 480x320 |

The native preview centers the 600x400 panel at (100,100) inside its 800x600
canvas with black surroundings, then scales panel and buttons with letterboxing.
Centering is a presentation policy consistent with the rectangles, not a
confirmed engine placement. System serif typography approximates original fonts.
The same bitmap is used for both modes because the inspected CFG declares one
BackgroundFile; live battle appearance has not been compared.

```bash
./tools/run-qt-shell.sh --mini-menu campaign
./tools/run-qt-shell.sh --mini-menu battle
ctest --test-dir working/build/qt-shell -R 'qt-(mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

The preview loads main/Quick Battle assets for navigation, and loads Mini Menu
assets only for a requested Mini Menu preview. Cancel/Escape returns to the main
preview; this does not resume gameplay. Other actions report selection, including
quit intents, without closing the preview or launching the game. Missing assets
return status 9; invalid mode/combined entry points return CLI usage status 2.

Synthetic tests cover the configured labels, all eight section-to-action
mappings, inactive controls, mode changes/focus, Space, Escape/autorepeat,
wide/tall geometry, exact BMP pixels, panel placement, invalid coordinates,
corrupt images and retained prior state. Preview tests cover both modes,
Cancel/Escape return and distinct quit intents without window closure.
All five targeted Qt CTests pass. Installed-asset offscreen smoke checks pass
for both modes; Xvfb screenshots were visually inspected at
`working/tests/mini-menu-preview/campaign.png` and `battle.png`. No game was
launched. Original-manifest checks passed before and after installed asset use.
CLI checks also confirm status 2 for invalid modes/combined entry points and
status 9 for a missing installation.

Confidence is high for the literal CFG and decoded dimensions; synthetic native
behavior is separate from engine equivalence. Live state availability, pause
ownership, context-sensitive actions and return transitions remain pending in
[the coverage ledger](coverage-ledger.md). See also
[the layout inventory](../formats/menu-migration-inventory.md).
