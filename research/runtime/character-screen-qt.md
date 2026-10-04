# Native Character Improvement Screen preview

Implemented 2026-10-04 in `apps/qt-shell/character_screen_widget.*`, connected
from Region Entry's Character button. This is local native editing with supplied
character data/costs. It does not change original campaign or engine progression.

## Installed evidence and scope

Read-only inspection of `Interface/CharacterScreen/screen (Character Screen).cfg`
confirms 17 text sections, three stat bars, three talisman sprite bars, twelve
standard buttons and OK/Cancel. `CharacterScreentutorial.cfg` describes spending
experience to improve characteristics, OK accepting changes and Cancel undoing
changes. Confidence is high for these installed configuration/tutorial contents;
engine price computation and persistence remain unrecovered.

The installed 800x600 background is `CharacterScreen.JPG` (no `800-600` suffix).
Static labels come from the installed string table: Mana, Health, Control Limit,
Law/Neutral/Chaos Talismans, Experience Points, Cost, OK and Cancel. Character
name, experience balance, six costs and optional rating are supplied displays.
`VAR_EXPERIENCE` and the six `VAR_*COST` tokens are dynamic roles, not string IDs.
The name's default string 31 is Cornelius; the preview supplies its own sample
name and does not infer a campaign character from that default.

| Attribute | Configured bounds | Configured increment |
| --- | --- | --- |
| Mana | 0–200 | 5 |
| Health | 0–800 | 20 |
| Control Limit | 0–40 | 1 |
| Each of Law, Neutral and Chaos Talismans | 0–7 | 1 |

`STEPS_COSTS` provides these increment amounts and comments describing an
initial/linear/exponential price formula. The native preview reads the increment
amounts only. The engine's increment index, rounding, baseline, special cases
and purchase/refund accounting have not been recovered; costs are explicitly
caller-supplied rather than computed from those coefficients.

Standard buttons at x 660 use sprite indices 6/7/8, while those at x 750 use
9/10/11. Bounded offline native decoding of installed `800x600/sprites.spr`
frames 6 and 9 produced the inspected contact sheet at
`working/tests/character-screen-preview/buttons.png`; these show plus on the
left and minus on the right. Both normal frames are 30x30 RGB565. Confidence is
high for decoded glyph/coordinate association; original event dispatch remains
unobserved. The preview uses text +/− at those configured rectangles.

Sprite bars provide `Rect2=270,360/410/460,0,0`: these are anchors with zero
right/bottom fields, not valid rectangular areas. The native text replacements
are 350x30 at anchor+(0,15), aligned with the matching rows. This extent is native
policy. The three configured STATBAR `Text` BMPs now supply the original bar
textures. Installed 800x600 textures are 700x20, with a coloured 350x20 left
half and grey 350x20 right half; the widget clips the coloured half by the
current value and draws the grey half for the remainder. This rendering policy
is supported by asset inspection, not an observed original callback.

A caller-supplied `portraitIndex` selects WizardFace0/1/2.JPG; -1 retains text
fallback. Installed faces are 400x300 with a saturated blue backdrop. Native
presentation keys pixels with R<40, G<40, B>200 transparent and places the full
image at `(0,0,400,300)`, scaled with the canvas. The face aligns with the left
background frame in the inspected preview. There is no portrait rectangle in
this CFG; placement, blue-key threshold and face-to-progression mapping are
native policies pending original comparison. See [image integration](menu-image-integration.md).
The text fallback occupies `(25,50,180,150)`.

## Supplied snapshot, edits and requests

`Character` carries an opaque ID, display name, portrait text/index, optional rating,
experience points and six `Stat` records. Each Stat supplies its current value,
an ordered vector of costs for successive purchases from that snapshot and
upgrade availability. IDs are never interpreted as paths or engine addresses.
Attribute ordinals follow visible native rows, not recovered engine fields.
Names and display strings are bounded single-line plain text. Stat values must
fit configured domains. Costs are nonnegative, at most 1,000,000 each, bounded
to 1,000 entries and cannot extend beyond available increments to the maximum.
Experience is nonnegative and at most 1,000,000,000 as a native numeric bound.
Invalid snapshots preserve the prior accepted state and pending draft.

A purchase adds the configured increment and spends the next supplied cost.
All six attributes share the remaining experience balance. Buttons guard missing
character IDs, caller availability, insufficient budget, exhausted schedules
and maximum values. Explicit zero-cost purchases are supported. Next cost,
remaining experience and stat/talisman displays update immediately.

The minus button undoes the latest draft purchase for that attribute and refunds
that purchase's exact supplied cost. It cannot reduce the opening/accepted
baseline or refund historical upgrades. This refund policy is native preview
policy, not a recovered original contract. Missing cost entries disable further
upgrades and display an em dash; no cost extrapolation occurs.

OK emits a typed `Request` with character ID, resulting values, purchased counts
and remaining experience. It advances the local accepted snapshot and removes
consumed costs; reopening retains that local acceptance. No engine update or
persistence is claimed. Cancel/Escape discards all current purchases, preserving
the last accepted snapshot. Enter activates the focused button once; repeat is
suppressed. Asset reload validates the accepted model and draft before mutation,
and rejects increment changes that would reinterpret pending edits.

## Preview navigation, fidelity and validation

Region Entry's Character button opens the screen with sample stats, 100 experience
and sample cost schedules (5,10,15,20 per attribute). These are illustrative
caller data, not recovered starting stats or original costs. OK and Cancel return
to the same Region Entry instance, restore Character button focus and preserve
its region/difficulty. Standalone `--character-screen` returns to Main Menu.
Asset failure retains the caller and local snapshots.

Original background, labels and rectangular control positions are reused.
System serif fonts, text buttons/talisman counts, portrait transparency/placement,
bar clipping and scaled letterboxing remain approximations. No original
screenshot equivalence or live progression validation is claimed. See
[visual fidelity](battle-results-qt.md#visual-fidelity).

```bash
./tools/run-qt-shell.sh --character-screen
./tools/run-qt-shell.sh --region-entry
ctest --test-dir working/build/qt-shell -R 'qt-(character-screen|region-entry|multiplayer-lobby|single-player-battle|multiplayer-game-selection|multiplayer-setup|preferences|save-game|load-game|map-selection|quick-battle-results|battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

All seventeen targeted Qt checks passed. Two installed-asset smoke checks and
two conflicting-preview-flag checks passed. Synthetic checks cover all six increments, shared budgets, refunds, exact/zero
costs, maximum/availability/schedule guards, complete request snapshots,
accept/cancel/reopen semantics, rejected models/assets, dynamic role validation,
sprite anchors, reload preservation, keyboard input, geometry/background and
Region/standalone/error navigation. Installed-asset smoke and visual capture
are retained at `working/tests/character-screen-preview/preview.png`.
Original-manifest verification surrounds original-derived artifact use.
Original pricing/indexing/refund rules, campaign snapshots, portrait progression mapping,
character persistence and the engine command adapter remain outstanding.
