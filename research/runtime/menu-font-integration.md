# Native menu SFT font integration

Reviewed 2026-10-04. Offline Qt presentation; no injected engine replacement.

## Scope and evidence

All migrated QWidget menus now use one shared font service in
`apps/qt-shell/menu_fonts.{hpp,cpp}`. It consumes the owned native SFT reader;
Qt does not parse SFT input or read it through a plugin. The service generates
an in-memory TrueType outline font and registers it with Qt so ordinary labels,
buttons, lists, editable fields/cursors, text browsers and local chat retain
native Qt text layout and interaction. No installed asset is rewritten and no
font binary is checked into the repository.

| Role | Installed source | Native line size at the 800x600 canvas |
| --- | --- | --- |
| Heading / CFG LARGE | `Sprites/heading text 800.sft` | 32 pixels (24 ascent, 8 descent) |
| Body / CFG SMALL | `Sprites/body text 800.sft` | 22 pixels (17 ascent, 5 descent) |
| Compact controls, tooltips / CFG TINY | `Sprites/Tooltip Text.sft` | 15 pixels (12 ascent, 3 descent) |
| Spellbox quantity overlay | `Sprites/yellowtext.sft` | 14 pixels (11 ascent, 3 descent) |

These are font line sizes, not opaque glyph heights. The same four roles scale
with the existing letterboxed canvas; the 640 source fonts are not substituted
when resizing the 800-based layouts. Existing fitted headings and setup labels
may shrink within their rectangles. Synthetic emboldening has been removed.
Join/Create edit fields retain their configured LARGE role; Load/Save lists
and edits use SMALL. Grimoire prose uses Body, titles Heading, and compact
auxiliary controls Tooltip. Spellbox cells set an unscaled logical font inside
their existing painter transform, preventing scaling twice.

Registration names derive from the converted content. Simultaneous menus share
registrations through weak caching and owned references, and the final owner
unregisters them. Loading validates all four fonts before registration; missing,
partial or corrupt installed catalogs do not result in a partially installed
font set. A completely absent catalog supports synthetic menu fixtures with an
explicit `menuFontSource=fallback` property and the same role/scale policy.
Successful installed screens expose `menuFontSource=SFT`. Asset replacement
occurs after the rest of each screen's required inputs validate. QWidget
presentation and font registration are GUI-thread operations.

Tooltips select the active menu's Tooltip font when requested. Existing child
controls, including Qt scroll-area helpers, receive the shared Body font before
screen-specific roles are applied. Existing sprite lettering remains baked
into its original SPR images; it is not redrawn as Qt text.

## Native conversion policy and remaining boundaries

The outline bridge converts each opaque horizontal run to a clockwise rectangle
at 32 units per native pixel, preserving transparent holes and signed sprite
origins. Qt then provides antialiasing. It preserves opaque palette index zero;
raw palette RGB is deliberately not used as text colour because the original
palette conversion remains unverified. Existing menu foreground, disabled,
selection and focus colours are retained. The Yellow role denotes the source
font shapes; its pixels do not override the control's chosen foreground colour.
This is **mask-shape integration**, not reproduction of original coverage,
colour effects, shading or pixel-perfect rasterization.

Glyph ordinals map source bytes 33..255 to Unicode Latin-1 code points;
`yellowtext.sft` only covers 33..122. Space is invisible and uses the 'a' profile.
Ordinary glyph advances use the maximum trailing contour plus two native pixels
of tracking (or width when no profile exists). This is a shared native spacing
policy. The recovered engine's stateful preceding/current contour comparison,
punctuation adjustments, wrapping and code-page selection are not reproduced
or validated by this change. Qt controls keep underscore glyphs available for
file names/IDs rather than treating underscores as invisible engine spacing.
Unsupported Unicode uses Qt font merging, preserving user-supplied text and
editing semantics. Exact original font/layout/palette equivalence still needs
an original text-rendering oracle and paired screenshots.

The bridge bounds source input, frame dimensions/count, decoded pixels/profile
bytes, signed origins, line metrics, contour coordinates/count and font names.
Generated tables include Unicode mapping, per-glyph outlines, horizontal
metrics, naming and checksums. It does not expose arbitrary font file loading.

## Verification

- `qt-menu-fonts`: Qt acceptance of generated font; Latin-1/missing-glyph mapping;
  exact signed-origin/baseline outline bounds; transparent holes and space;
  profile advances; invalid metric rejection; explicit absent/corrupt catalog
  handling and shared scaling.
- Installed audit: all 21 menu variants, every child widget's font family and
  synthetic-bold state at 800x600, 400x300 and 1200x600; shared registration
  lifetime after one menu is destroyed, editable underscore/Unicode text and
  the tooltip role. Native fonts selected on every installed screen.
- All 24 targeted Qt menu/asset/audio/startup checks passed. Existing menu checks updated from the former arbitrary 26/20 pixel serif
  sizes to the recovered 32/22 role metrics, including half-size assertions.
- Main, Load Game, Preferences, Grimoire and host Lobby captures inspected under
  `working/tests/menu-fonts/`; letter shapes are readable and consistent with
  recovered masks. These captures do not establish original screenshot parity.

Run the repeatable installed audit from the repository root:

```bash
cmake -S apps/qt-shell -B working/build/qt-shell
cmake --build working/build/qt-shell --target menu-font-test --parallel 4
QT_QPA_PLATFORM=offscreen working/build/qt-shell/menu-font-test working/game-nocd
ctest --test-dir working/build/qt-shell -R qt-menu-fonts --output-on-failure
```

The immutable-input manifest passed before and after installed-asset inspection
(2,927 files). See [native SFT loading](../formats/sft-native-loading.md) and
[original reader/consumer findings](sft-font-loading.md) for the separate
confirmed schema and remaining original behavior questions.
