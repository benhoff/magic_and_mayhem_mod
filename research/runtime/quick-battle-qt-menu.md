# Native Quick Battle menu

Implemented 2026-10-04 in `apps/qt-shell/quick_battle_menu_widget.*`.
This is native widget presentation and standalone navigation; it does not
replace the original engine's menu or create/join/start a game.

`QuickBattleMenuWidget` loads the installed JPEG, heading ID 2, four button
labels and Rect2 coordinates through shared read-only `menu_assets.*` helpers.
These helpers also serve MainMenuWidget and retain size limits and validation.
Failed widget reloads retain the prior display. Asset resolution remains in
`assets/`; no Wine, game addresses or process staging enter either widget.
Typography uses a system serif font, not the original SFT glyphs.

Actions are `CreateMultiplayer`, `JoinMultiplayer`, `CreateSinglePlayer`, and
`Cancel`, emitted through `actionRequested(Action)`. Escape emits Cancel as a
native navigation policy; original keyboard equivalence has not been established.
The widget scales an 800x600 canvas with letterboxing and native keyboard focus.

`MenuPreview` owns the standalone main/Quick Battle navigation using
QStackedWidget. MainMenuWidget's QuickBattle action selects Quick Battle;
Cancel/Escape selects the main menu and returns focus to its Quick Battle
button. Other actions report selection without launching the game. Quit closes
the preview. Both screens' assets must load successfully before preview startup.

```bash
./tools/run-qt-shell.sh --main-menu
./tools/run-qt-shell.sh --quick-battle-menu
ctest --test-dir working/build/qt-shell -R 'qt-(main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

Validation: the four targeted CTests pass. Synthetic fixtures verify the four
action IDs, heading/label lookup, Space activation, Escape from a focused child,
wide/tall geometry, JPEG/letterbox pixels, invalid headings, missing labels,
corrupt images and preservation of prior state on failed reload. The preview
test covers main-to-Quick Battle navigation, Cancel/Escape return, focus transfer,
and direct Quick Battle startup. The existing MainMenuWidget test passes after
extracting shared helpers.

Installed-asset offscreen smoke checks pass for both preview entry points.
The Xvfb Quick Battle preview was visually inspected; screenshot and log are
in `working/tests/quick-battle-menu-preview/`. No game was launched. Original
manifest verification passed before and after installed asset use.

Confidence is high for synthetic native UI behavior and this host's installed
asset preview. Engine entry/exit states, available actions, network setup and
single-player setup transitions still need observation and a tested adapter.
Those boundaries remain separate from this milestone in
[the coverage ledger](coverage-ledger.md). Layout evidence is in
[the menu inventory](../formats/menu-migration-inventory.md).
