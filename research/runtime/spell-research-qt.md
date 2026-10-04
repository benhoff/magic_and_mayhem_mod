# Native Spell Research view

Reviewed 2026-10-04. Offline presentation and semantic requests; original gameplay
research rules and live replacement are unvalidated. Confidence is high for the
installed text/art inputs and tested native widget policies.

## What is original, and what is native

Installed `RlmBtn.spr` frames 6/7 visibly label Spell Research. Inspection found no
separate research-screen directory, background or control CFG under Interface.
This is not evidence of a recovered original research-screen contract. The new
`SpellResearchWidget` is a native browsing/request view using the installed
Grimoire book artwork, its chapter-five spell prose and shared SFT font roles.
Original research costs, prerequisites, clocks, knowledge progression and spell
identity mapping are not inferred from button lettering or static descriptions.

Inputs are native-loaded Grimoire `Backdrop.JPG`, `GenericL01.JPG`, `GenericR01.JPG`,
`icons.spr`, the existing bounded Grimoire catalog/parser and the four menu SFT
fonts. The two generic pages remain 400x600 halves of the 800x600 canvas. The
existing JPEG magenta text-region/blue transparency treatment was extracted to
`loadGrimoirePageArt` and shared with Grimoire rather than duplicated. The native
research layout fits headings, a filter and list on the left page, and a spell
heading, state, scrollable plain-text description and Research action on the right.
The original vertical close-button sprite is retained. Native scrollbars use the
existing brown/gold palette; neither their styling nor this layout is claimed as
original screenshot parity.

The Realm Viewer exposes its previously unused Spell Research sprite pair in
its original button sequence between Spell Selection and Grimoire. The five
semantic auxiliary indices retain existing Spellbox/Grimoire/Character/Options
values; Research is appended as index four. Its availability is caller supplied.
The footer now places Cancel/OK/Spell Selection/Research/Grimoire at X=10/110/210/
310/410, Character at 510, Options at 575 and the elided region heading at 665.
These positions remain native policies. Returning from Research restores the
map and its Research button focus. Standalone Research returns to Main.

## Supplied model and request boundary

`Catalog` owns an owner ID, at most 256 unique-ID spells and a selected spell ID.
Each `Spell` owns a name, plain description, availability and learned state.
Descriptions are bounded to 65,536 characters individually and one million
characters in total. Names/IDs are nonempty bounded single lines; duplicate or
unknown selected IDs, NUL text and oversized inputs are rejected atomically.
An empty catalog is valid for a known owner, and research is disabled.

Filtering is case-insensitive and does not change the stable selected ID. A
filtered-out selection clears the visible details and disables Research; clearing
the filter restores it. Selecting a row preserves literal text (including
markup-like labels). Learned and unavailable spells remain inspectable and cannot
request research. Clicking Research or pressing Enter on an eligible selection
emits `Request{ownerId,spellId}`. The widget does not change learned state, spend
resources or advance a clock; callers must supply authoritative refreshed models.
Escape/Close return to the caller. Auto-repeated Enter/Escape is suppressed.

The standalone preview derives 42 sorted display entries from Grimoire chapter
five. Each description joins the pages of the lowest stored knowledge level,
omitting dynamic labels with unresolved values. `grimoire-spell-{section}` IDs
and `sample-character` ownership are explicit native preview identities; they
are not engine IDs. All sample spells are exposed as available/unlearned, which
is not a campaign snapshot. Reopening retains the supplied catalog/selection.
Required art/font inputs validate before replacing the current presentation;
failed navigation leaves the caller active. The menu audio controller binds
research and close intents through its existing semantic cue policy.

## Validation and reproduction

`qt-spell-research` covers supplied-model bounds/duplicate/selection rollback,
filtering, literal prose, learned/unavailable/empty-result guards, typed requests
without invented progression, Enter/repeat/Escape/Close, canvas/font scaling,
asset rollback, installed chapter extraction and direct Realm opening/return,
caller availability, retained selection and failed-open caller preservation.
The installed audit traverses all 42 spell descriptions and checks every control
uses SFT families without synthetic bold at 800x600, 400x300 and 1200x600. The
shared font audit now includes 22 installed menu variants. All 26 targeted Qt
checks pass. Standalone `--spell-research --smoke-test` passes; conflicting
`--spell-research --grimoire` flags exit with status 2. An isolated build of the
scoped commit also passes the four affected-screen checks. The inspected native
capture is `working/tests/spell-research/research.png`.

```bash
./tools/run-qt-shell.sh --spell-research
cmake --build working/build/qt-shell --target spell-research-test --parallel 4
QT_QPA_PLATFORM=offscreen working/build/qt-shell/spell-research-test working/game-nocd
ctest --test-dir working/build/qt-shell -R 'qt-spell-research' --output-on-failure
```

Original manifests verified before/after installed-asset experiments (2,927
files). See [Grimoire format](../formats/grimoire-text.md),
[Realm Viewer asset evidence](../formats/realm-viewer-assets.md) and
[font integration boundaries](menu-font-integration.md). Remaining work is the
original research caller contract, campaign knowledge/state/cost identities,
paired visual comparison and engine-thread action dispatch.
