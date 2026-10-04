# Grimoire text and page inputs

Read-only installed inspection, 2026-10-04. Evidence is the installed
`Interface/Grimoire/Grimoire.txt`, Grimoire.cfg, Grimoiretooltip.cfg, JPEG pages,
native SPR decoding and complete offline reader traversal. No original engine
parser or live knowledge transition was observed.

## Text document

The supplied 136,325-byte Latin-1 text document includes an embedded explanatory
comment and eight actual chapter declarations, with 148 sections and 148 levels.
Source comments start `~[` and end `]`; nested comments can use either `~[` or
`[` inside an open comment. The native reader preserves line breaks while
stripping comments, caps nesting at 16 and rejects unterminated blocks/NULs.

| Marker | Observed meaning / native handling |
|---|---|
| `~C<n>-<icon>-"title"` | Chapter number 0..7; icon number preserved in source, chapter title displayed |
| `~S<id>-<icon>-"title"` | Entry/section number and icon ID; native catalog preserves section/icon and title |
| `~L<n>` | Level of text, documented as knowledge 0..4; native reader stores exact declared levels without inventing unlock rules |
| `~NL` | New paragraph; native reader inserts a blank line |
| `~NB` | Documented new block; native reader treats as paragraph separation, without reproducing original block layout |
| `~NP` | Documented new page; native reader starts another text page |
| `~D "label" ...` | Dynamic display declaration. Native reader preserves display labels; values are unavailable and shown as dashes |

The installed data has 321 dynamic declarations. Their payload includes size,
colour and symbol/selector fields; interpretation, numeric values and original
rendering are not recovered here. The embedded example is not counted as real
entries. Section zero is documented as virtual chapter contents. Unknown
native directives, duplicate chapter/section/level IDs, text before a level and
missing chapters/levels are rejected. Native limits are 256 KiB source, eight
required nonempty chapters, 512 total sections, five possible levels, sixteen
pages per level, 64 dynamic labels per level and 64 KiCharacters per page.
These are native bounds, not original engine capacities.

Grimoire.cfg defines eight Cn_SORTED flags. The installed introduction/realm
chapters retain source order; creature/spell/item/artefact chapters request
sorting. Native sorting is stable, case-insensitive QString title comparison;
original language/collation behavior remains unverified. The layout supplies
800x600/640x480 tab, page-turn and close anchors; this widget uses 800x600.

## Images and sprites

The installed 800x600 directory has 190 half-page JPEGs (400x600), plus Backdrop
and ClosedBook (800x600). Article names use chapter/section/page prefixes such
as C0S01P1 followed by a descriptive suffix. Prefixes, not title-to-filename
conversion, associate art with text: spelling, spaces and punctuation differ
between some text and image names. P1 is treated as left art and P2 as companion
right art in this native preview. Duplicate prefix matches are rejected.
One actual entry has no P1 match and uses GenericL01; 36 have P2 matches.

Decoded inspection shows blue outer areas and magenta rectangular text areas.
The native widget keys saturated blue/magenta transparent over Backdrop and
infers text rectangles from contiguous magenta rows. Thresholds and text flow
are native policies; original colour-key/layout behavior is not proven.
GenericL01/GenericR01 supply missing/readable page areas.

Native SPR inspection confirms 34 800x600 icon frames, 4 generic page-turn
frames and a separate 155-frame generic entry-icon sheet. The native widget
uses the close pair, eight left/right chapter-tab pairs and two page-turn pairs.
Frame/state association is based on decoded art and layout anchors. Entry icons
and original page-turn animation are not rendered. ClosedBook is not animated.
See [widget behavior and validation](../runtime/grimoire-qt.md).
