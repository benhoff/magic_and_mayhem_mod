# Native Load Game preview

Implemented 2026-10-04 in `apps/qt-shell/load_game_widget.*`, using caller-supplied
save identifiers and filenames. The preview does not enumerate real saves, read
save contents or perform an engine load.

## Installed inputs and native behavior

Read-only inspection confirms `Interface/LoadGame/screen (Load Game).cfg` supplies
Load/Cancel buttons, one edit box, one list box and two text sections. The
installed `Load Game 800-600.JPG` supplies the 800x600 background. Heading TEXT_2
uses string ID 1, Load ID 47, Cancel ID 11. The separate TEXT_1 is Loading...,
ID 48, overlapping the list. That progress overlay is omitted because this
preview never performs a load; its live activation/rendering remains unverified.

Rect2 values, expressed as x/y/width/height, are heading `(230,55,345,47)`,
filename `(163,150,474,42)`, list `(165,233,470,267)`, Load `(70,530,150,50)` and
Cancel `(580,530,150,50)`. Heading/buttons use LARGE, edit/list SMALL, heading
CENTRE. Confidence is high for these installed configuration facts.

The `Save` model separates an opaque ID from its displayed filename. Neither
is interpreted as a path. IDs and names must be nonempty and unique; native
filenames are single-line strings capped at 256 characters. Validation rejects
invalid data or absent explicitly selected IDs before changing the UI.
These constraints are native presentation policy, not recovered save-format
or engine filename rules.

List selection fills the filename field. Editing it selects an exact,
case-sensitive matching supplied filename; unknown names clear selection and
disable Load while retaining the typed text. This avoids inventing engine
behavior for filenames absent from the supplied model. Refresh retains the
selected ID when it still exists, updating its filename, or clears both when
removed. No first save is implicitly selected. Explicit selection is supported.

Load emits `loadRequested(id)` only for a matching selected entry. Enter in the
list/editor confirms once, suppressing autorepeat; standard list activation
also confirms. Qt arrow keys navigate, Space activates buttons, Escape cancels,
and Enter on focused Cancel cancels. Focus enters a populated list or Cancel
when empty. These bindings and selection rules remain native policy.

The preview supplies three clearly labeled sample saves and explicitly selects
sample-autosave. The Main preview's Load Game action now opens this screen.
Cancel returns to Main; Load reports selected intent and stays on the screen
with the engine adapter pending. No loading animation or engine transition is
simulated. Live load/cancel/error handling, installed save enumeration, filename
normalization and original load state remain separate integration milestones.

## Fidelity, validation and use

Original artwork, configured positions and fixed labels are reused. System
serif fonts, edit/list colors, borders, padding, selection and button states
remain approximations; uniform scaling and letterboxing are native policies.
There is no original screenshot comparison or live game validation. See the
[visual-fidelity boundary](battle-results-qt.md#visual-fidelity).

```bash
./tools/run-qt-shell.sh --load-game
ctest --test-dir working/build/qt-shell -R 'qt-(load-game|map-selection|quick-battle-results|battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

All nine targeted Qt tests passed. The new synthetic test covers labels,
empty guards, filename/list synchronization, stable-ID refresh, unknown typed
names, Enter/repeat behavior, transactional invalid data/layout rejection,
removed selections, child-control Cancel, scaling/background, Main navigation,
pending load intent and corrupt images. Installed-asset smoke and conflicting
preview rejection passed. The offscreen capture
`working/tests/load-game-preview/preview.png` was visually inspected.
Original-manifest verification surrounds installed-artifact use; no original
game was launched for this milestone.
