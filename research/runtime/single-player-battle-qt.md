# Native Single Player Battle Setup preview

Implemented 2026-10-04 in `apps/qt-shell/single_player_battle_widget.*`.
This milestone is native standalone presentation and local preview navigation;
no original game process, setup adapter or battle launch occurs.

## Installed layout evidence

Read-only inspection of
`Interface/SinglePlayerBattle/screen (Single Player Battle).cfg` confirms 40
SMALL text sections, 17 sliders, four standard buttons, six picture boxes and
three SMALL text buttons (Cancel 11, Start 53, Map 52). Heading 87 is Single
Player Battle. Confidence is high for these configuration contents and the
installed string table; runtime setup contracts remain unverified.

The installed `Single player Battle Setup 800-600.JPG`, all Rect2 positions,
visible string IDs, literal strings and slider bounds/steps are reused. Text
configuration uses `Textflags` (lowercase f), including LEFT and MIDDLE.
Quoted literals such as `"0"`, `""` and `"Large Medieval Forest"` are literals,
not string IDs. Empty text sections 9–12 overlap the handicap sliders and stay
hidden. Dynamic player, map and numeric displays come from the supplied model.

| Visible property row / native Rule | Min | Max | Step |
| --- | ---: | ---: | ---: |
| Mana | 50 | 200 | 10 |
| Health | 100 | 800 | 20 |
| Magic Items | 0 | 21 | 1 |
| Selection time | 30 | 3000 | 10 |
| Law Talismans | 0 | 7 | 1 |
| Neutral Talismans | 0 | 7 | 1 |
| Chaos Talismans | 0 | 7 | 1 |
| Game time | 0 | 240 | 5 |
| Lives | 1 | 20 | 1 |
| Places of power | 0 | 15 | 1 |
| Mana sprites | 0 | 50 | 1 |
| Artefacts | 0 | 50 | 5 |
| Control limit | 0 | 30 | 1 |
| Each of four player handicaps | 0 | 50 | 5 |

Slider comments 6/7 say chaos/neutral, but the visible text rows at their
coordinates say Neutral/Chaos. The native Rule names follow those visible
labels. This does not recover the engine's field mapping, ordering or units.
Time limits, zero-value special meanings, handicap interpretation and default
setup values remain unverified.

## Native model and actions

`Setup` supplies an opaque map ID/name, four player records (active flag,
name, portrait ID/text, colour ID/text, handicap), and thirteen property values.
Rule ordinals and IDs are native semantic choices, not engine offsets or wire
values. Invalid ranges, off-step values, half-specified maps and incomplete
active players are rejected before model mutation. Asset loads validate all
sections and the current model before changing presentation.

Numeric labels track sliders. Pointer input snaps to the nearest configured
step; keyboard arrows use that step. Snapping is native policy. Preview defaults
use the configured minimum values, four sample players, sample portraits/colours
and a sample map; these are not recovered original defaults or gameplay changes.
Start requires a map, an active human slot 0 and at least one active opponent.
This guard is native preview policy, not a confirmed original game rule.

All portrait and colour areas use text buttons instead of original sprites.
They emit slot-specific change requests; preview orchestration cycles three
sample portraits and four sample colour labels. The two original boot controls
emit removal requests for slots 2/3; orchestration marks those slots inactive.
Their portrait buttons can restore a sample player. Inactive colour, handicap
and removal controls are disabled. Colour uniqueness and actual wizard catalogs
are not established. These actions mutate only local sample data.

Start emits the complete typed setup snapshot and stays pending on the screen.
Enter activates the focused button or submits from a slider, once; autorepeat
is suppressed. Escape/Cancel returns to Quick Battle and restores the initiating
button's focus. Reopening retains the local setup draft.

Map opens the existing sample Map Selection widget with the current map
selected. OK updates its caller's map ID/name; Cancel preserves the map.
Both return focus to Map and retain all other setup edits. Standalone Map
Selection still returns to Quick Battle. Failed asset loads report an error
while keeping the caller and draft available.

## Fidelity and validation

Original background, labels and geometry are preserved. System serif fonts,
control colors, slider handles, numeric formatting, sprite placeholders and
player selection interactions are approximations. Long labels shrink to fit
within their configured rectangles. Scaling and black letterboxing are native
policies. No screenshot comparison against the original running setup or live
engine validation is claimed. See [visual fidelity](battle-results-qt.md#visual-fidelity).

```bash
./tools/run-qt-shell.sh --single-player-battle
ctest --test-dir working/build/qt-shell -R 'qt-(single-player-battle|multiplayer-game-selection|multiplayer-setup|preferences|save-game|load-game|map-selection|quick-battle-results|battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

All fourteen targeted Qt checks passed. Three installed-asset smoke checks and
two conflicting-preview-flag checks passed. Synthetic tests cover slider bounds/steps, pointer snapping, keyboard operation,
required-player/map guards, complete request snapshots, literal text, inactive
controls, transactionally rejected models/assets, geometry/background, local
sample actions, Quick/Map caller navigation, preserved drafts and failure paths.
Installed-asset smoke and visual capture evidence is kept at
`working/tests/single-player-battle-preview/preview.png`. Original-manifest
checks surround original-derived artifact use. Engine settings/default recovery,
wizard and map enumeration, portrait decoding, field mapping and actual battle
creation remain outstanding.
