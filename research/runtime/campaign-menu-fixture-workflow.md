# Connected campaign menu fixture

This development fixture connects the native Realm Viewer, Region Entry and
production Character bridge/session/controller without a running game or a
progressed save. Its campaign, availability, caller stack, wizard, prices and
assets are explicitly synthetic. It does not unlock or modify original progress.

Realm and Region navigation is supplied by the fixture host. Character input
crosses the actual V18 mapped channel and waits for a synthetic engine peer's
acknowledgement. The separate private PE tests exercise the actual original
Character getter/pricing/callbacks; they remain a distinct result.

The bounded journey checks Realm selection, Character buy/Cancel rollback,
buy/OK accepted baselines, reopening from both caller identities, preserving
region/difficulty/focus, Escape, disabled actions and return to the selected
Realm flag. It also retains three companion Qt tests and a synthetic screenshot.
Actual Realm command projection/dispatch, genuine campaign entry/progression,
save persistence, original tutorials/resources, complete window-close unwind,
pause and live replacement remain pending.

After configuring/building `apps/qt-shell` with testing enabled, review it with:

```sh
working/build/qt-shell/campaign-menu-fixture --interactive
```

The default mode runs a bounded automated journey and prints a JSON result.
`--capture <path>` additionally writes the final synthetic Realm view. Region
gameplay entry and unsupported auxiliary actions are disabled in this fixture.
No Wine process, installation staging or original media is needed.
Synthetic one-pixel sprite placeholders use readable text-button fallbacks in
this fixture. Its screenshots document navigation and control layout, not
original artwork fidelity.
