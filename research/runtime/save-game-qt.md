# Native Save Game preview

Implemented 2026-10-04 in `apps/qt-shell/save_game_widget.*` with supplied save
IDs/names and Save, Delete and Cancel intents. No save file is created,
overwritten or deleted, and no engine command is issued.

## Installed inputs and native behavior

Read-only inspection confirms `Interface/SaveGame/screen (Save Game).cfg` declares
an edit box, list box, Save/Cancel/third text button and two text sections.
The third button's comment says Cancel, but its text ID 94 resolves to Delete;
the widget uses the string table. Heading ID 75, Save ID 83 and Cancel ID 11
also come from installed strings. The installed `Save Game 800-600.JPG` supplies
the 800x600 background.

Rect2 x/y/width/height values are heading `(230,55,345,47)`, filename
`(163,150,474,42)`, list `(165,233,470,267)`, Save `(70,530,150,50)`,
Delete `(325,530,150,50)` and Cancel `(580,530,150,50)`. LARGE/SMALL roles and
CENTRE heading alignment are validated before committing loaded assets.
Confidence is high for installed inputs; original runtime semantics remain
unverified. TEXT_1 is the overlapping Saving... overlay and is omitted because
this preview performs no save.

The `Save` model separates opaque IDs from displayed names. It retains supplied
order, preserves selection by ID when refreshed and clears removed selections.
Invalid models are rejected transactionally. IDs/names must be nonempty and
unique; filenames are capped at 256 characters, single-line, nonblank, exclude
path separators/colon/NUL and cannot be dot/dot-dot. These are native UI policies,
not a recovered complete Windows/engine filename contract. No extension or
filesystem normalization is inferred.

List selection fills the edit field. Exact case-sensitive name matches select
an existing supplied save and enable Delete. Valid new names enable Save with
an empty existing ID; unknown names do not enable Delete. Empty/invalid names
disable Save. Displayed names are plain text.

New-name Save emits `saveRequested(name, emptyId)` immediately. Existing-name
Save opens an asynchronous native overwrite question, defaulting to No. Delete
opens a corresponding default-No question for the selected opaque ID. Yes emits
`saveRequested(name, existingId)` or `deleteRequested(id)`; No emits nothing.
The supplied list is unchanged until its owner provides updated data. Duplicate
requests cannot open multiple dialogs. Model/filename/selection changes, asset
reload or hiding the screen invalidate and dismiss pending questions; stale
responses cannot emit intent. These dialogs and policies are native behavior,
not recovered original overwrite/delete logic.

Enter in the list/edit field requests Save once; autorepeat is suppressed.
Enter on Delete/Cancel requests that focused action, Space uses Qt buttons,
and Escape cancels. Initial focus is the list when populated, Cancel when empty.
The campaign Mini Menu Save button opens this preview; Cancel returns to that
Mini Menu. A standalone preview returns to Main. The preview reports sample
Save/Delete intent as adapter pending and does not simulate completed persistence.

## Fidelity and validation

Original artwork, fixed labels and rectangles are reused. Native serif fonts,
edit/list/button colors, borders, padding and Qt confirmation dialogs remain
approximations. Scaling/letterboxing and modal questions are native policy.
No original-screen screenshot comparison or live game execution was performed.
See [visual-fidelity boundaries](battle-results-qt.md#visual-fidelity).

```bash
./tools/run-qt-shell.sh --save-game
./tools/run-qt-shell.sh --mini-menu campaign
ctest --test-dir working/build/qt-shell -R 'qt-(save-game|load-game|map-selection|quick-battle-results|battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

All ten targeted Qt tests passed in the normal build. An isolated build also
passed all eight menu fixtures and the shell help/startup checks while the
shared audio work was temporarily awaiting a source file. Synthetic checks cover new-name intent, overwrite/delete Yes/No, default No,
duplicate questions, stale question invalidation, availability/name constraints,
refresh, transactional failures, keyboard/repeat behavior, geometry/background,
pending sample intent and caller-specific Cancel routes. Installed-asset smoke and conflicting-preview rejection passed. The offscreen
capture `working/tests/save-game-preview/preview.png` was visually inspected.
Original-manifest checks surround installed-artifact use. Actual persistence, filename/overwrite policy, engine availability and
live replacement remain separate integration work.
