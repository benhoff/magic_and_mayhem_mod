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
FP/border/silhouette ordinals exist. Map variants at 640x480 also exist. PCX silhouette/border inputs and flags.ani
are now consumed by the [native visual integration](../runtime/realm-viewer-visuals.md).
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
Campaign IDs, ownership/banner IDs, availability, unlock conditions and original
movement/animation callbacks and control placement remain outside this asset
evidence. Native shape/animation consumption is described below.

## PCX and ANI visual inputs

Follow-up inspection, 2026-10-04: all 36 selected silhouette/border pairs decode
through the native PCX reader as 800x600 images with origin (0,0). Silhouettes
contain only indices zero and one: zero is blue background, one is magenta
membership. Border background palette indices vary (e.g. Celtic 01: 249,
Greek 01: 253, Medieval 01: 255), but their RGB is exactly (0,0,255). Border
images include region texture and a bright outline; they are not just thin lines.
The native display keys exact blue to transparency and uses silhouette indices
for hit testing. Original draw/callback keying remains unverified.

`Generic/flags.ani` is version five: 440 records, 101 sequences, associated
`Flags.spr`. Sequence zero holds eleven green-flag frames (0..10), sets delay
one and loops back to its start. Sequence 50 cycles twelve figure frames
(48..59) and loops with default delay zero. Their display metadata offsets are
respectively (-2,-47) and (-18,-41), relative to the display anchor; original
SPR origins remain part of placement. Native roles and timer rate are separate
from original campaign/action bindings. Unused sequence 96 has a relative jump
outside its own sequence range; the native viewer does not execute it.

The selected 36 FP files contain 75 paths/15,424 stored points, all in the
800x600 canvas. Each available selected region now displays a bounded native
preview of its first route. This does not establish original route choice,
slot reservation, ownership, speed or travel transitions. See
[visual policies and validation](../runtime/realm-viewer-visuals.md).
