# Native Realm Viewer preview

RealmViewerWidget displays the three original realm-map BMPs and supplied
regions with explicitly selected SPR flag frames. It is a native offline
campaign-map preview, not a live campaign service or replacement of original
menu execution. Main Menu's New Game now opens it; --realm-viewer opens the
same sample campaign directly. No engine commands or game process are started.

## Model, input and navigation

Campaign carries opaque ID/name, three region lists, realm and auxiliary
availability, active realm and a remembered selected ID for each realm. Regions
carry opaque ID/name, explicit Region Entry artwork ordinal, supplied flag-cell
position/frame, availability and Region Entry auxiliary availability. No banner
selection or unlock state is inferred from those IDs. Duplicate region IDs or
art ordinals, invalid realms/selections/art/positions, and oversized inputs are
rejected before mutation. Native limits are 8/12/16 records per realm, IDs up to
128 and names up to 256 single-line characters, flag frame -1 or 0..88, and cell
positions X=0..752/Y=0..548. -1 retains literal text fallback. Empty lists have no
entry request. An unavailable region remains selectable for inspection, but OK,
Enter and double-click cannot open it. Caller-disabled realm/auxiliary controls
remain disabled.

A single flag click selects it. OK, double-click or Enter on a flag requests
Region Entry with typed campaign/region/realm/artwork fields. Enter repeat is
suppressed; Escape/Cancel returns to Main Menu with New Game focus. Tab traverses
realm selectors, flags and actions. Selection is retained separately for each
realm. Long heading text is elided with its complete plain-text tooltip and
accessible label retained.

MenuPreview resolves an accepted region request against the current supplied
campaign and atomically loads the Region Entry model/art. Region Entry Cancel
returns to the same map/selection with flag focus. Each campaign/region's chosen difficulty
survives return/reopen locally; opening a different region starts at Initiate.
A second campaign with the same region IDs has its own difficulty snapshot.
Region Entry's nested Character/Grimoire/Spellbox paths still return to Entry.
The map's direct Portmanteau/Grimoire/Character paths return to the map icon.
Options opens native Preferences and returns to the map on apply/cancel; this
Options routing is a preview policy, not a recovered original callback.
Standalone Region Entry/auxiliary return routes retain their own callers.
A failed map, catalog, auxiliary or selected-region asset load retains the caller
and previously accepted models/art. The Region Entry loader now accepts a supplied
region plus root in one transaction, avoiding a partially changed previous screen.

The application adapter binds accepted Realm Viewer open/auxiliary/cancel signals
to existing optional menu audio. UI widgets contain no audio session lifecycle,
engine addresses or thread command encoding. Flag selection/realm switching do
not invent an accepted engine command or progression update.

## Art and native display policies

Maps, button pairs and flags use bounded native BMP/SPR APIs. loadAssets validates
all three maps and required sprites/tooltips before replacing display assets.
The separate application catalog reads installed display names and first FP
positions through AssetStore/native FP decoding; it supplies illustrative
sample-* IDs and exposes all 36 regions. Those IDs and all-enabled availability
are authored sample state, not original campaign state. [Asset evidence and
FP placement limits](../formats/realm-viewer-assets.md).

The 800x600 map scales with 4:3 letterboxing. Static flags use explicit frame zero
in the sample, with original signed origins, a 48x52 canvas and a selected outline.
The sample uses the first stored FP flag position, shifted upward 52 pixels and
clamped at the top as a native placement policy. Native realm selectors occupy
(530+90*r,10,84,39), and the elided heading (600,550,190,39); final captures verify
that these controls do not obscure the 36 sample flags. Cancel/OK/Spell Selection/
Grimoire use the original RlmBtn visual pairs at X=10/110/210/310,Y=550,94x39;
Character uses the shared face triplet at (410,530,60,60); Options is native text
at (480,550,94,39). Font, hit rectangles, hover/pressed pairing and placement remain
approximate. PCX region silhouettes/borders, flags.ani/path movement, Spell Research,
original fonts and exact screenshot equivalence are not implemented/verified.

## Verification

The synthetic qt-realm-viewer check covers bounded catalog reads, model validation,
all realms/remembered selection, availability guards, typed open/auxiliary signals,
Enter/double-click/repeat/Escape, literal labels, sprite/map/config rollback,
scaling/letterboxing, Main New Game navigation, per-region difficulty, nested/direct
Grimoire returns, selected-art failure and standalone-caller preservation.
All 23 targeted Qt menu/image/sprite/audio/help/startup checks pass.

An installed inspector under working/tests/realm-viewer-preview opens and returns
from all 36 Region Entry images, checks difficulty, all four direct auxiliary
returns, map reopen and Main New Game. Celtic-preview.png, Greek-preview.png and
Medieval-preview.png were visually inspected; a detected Medieval flag/control
collision was corrected before the final inspection. Installed --realm-viewer and
--main-menu smoke checks pass; conflicting menu flags exit 2. All 2,927 original
files verify unchanged before and after installed-asset experiments.

Campaign unlocking/ownership, actual wizard/loadout/knowledge snapshots, progression,
pause/session flow and engine-thread dispatch remain pending. These native checks
do not establish original live menu equivalence.

```bash
./tools/run-qt-shell.sh --realm-viewer
./tools/run-qt-shell.sh --main-menu
ctest --test-dir working/build/qt-shell -R '^qt-realm-viewer$' --output-on-failure
```
