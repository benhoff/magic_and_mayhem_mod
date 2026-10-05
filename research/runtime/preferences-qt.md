# Native Preferences preview

Implemented 2026-10-04 in `apps/qt-shell/preferences_widget.*` with a local
settings snapshot and editable draft. It emits accepted settings or Cancel
intent; no engine option, display mode, audio level or configuration file changes.

## Installed inputs and local model

Read-only inspection of `Interface/BattleOptionsScreen/screen (Battle Options).cfg`
confirms eleven text sections, twelve radio buttons, two sliders and OK/Cancel.
The installed `Battle Options Screen 800-600.JPG` supplies the 800x600 background.
Labels, Rect2 positions, LEFT/CENTRE alignment and LARGE/SMALL roles are loaded
from CFG and the installed string table. Three blank text sections remain hidden.
Confidence is high for installed contents. The later
[engine contract recovery](preferences-engine-contract.md) establishes pinned
state/action mappings and isolated callback effects; live activation remains
unconnected.

Five independent exclusive groups follow the labels and layout:

| Group | Radio sections | Local choices |
| --- | --- | --- |
| Resolution | 1–2 | High / Low |
| Animation | 3–4 | Full / Cut |
| Dialogue scrolling | 5–7 | Fast / Medium / Slow |
| Game play | 8–10 | Fast / Medium / Slow |
| Border picture | 11–12 | On / Off |

These groupings are native policy inferred from the labeled configuration,
not proof of engine enum encoding. The game-speed comments mention frame rates;
the widget does not alter timing or assign rates to the local Speed enum.
Both sliders retain configured minValue `-5000`, maxValue `0`. Their units,
original mapping and audio application are not inferred from the range alone.
Native keyboard steps are 100, page steps 500. Slider bounds are validated and
reload rejects ranges incompatible with either accepted or draft state.

`Settings` contains music/sound raw levels, resolution, animation, two speed
choices and border-picture state. The sample defaults (-1500/-1000, High,
Full, Medium/Medium, border On) are native preview values, not installed-user
settings or recovered engine defaults. `setSettings()` validates bounds/enums
before replacing accepted and draft values. Asset reload preserves both states.

Editing controls changes only the draft. OK copies it to the accepted local
snapshot and emits `settingsApplied(snapshot)`. Cancel restores the accepted
snapshot and emits `cancelled()`. Enter accepts, Enter on Cancel cancels, Escape
cancels, and held Enter/Escape repeats are suppressed. Standard Qt arrow/Space
radio interaction and slider keys remain native behavior. The music slider
receives initial focus.

The preview opens from Main Preferences and both Mini Menu variants. OK/Cancel
return to the caller with focus on its Preferences button; standalone opens
return to Main. Locally accepted values survive reopening. The status message
identifies acceptance as local with engine/persistence pending. No save/config
file is read to establish initial settings and none is written.

## Fidelity and validation

Original artwork, labels and positions are preserved. System serif font sizes,
radio indicators, slider handles, colors and button states are native
approximations. Scaling/letterboxing and keyboard steps are also native policy.
No original screenshot comparison or live game validation was performed. See
[visual-fidelity boundaries](battle-results-qt.md#visual-fidelity).

```bash
./tools/run-qt-shell.sh --preferences
ctest --test-dir working/build/qt-shell -R 'qt-(preferences|save-game|load-game|map-selection|quick-battle-results|battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

All eleven targeted Qt tests passed. The new synthetic test covers group
exclusivity/independence, slider ranges, draft versus accepted settings, Cancel
rollback after editing and after prior apply, full-model apply signals,
keyboard confirmation/repeat suppression, invalid enum/range snapshots,
transactional asset reload, scaling/background, Main and both Mini Menu routes,
and accepted local values across reopening. Installed-asset CLI smoke and
conflicting-preview rejection passed. The offscreen capture
`working/tests/preferences-preview/preview.png` was visually inspected.
Original-manifest verification surrounds installed-artifact consumption.
Pinned engine option mapping, static availability and isolated apply/rollback/
writer effects now have [contract evidence](preferences-engine-contract.md).
Live audio/display application, persistence durability and caller equivalence
remain separate integration milestones.
