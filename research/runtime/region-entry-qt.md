# Native Region Entry preview

Implemented 2026-10-04 in `apps/qt-shell/region_entry_widget.*`. This is
standalone native presentation with caller-supplied region data and semantic
intent. It does not read campaign state, enter a region or open engine screens.

## Installed evidence

Read-only inspection of `Interface/RegionEntry/screen (Region Entry).cfg`
confirms two LARGE text buttons, four SMALL difficulty radios, three
sprite-backed standard buttons and one LARGE/LEFT title. The configuration
has `[GLOBAL]` with no background filename; `TEXT_1` contains the literal
`Current Region`, not a string-table ID. Confidence is high for these layout
contents and installed string labels.

| Control | Installed label / comment | Rect2 as x,y,width,height |
| --- | --- | --- |
| Enter | String 78, Enter Region | 50,510,275,50 |
| Cancel | String 11, Cancel | 475,510,275,50 |
| Difficulty 1–4 | Strings 79–82: Initiate, Apprentice, Adept, Wizard | x 50/225/400/575, y 90, width 175, height 25 |
| Standard button 1 | Grimoire, sprite indices 3/4/5 | 650,25,60,60 |
| Standard button 2 | Spellbox, sprite indices 6/7/8 | 715,25,60,60 |
| Standard button 3 | Character screen, sprite indices 0/1/2 | 585,25,60,60 |
| Current region title | Caller-supplied display name | 50,40,475,35 |

Auxiliary action names come from configuration comments and sprite-index roles.
Confidence is high for those comments/indices; actual engine dispatch and
availability have not been observed. The native widget validates the expected
index groups and renders the decoded Sprites/Buttons.spr triplets with full
tooltips and accessible names. State assignment, disabled dimming and original
file binding remain native policies pending live comparison; see
[SPR integration](menu-sprite-integration.md).

Unlike other menus, illustrations are named `Celtic_Region_01.JPG` without an
`800-600` filename suffix. Read-only installed-file enumeration finds Celtic
01–08, Greek 01–12 and Medieval 01–16 in the 800x600 directory. These counts
bound artwork selection only; they do not establish campaign IDs, unlocking,
realm progression or which image belongs to a supplied runtime region.
The existing bounded menu loader now accepts an explicit image basename,
rejecting separators, absolute/drive paths and dot traversal. Image size and
format checks remain in place. The widget generates filenames from a closed
Realm enum and validated numeric range.

## Native model and behavior

`Region` carries an opaque ID/display name, independently chosen artwork realm
and number, difficulty, Enter availability and three auxiliary availability
flags. IDs are caller-owned and are never used as filenames or engine addresses.
Names are plain text, single-line and bounded to 256 characters. Invalid enums,
artwork numbers or half-specified IDs/names are rejected before mutation. A
region with both ID and name empty may be displayed, with Enter/icons disabled.

The four radios are exclusive. The native default is Initiate; it is not a
recovered original campaign default. Enter emits `Request { regionId,
difficulty }` only for a nonempty ID with Enter enabled. Availability flags
also guard auxiliary requests. Enum ordinals follow visible rows as native
semantic values; engine difficulty encoding remains unknown.

Before assets are loaded, `setRegion()` updates the local model. After loading,
changing artwork performs a bounded load of the new illustration/layout/text
through the same root before committing any state. Missing/corrupt images or
invalid layouts preserve the old model, illustration, labels and selection.
Reloading preserves difficulty and availability. Same-artwork model updates use
the already validated presentation.

Enter from a radio emits one Enter request; repeated Enter is suppressed.
Enter on a focused auxiliary button or Cancel performs that button's action.
Escape cancels. Native Qt Space/radio/tab behavior is retained. Focus starts on
the selected difficulty. Scaling uniformly fits the 800x600 canvas with black
letterboxing; long titles shrink within their configured rectangle.

## Preview orchestration and fidelity

`--region-entry` supplies a sample Celtic region with the installed first
Celtic illustration. Enter and Spellbox report adapter-pending intent and remain
on the preview. C opens the [native Character Improvement preview](character-screen-qt.md);
its OK/Cancel return here with region/difficulty retained. Region Cancel returns
to Main Menu and focuses New Game; reopening
retains local difficulty. Main Menu's New Game action remains unconnected:
this preview does not establish the campaign flow or skip character creation.

Original region art, configured labels and geometry are reused. System serif
fonts, radio indicators, button colors and sprite interaction styling remain
approximations. No original screenshot equivalence or live campaign validation
is claimed. See [visual fidelity](battle-results-qt.md#visual-fidelity).

```bash
./tools/run-qt-shell.sh --region-entry
ctest --test-dir working/build/qt-shell -R 'qt-(region-entry|multiplayer-lobby|single-player-battle|multiplayer-game-selection|multiplayer-setup|preferences|save-game|load-game|map-selection|quick-battle-results|battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

All sixteen targeted Qt checks passed. The installed-asset CLI smoke check and
two conflicting-preview-flag checks passed. Synthetic checks cover four typed difficulty requests/exclusion, empty and
unavailable guards, literal names, accessible auxiliary actions, keyboard
handling, all three artwork families, range/model/layout/icon rejection,
transactional missing-artwork failure, reload, scaling/background, explicit
filename confinement and preview pending/Cancel/reopen/error routes.
Installed-art smoke and a visually inspected capture are retained at
`working/tests/region-entry-preview/preview.png`. Original-manifest verification
surrounds original-derived artifact use. Campaign selection/unlocking, region
ID and difficulty mapping, character/spell screens and the engine command
adapter remain outstanding.

Grimoire also opens its [native installed-book reader](grimoire-qt.md); Close/Escape restores the Region caller and Grimoire icon focus.

Spellbox navigation now opens the [native Portmanteau preview](spellbox-qt.md).
Accept/Cancel returns with the current region/difficulty and Spellbox icon focus.
Its accepted inventory draft is local; campaign loadout commands remain pending.

[Realm Viewer](realm-viewer-qt.md) now supplies selected region models/artwork.
Region Entry Cancel returns to that map flag, and difficulty is remembered locally
per region. The root/model asset load is atomic; failed region art preserves the
map caller and previous Entry model. Standalone Entry still returns to Main.
