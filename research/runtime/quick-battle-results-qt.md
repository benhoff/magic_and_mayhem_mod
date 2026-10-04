# Native Quick Battle results preview

Implemented 2026-10-04 in `apps/qt-shell/quick_battle_result_widget.*`.
The widget consumes a display-only model and emits Spectate, Continue or Quit
intent. It does not start a game, spectate a session or calculate scores.

## Installed layout and native policies

Read-only inspection of `Interface/QuickBattleEnd/screen (Quick Battle End).cfg`
confirms 26 text sections, four picture boxes and three text buttons. The
installed 800x600 `Quick Battle End 800-600.jpg` supplies the background.
The title and five column headings come from the installed string table:
Game Over, Player, Kills, Deaths, Handicap Bonus and Score. Action labels use
IDs 67, 68 and 5. Rect2 positions, LEFT/MIDDLE alignment and LARGE/SMALL roles
are loaded from the CFG. Textflags uses that exact case in this layout.

Dynamic fields are column-major: sections 7–10 are player names, 11–14 kills,
15–18 deaths, 19–22 handicap bonuses, 23–26 scores. `Results` supplies four
explicitly active/inactive player slots, their display strings and text portrait
placeholders. Inactive slots are hidden and cleared, including old statistics
and portrait labels. Labels always use plain text.

Spectate and Continue have the same configured rectangle `(150,525,225,50)`;
Quit uses `(425,525,225,50)`. The model therefore selects one primary action:
None, Spectate or Continue, with a separate Quit availability flag. Hidden
primary controls are also disabled, and unavailable controls cannot emit actions.
This exclusivity is a native presentation policy supported by the overlapping
layout; the original runtime rules selecting actions remain unverified.

The first available button receives focus. If none are available, the widget
retains focus and emits nothing. Enter activates the focused available action;
Space uses Qt button handling; Escape requests Quit only when allowed. Enter
and Escape autorepeats are ignored. These bindings remain native policy.

The native preview uses four explicitly labeled sample players. Continue opens
the native Quick Battle navigation preview; Quit returns to native Main;
Spectate reports that the engine adapter is pending and stays on this screen.
These preview routes do not establish original engine transitions.

## Visual fidelity and boundaries

Original background artwork is reused, retaining its parchment and decorative
borders. Player portrait sprites are approximated by text within the original
picture-box rectangles. Native serif fonts, sizes and shared text/button colors
remain approximations. Handicap Bonus wraps within its configured heading box
as a native policy. The canvas scales from 800x600 with black letterboxing.

No original result-screen screenshot comparison, original font/color validation,
live result data or live replacement was performed. See the
[broader fidelity boundary](battle-results-qt.md#visual-fidelity). Static installed
layout evidence has high confidence; runtime activation, score interpretation,
player ordering and spectate/continue/quit behavior need separate recovery.

## Validation and use

```bash
./tools/run-qt-shell.sh --quick-battle-results continue
./tools/run-qt-shell.sh --quick-battle-results spectate
ctest --test-dir working/build/qt-shell -R 'qt-(quick-battle-results|battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

All seven targeted Qt checks passed. The new synthetic test covers four-player
column mapping, stale-row clearing, plain text, headings/font roles, mutually
exclusive controls, availability guards, focus fallback, Enter/Escape and
repeat handling, scaled geometry/background, transactional invalid reload,
corrupt-image rejection and native preview routes.

Installed-asset smoke checks passed for both primary actions, as did invalid
mode/conflicting-preview rejection. Offscreen captures were visually inspected:
`working/tests/quick-battle-results-preview/continue.png` and `spectate.png`.
Original-manifest verification surrounds artifact consumption. No original game
process was launched for this milestone.
