# Native Map Selection preview

Implemented 2026-10-04 in `apps/qt-shell/map_selection_widget.*`. This is an
independent Qt preview with caller-supplied maps; it does not enumerate installed
maps, parse map files, configure a battle or issue engine commands.

## Inputs and behavior

Read-only inspection confirms `Interface/MapSelectionScreen/screen (Map Selection
Screen).cfg` declares one heading, one list box and OK/Cancel text buttons.
The installed `Map Selection Screen 800-600.jpg` supplies the 800x600 background.
String-table IDs 49, 10 and 11 supply Map Selection, OK and Cancel. Rect2 values
are heading `(234,62,334,53)`, list `(165,178,470,270)`, OK `(70,530,150,50)` and
Cancel `(580,530,150,50)`. The heading uses CENTRE/LARGE, list SMALL, buttons
LARGE. Confidence is high for these installed configuration facts.

`Map` has an opaque ID and display name; IDs are not interpreted as file paths.
`setMaps()` rejects empty IDs/names, duplicate IDs and absent explicitly selected
IDs before altering the list. Duplicate display names are permitted. Refresh
preserves the selected ID if it remains present, or clears selection if removed.
There is no implicit first-map choice. An explicit selected ID can initialize
the list. Empty/no-selection states disable OK and cannot emit confirmation.

Clicking OK or activating a selected list item emits `mapSelected(id)`.
The list supports Qt arrow-key navigation; Enter is consumed exactly once by
the widget's list event filter, suppressing autorepeat and avoiding duplicate
Qt activation/parent handling. Space uses native button activation. Cancel and
Escape emit `cancelled()`; Enter on the focused Cancel button also cancels.
Keyboard shortcuts, list styling, initial focus and activation are native
policies, not validated original runtime behavior. Focus enters the list when
populated and Cancel when empty. Text is displayed literally, not as HTML.

The preview supplies three clearly labeled sample maps and explicitly selects
sample-forest. Both OK and Cancel return to native Quick Battle navigation;
OK reports the opaque selected ID. No selection is passed to the engine.
Actual map availability, names, ordering and original return/confirmation
semantics remain future adapter/data-contract work.

## Visual fidelity and validation

Original background artwork, configured positions and static labels are reused.
System serif fonts, native list border/padding/selection colors and shared button
states are approximations. Uniform scaling and black letterboxing are native
policies. There was no original screenshot comparison or live game execution.
See [visual-fidelity boundaries](battle-results-qt.md#visual-fidelity).

```bash
./tools/run-qt-shell.sh --map-selection
ctest --test-dir working/build/qt-shell -R 'qt-(map-selection|quick-battle-results|battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

All eight targeted Qt tests passed. The new synthetic test covers installed-style
layout/font roles, empty-selection guards, stable IDs despite duplicate labels,
keyboard selection/single confirmation/repeat suppression, refresh preservation,
removed selections, invalid model transactional rejection, Cancel from child
controls, scaling/background, corrupt inputs and native preview routes.
Installed-asset smoke and conflicting-preview rejection also passed.
The offscreen capture at `working/tests/map-selection-preview/preview.png` was
visually inspected. Original-manifest verification surrounds artifact use.
