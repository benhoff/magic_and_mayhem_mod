# Main menu layout inputs

Inspected 2026-10-04, read-only, in `working/game-nocd`. This records static
configuration evidence, not observed menu behavior or a native implementation.

`Interface/MainScreen/screen (MainMenu).cfg` is plaintext with semicolon
comments and section/key entries. `[GLOBALS] BackgroundFile="Main Menu"`
corresponds to these installed images:

- `Interface/MainScreen/640x480/Main Menu 640-480.JPG`
- `Interface/MainScreen/800x600/Main Menu 800-600.JPG`

The five large text buttons and one tiny text button declare the following
rectangles. Labels below are resolved by matching `Text=N` to `STR_NN` in
`CFG/interface screens text.cfg`, section `[STRINGS]`.

| Section | Text ID / label | Rect1 (640x480) | Rect2 (800x600) |
|---|---|---|---|
| TEXTBUTTON_1 | 0 / New Game | 120,248,520,280 | 150,310,650,350 |
| TEXTBUTTON_2 | 1 / Load Game | 120,288,520,320 | 150,360,650,400 |
| TEXTBUTTON_3 | 2 / Quick Battle | 120,328,520,360 | 150,410,650,450 |
| TEXTBUTTON_4 | 4 / Preferences | 120,368,520,400 | 150,460,650,500 |
| TEXTBUTTON_5 | 5 / Quit | 120,408,520,440 | 150,510,650,550 |
| TEXTBUTTON_6 | 6 / CommandLine Battle | 200,212,440,240 | 250,265,550,300 |

`[TEXT_1]` is marked as version text, uses `Font=TINY`, `TextFlags=RIGHT`,
and rectangles `480,452,632,472` / `600,565,790,590`. No version string is
specified in that section.

Confidence is high for the literal file contents and installed paths.
Interpreting the four coordinates as left/top/right/bottom is consistent with
the configuration and scaling, but endpoint inclusivity and actual hit testing
have not been recovered. Text-ID lookup is a strong static inference; the
engine lookup routine has not been traced. The presence of CommandLine Battle
does not establish that it is visible or enabled in ordinary launches. The
string table also contains Internet Battle at ID 3, but this layout has no
button referencing it. Font rendering, hover colors, keyboard shortcuts,
conditional controls and action dispatch remain unverified.

The installed SPR/SFT evaluator report at
`working/tests/mmsprite/run-j6vamgww/report.json` records parsing of 187 files
and 28 successful sampled frame decodes, including some font samples. It
explicitly records no comparison against original game output, no native
decoder and no live replacement. It is exploratory decoder evidence, not
proof of exact menu typography or palette behavior.

See [Qt migration assessment](../runtime/main-menu-qt-migration.md).
