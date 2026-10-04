# Native Grimoire preview

`GrimoireWidget` is an offline installed-book reader with a typed
Location (chapter, section, declared level, text-page index, illustration mode).
It is presentation only; campaign unlocks/knowledge, original dynamic stats,
research progression and live engine dispatch remain unconnected.

## Inputs and behavior

The read-only bounded catalog loader lives in grimoire_assets.cpp, separate
from QWidget presentation. It reads Grimoire.cfg, Grimoiretooltip.cfg and
Grimoire.txt through AssetStore, parses nested comments and chapter/section/level
markers and preserves titles/entry icons/declared levels/dynamic labels.
The default preview exposes all installed entries at their first declared level;
it does not infer the player's knowledge or unlock entries. setLocation permits
other explicitly declared levels and rejects unavailable IDs/levels/pages.
[Format evidence and limits](../formats/grimoire-text.md).

Chapter contents are virtual section zero. Left/right chapter tabs select the
same chapter contents. Double-click or Enter activates the selected list entry.
Previous/Next walk contents, entries and explicit text pages in configured
chapter order; they stop at either end. Page Up/Page Down navigate from controls
or reading text. Enter activates a focused button once, with autorepeat ignored;
contents Enter repeat is also ignored. Escape closes from child controls.
Long prose is plain text in a transparent, scrollable QTextBrowser; titles and
contents retain literal text rather than interpreting markup.

The Contents button returns to the current chapter. Artwork toggles a companion
P2 illustration into a two-page artwork view, and Read restores prose; it is
available only for entries with companion art. It is a native preview control,
not a recovered original button. Reading flow uses the largest usable magenta
text area; where neither original image offers room, GenericR01 provides a
readable right page. Article text, scrollbar wrapping and dynamic-stat dash
rows are native presentation. No fabricated engine values are shown.

JPEG images use the native JPEG API and SPR controls use the native menu sprite
bridge. Blue (R<40,G<40,B>200) and magenta (R>200,G<60,B>200) pixels become
transparent over the original Backdrop. Contiguous magenta row regions with
at least 50 marked pixels infer heading/text areas; only areas at least 100x20
are retained. Typeface, text flow, key thresholds and generic-paper substitutions
remain approximate. CFG anchors are used for close/page-turn/tab controls,
with SPR signed origins preserved. Contents/Artwork footer rectangles and paired
sprite hover/pressed assignments are native policies. Original 155-frame entry
icons, SFT fonts, block pagination and ClosedBook animation remain pending.

Catalog enumeration is bounded to 512 directory entries and rejects duplicate
chapter/section/page image prefixes. A known Backdrop file is resolved through
PathResolver to locate the case-correct directory; all image contents are then
opened through AssetStore. Selected page images are loaded lazily, so opening
the book is not proof that every image is valid. A failed catalog/page/art/sprite
load retains the caller, current book, reading location and displayed spread.

## Preview navigation

Region Entry's Grimoire icon opens this reader. Close/Escape restores Region
Entry's difficulty/model and focuses that icon. Reading location (including
artwork mode) survives close/reopen. Standalone --grimoire returns to Main Menu.
Asset/navigation failures are surfaced in the preview status bar; no original
engine calls or game process are made.

```bash
./tools/run-qt-shell.sh --grimoire
./tools/run-qt-shell.sh --region-entry
ctest --test-dir working/build/qt-shell -R 'qt-(grimoire|menu-sprites|menu-images|character-screen|region-entry|multiplayer-lobby|single-player-battle|multiplayer-game-selection|multiplayer-setup|preferences|save-game|load-game|map-selection|quick-battle-results|battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

## Validation

Synthetic checks cover nested comments, all eight chapters, sorting/source
order, explicit levels/pages, paragraph/dynamic handling, literal titles,
paired tabs, contents/entry/previous/next boundaries, companion Artwork/Read,
keyboard focus/repeat, invalid models/catalogs/layout/images/sprites, lazy-load
failure rollback, scaling/letterboxing, caller close/reopen/failure paths and
Region Entry difficulty/focus restoration. No proprietary source is copied
into the synthetic fixtures.

All 20 targeted Qt checks passed. Four installed-art smoke checks and three
conflicting-preview-flag checks passed. Complete installed traversal passed
for eight chapters, 148 entries/declared text pages, 321 dynamic labels and all
36 companion artwork views. One entry uses generic left art. Four reading captures and a companion-artwork capture are
retained at `working/tests/grimoire-preview/`. The installed Region → Grimoire
→ Close path also passed. All 2,927 original files verified unchanged before and after installed asset
use. Confidence is high within these offline input/native presentation
policies; original parser, knowledge, flow and screenshot equivalence remain
unverified. This is not live menu replacement.
