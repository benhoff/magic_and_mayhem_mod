# Native Multiplayer Game Selection preview

Implemented 2026-10-04 in `apps/qt-shell/multiplayer_game_selection_widget.*`.
This is standalone Qt presentation with caller-supplied session IDs and names.
It performs no discovery, transport initialization or joining.

## Installed evidence and fidelity

Read-only inspection of
`Interface/MultiplayerGameSelect/Screen (Multiplayer Game Selection).cfg`
confirms one LARGE/MIDDLE heading (string 66, Select Game), a SMALL list box,
and LARGE OK/Cancel buttons (strings 10/11). Rect2 positions on the 800x600
canvas are heading `(100,82,600,50)`, list `(165,175,470,270)`, OK
`(75,475,200,50)` and Cancel `(525,475,200,50)`. The background is
`Multiplayer Game Select 800-600.JPG`. Confidence is high for these installed
configuration contents; they do not establish runtime session behavior.

The widget reuses the installed art, labels and positions. System serif fonts,
control borders, selection colors and scaled letterboxing remain native
approximations. No pixel comparison against the original running screen or
live network behavior was performed. See [visual fidelity](battle-results-qt.md#visual-fidelity).

## Native behavior and boundaries

`setSessions()` accepts opaque, unique nonempty IDs and nonempty display names;
duplicate names are allowed. It rejects invalid input and unknown explicit
selection IDs before mutation. Refresh preserves the selected ID across reorder,
clears removed selections and never silently selects the first session. Empty
lists and a current item without a selection cannot confirm. Enter confirms
once, repeated Enter is suppressed, activation confirms and Escape cancels.
The widget emits `sessionSelected(id)` or `cancelled()`; it knows no engine
addresses, session handles or wire formats.

Join's OK opens the sample session list, retaining the submitted username and
transport in preview orchestration. Session OK now opens the [sample guest lobby](multiplayer-lobby-qt.md);
lobby Cancel returns to this list. Cancel returns to the existing Join form and focuses OK;
its local draft survives. Create now opens the sample host lobby.
Asset failure while opening selection leaves the Join form available and its
draft intact. The CLI route also supplies a Join form as the Cancel destination.
Preview orchestration keeps two explicitly labeled sample sessions; these are
not discovered or verified games. No successful connection is simulated.

```bash
./tools/run-qt-shell.sh --multiplayer-game-selection
./tools/run-qt-shell.sh --join-multiplayer
ctest --test-dir working/build/qt-shell -R 'qt-(multiplayer-game-selection|multiplayer-setup|preferences|save-game|load-game|map-selection|quick-battle-results|battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

All thirteen targeted Qt checks passed. Three installed-asset smoke checks and
conflicting-preview-flag rejection passed. Synthetic checks cover stable IDs, duplicates, empty/removed/cleared selection,
transactional rejection, refresh, keyboard input, geometry, background,
asset failure and Join/selection/Cancel routes. Installed-asset smoke checks
and a visually inspected capture at
`working/tests/multiplayer-game-selection-preview/preview.png` cover presentation.
Original-manifest verification surrounds original-derived artifact use.
Discovery, session expiry, transport support, connection errors, original
flow semantics and the engine command adapter remain outstanding.
