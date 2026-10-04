# Realm Viewer shapes and animation

Reviewed 2026-10-04. Offline QWidget presentation; no original screen replacement
or engine-thread dispatch. Confidence is high for the decoded inputs and tested
native presentation policies; original caller equivalence remains unverified. Extends [Realm Viewer](realm-viewer-qt.md) and
[shared fonts](menu-font-integration.md).

## Native visual implementation

`apps/qt-shell/realm_viewer_visuals.{hpp,cpp}` loads the existing 8 Celtic,
12 Greek and 16 Medieval region catalogs through bounded native PCX, ANI and FP
readers. Every region has an 800x600 binary silhouette, an 800x600 border image
and owned path points. The widget loads the whole visual catalog before changing
an existing screen, preserving the prior model/art on any failed reload.

Silhouettes use index zero as background and index one as region membership.
Their original magenta/blue palette is diagnostic data, not visible map art.
Map hit testing uses the actual binary pixels, including disconnected pieces and
holes. Widget coordinates convert through the same letterboxed canvas used for
painting. Shared/overlapping mask pixels choose the lowest supplied artwork
ordinal; absent regions in the supplied campaign are excluded. Flags remain
independent keyboard-accessible shortcuts. Availability permits inspection but
continues to guard Open/Enter/double-click requests.

Selected and hovered regions draw the original border images over the map.
Exact RGB (0,0,255) is made transparent, regardless of its palette index; all
other palette colours are retained, including black. This keying matches the
observed installed image backgrounds, but is a native interpretation rather
than a recovered original draw callback. Masks and borders are cropped to their
nonempty bounding rectangles after decoding, reducing retained memory. The
selected flag rectangle is replaced by the map border; Qt's keyboard focus
rectangle and unavailable indicator remain visible on flag shortcuts.

ANI sequence zero provides the green flag's eleven frames. ANI sequence 50
provides twelve walking-figure frames for the path preview. These are explicit
native display roles, not recovered campaign ownership/action IDs. Caller-selected
static sprite ordinals other than zero retain their identity. The application
compiles only these two consumed sequences, supports display/delay/back-to-start
loop/terminal-stop records, validates their frame indices, offsets and control
flow, and leaves unrelated sequences unexecuted and the source asset unchanged. Other ANI opcodes and sequence roles remain separate.
Unused sequence 96 contains a stored back jump outside its own offset range;
this change does not normalize, execute or patch that sequence.

Each display record's signed X/Y displacement is combined with its SPR origin.
Flags convert the resulting foot-relative placement into the existing padded
48x52 button canvas using a local anchor at (2,48). All eleven installed flag
frames fit this canvas. The walking marker draws relative to its stored FP
point directly. A QTimer supplies one native presentation tick every 40 ms;
ANI delay N holds a frame for N+1 ticks. This timer rate is a preview policy,
not evidence of the original animation clock. Hidden menus stop their timer and
resume their retained animation phase when shown; no engine worker is involved.

The available selected region previews its first stored FP path using the
walking figure, one point every two native ticks, looping back to the first
point. Selection/realm/campaign changes reset this path phase. The marker draws
behind controls and cannot activate or move a region, change unlock state, or
commit campaign travel. Original path direction choice, position-slot reservation,
movement speed, character/direction animation selection and transition callbacks
remain unverified. This visible route preview consumes the recovered path data
without promoting it into campaign behavior.

## Bounds and validation

PCX inputs are limited to 2 MiB, exactly 800x600 with zero file origin, and
480,000 pixels. Binary silhouettes and nonempty keyed borders are required.
Each FP has at most 4,096 points/32 KiB decoded points/64 KiB input, with points
inside the 800x600 canvas. ANI input is limited to 64 KiB, 1,024 records and
128 sequences; its associated sprite filename must be `flags.spr`. Consumed
clips have bounded delay/displacement, valid frame indices and closed loop
control flow. The full catalog owns 36 shape/border pairs and 75 paths/15,424
stored points; it does not use process pointers or original slot state.

The synthetic `qt-realm-viewer-visuals` check covers mask boundaries/holes,
colour-key transparency, ANI holds/loops, hover/click/double-click, availability,
FP stepping/reset/loop, letterboxing/half scale, hidden timer lifetime, and
transactional PCX/ANI reload rejection and path-coordinate bounds. The original
Realm Viewer fixture now includes complete synthetic PCX/ANI/FP visual inputs.

All 25 targeted Qt menu/asset/audio/startup checks passed, along with installed
Realm Viewer/Main smoke checks and the 21-menu font audit.

The optional installed audit checks all 36 region IDs can be selected through
their actual masks at 800x600, 400x300 and 1200x600; verifies ANI frame catalogs
and flag placement bounds; and captures Celtic/Greek/Medieval highlights,
flags and moving markers. Captures are under `working/tests/realm-visuals/`.
Visual inspection caught foot-relative flag frames clipping above their button;
the cell-anchor conversion and installed bounds assertion now guard that case.
These captures are native evidence, not paired original screenshot equivalence.

```bash
cmake -S apps/qt-shell -B working/build/qt-shell
cmake --build working/build/qt-shell --target realm-viewer-visuals-test --parallel 4
QT_QPA_PLATFORM=offscreen working/build/qt-shell/realm-viewer-visuals-test working/game-nocd
ctest --test-dir working/build/qt-shell -R 'qt-realm-viewer' --output-on-failure
./tools/run-qt-shell.sh --realm-viewer
```

The immutable-input manifest verified all 2,927 original files before and after
installed-asset experiments. See [visual asset evidence](../formats/realm-viewer-assets.md),
[native PCX](../formats/pcx-native-loading.md),
[native ANI](../formats/ani-native-loading.md) and
[FP reader/caller findings](fp-flag-path-loading.md).
