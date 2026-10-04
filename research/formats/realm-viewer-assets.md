# Realm Viewer installed artwork and display metadata

Read-only inspection, 2026-10-04, bracketed by original-manifest verification.
Confidence is high for decoded files/text and the previously recovered FP input
contract; UI placement and campaign bindings are native interpretations.

| Installed relative input | Confirmed content |
|---|---|
| Interface/RealmViewer/800x600/Celtic_Map.bmp | 800x600 RGB map decoded through the native BMP loader |
| Interface/RealmViewer/800x600/Greek_Map.bmp | 800x600 RGB map decoded through the native BMP loader |
| Interface/RealmViewer/800x600/Medieval_Map.bmp | 800x600 RGB map decoded through the native BMP loader |
| Interface/RealmViewer/Generic/flags.spr | Version-4 RGB565, 89 frames; 0..10 visually show a green flag sequence, 11..47 illustrated banners, 48..88 figures/animation frames |
| Interface/RealmViewer/Generic/RlmBtn.spr | Version-4 RGB565, 12 frames; visual pairs 0/1 Cancel, 2/3 OK, 4/5 Spell Selection, 6/7 Spell Research, 8/9 Grimoire, 10/11 blank |
| Interface/RealmViewer/realmviewtooltip.cfg | HEADER ValidConfig=TRUE; STR_00..03 are Portmanteau, Grimoire, Character Improvement and Options |
| Interface/RealmViewer/realmviewtutorial.cfg | Four dialogs describe clicking another region, changing spells through Portmanteau, spending experience through Character Improvement and consulting Grimoire knowledge |
| Interface/RealmViewer/Generic/{realm}_Region_{ordinal}.txt | Installed Latin-1 region display names, e.g. Celtic 01 is The Forest of Pain; Greek 12 is Knossos; Medieval 16 is Dinas Emrys |
| Interface/RealmViewer/Generic/{realm}_FlagPath_{ordinal}.FP | Version-2 path data with four stored flag positions; [format and reader evidence](fp-native-loading.md) |

The application catalog reads 8 Celtic, 12 Greek and 16 Medieval name/FP pairs.
These ranges match the existing Region Entry image-selection bounds; they are
not evidence of the entire original campaign's region/state space. Other generic
FP/border/silhouette ordinals exist. Map variants at 640x480, PCX silhouette/border
inputs and flags.ani also exist; they are not consumed by this widget.
No dedicated Realm Viewer control-rectangle CFG was found in its directory.

The native catalog reads names through AssetStore with a 1 KiB bound, trims
outer whitespace, and requires nonempty single-line names up to 256 characters.
FP inputs use the existing native loader with 64 KiB input, 4,096 points and
32 KiB decoded-point limits. The first stored flag position must fit
X=0..752/Y=0..548. The native cell starts at (X,max(0,Y-52)), using a 48x52
canvas so its lower edge is near that stored point. This offset/top clamp is an
explicit preview policy, not recovered original scaling/drawing. The FP header
point, other position slots, paths and slot reservation are not UI state.

Original sprite RGB565 colours, opacity masks and signed origins are preserved.
Flag frame sizes span 15..41 by 36..44 pixels; RlmBtn frames alternate 94x39
with origin (0,0), and 93x38 with origin (-1,-1). Pairing visual counterparts
as normal versus hover/pressed is a native choice, not a recovered event mapping.
The shared Character icon comes from Sprites/Buttons.spr frames 0..2.
Campaign IDs, ownership/banner IDs, availability, unlock conditions, region
hit shapes, movement/animation and exact original fonts/control placement remain
outside this asset evidence.
