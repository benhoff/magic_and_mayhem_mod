# Native Victory / Defeat result previews

Implemented 2026-10-04 in `apps/qt-shell/battle_result_widget.*` with one
outcome-aware widget, a display-only results model and `continueRequested()`.
No game process is started and no engine rewards or progression are changed.

## Installed inputs and native behavior

Read-only inspection of `working/game-nocd/Interface/BattleEnd` confirms two
layouts, each with 21 text sections and one OK button. The layouts contain
`[GLOBAL]` with no background name. The widget explicitly selects the installed
`Battle Victory Screen 800-600.JPG` or `Battle Defeat Screen 800-600.JPG`.
Both decode to 800x600; the shared loader still validates format and dimensions.

The widget uses installed Rect2 rectangles, LEFT/RIGHT/CENTRE alignment and
LARGE/SMALL font roles. Fixed Achievement, Experience Points, Total, Rating
and OK labels come from the installed string table. Dynamic sections include
seven achievement/points pairs, total, maximum, summary and rating. The
`Results` model supplies strings for these fields and an optional title;
it performs no score calculation and infers no engine availability rules.
Qt labels use plain text, including strings resembling markup.

Unpopulated reward rows are hidden and cleared. Rating takes precedence over
totals because these areas overlap in the configuration. This is a native
presentation policy, not a recovered engine rule. Some seventh-row and summary
rectangles also overlap footer/reward areas; callers must supply the appropriate
visible result combination until original activation rules are recovered.

Realm files Celtic/Greek/Medieval supply VictoryText, DefeatText and map titles,
but their rendering/selection contract is not established. The optional native
title uses a banner rectangle `(50,25,700,55)` inferred from the artwork, not
from a configured rectangle. The preview uses English Victory!/Defeat! titles
and explicitly marked sample reward/summary/rating values. Localized realm
selection and actual engine result data remain future integration work.

OK, Enter and Escape emit continue intent; held-key repeats are ignored.
The preview returns to its native Main menu. These keyboard bindings and
navigation are native policy, not proven original result-screen behavior.
The 800x600 canvas scales uniformly with black letterboxing, and OK receives
initial focus. Reloads validate all inputs before replacing the visible state.

## Visual fidelity

Original background artwork is reused, retaining its ornamentation, texture
and colors as decoded by Qt. Static text, positions, alignment and font-role
names come from installed data. This preserves much of the game's visual
identity but does not establish a pixel-equivalent reproduction.

Like Main, Quick Battle and Mini Menu, text uses a system serif font rather
than the original bitmap font. The native text color `#3e2313` and shared
button hover/focus/pressed colors are selected approximations. No sampled
original color/palette or screenshot comparison validates them. Qt scaling,
font metrics, antialiasing, title positioning and interaction effects also
remain unverified against original rendering. Mini Menu panel centering is
another documented native policy. The prior live menu trace validates original
dispatch/transition ordering, not Qt visual equivalence.

A future fidelity pass needs original screenshots of each state and resolution,
original font metrics/rendering, palette/color recovery and comparisons covering
normal, focused, hovered, pressed and disabled text. Installed-asset preview
inspection alone must not be reported as original-screen visual verification.

## Validation and use

```bash
./tools/run-qt-shell.sh --battle-results victory
./tools/run-qt-shell.sh --battle-results defeat
ctest --test-dir working/build/qt-shell -R 'qt-(battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

All six targeted tests passed. The new synthetic test covers both outcomes,
seven-row data, blank/stale clearing, rating versus total visibility, configured
labels/alignment/font roles, plain text, input signals, focus, scaled geometry,
background selection, preview return and transactional corrupt-input rejection.
It establishes native behavior within this scope, not engine equivalence.

Both installed-asset CLI smoke checks passed. Offscreen previews were captured
and visually inspected at `working/tests/battle-results-preview/victory.png`
and `defeat.png`. Original-manifest verification surrounds installed-artifact
inspection and preview execution. Engine data integration, original result
activation/continuation rules and live replacement remain separate milestones.
