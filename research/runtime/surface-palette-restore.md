# Indexed Surface2 palette retention and explicit reload

2026-10-06. High confidence within the retained Wine11.16 built-in Surface2
capture and bounded native API tests. No game executable or original wrapper is
loaded. This extends [real driver Restore findings](surface-driver-restore.md)
and keeps [original wrapper scheduling](original-surface-retry.md) separate.

## Probe, inputs and evidence

`tests/surface-palette-restore-reference.c` uses an isolated Wine virtual desktop,
EXCLUSIVE|FULLSCREEN, an actual verified 8-bit display mode, two palettes created
with 8BIT|ALLOW256 (`0x44`), and four requested surface cap classes:
system-memory offscreen (`0x840`), video-memory offscreen (`0x4040`), default
offscreen (`0x40`) and primary (`0x200`). Offscreen surfaces are 8×6; the primary
is 640×480. Successful Lock descriptors confirm indexed8 and zero RGB masks.
The primary surface is sampled at its upper-left 8×6 region.

The probe requires foreground ownership after bounded message pumping, an
initial successful Restore control and a terminal completion record. The
collector uses the existing single-Wine-session reservation, refuses external
Wine sessions, copies a disposable prefix, and shuts down/waits for only that
prefix. No original artifacts are consumed. Failed setup runs remain under
`working/tests/surface-palette-restore/` and are excluded from passing evidence.

Palette A is shared by surfaces0/1/3; palette B belongs to surface2. Initial RGB
entries use deterministic input formulas, with flags zero. Each surface has key
`0x35` and full initial fill `0x12`. The display changes to 800×600 and palette A
entries37..43 are updated through the palette object while surfaces are lost.
After Restore attempts and returning to 640×480, each surface is restored and
Restore is repeated. A successful Lock explicitly writes **every index byte**:
`((y*8+x)*37 + surface_id*11 + 6*13) & 255`. Both palettes then receive full
updates. Surface1 is finally rebound to B; no pixel write follows that rebind.

- [Capture report](surface-palette-restore-capture-20261006.json): exact source,
  driver/probe/output hashes and portable compressed six-u32 operation records.
- [Offline verification](surface-palette-restore-offline-20261006.json): 8,233
  ordered records, 6,656 palette entries, 26 readable states and nine explicitly
  reloaded/update/rebind states. Nine refusal/provenance tests pass.
- [Native policy verification](surface-palette-restore-native-20261006.json):
  nine 8×6 cases, 45 unknown-content refusals, 18 index comparisons and 864
  input-derived RGB pixel comparisons.

The successful capture was first written under `working/tests/`; publication
relocates identical compressed bytes to the portable fixture. Its report retains
all source digest values and records the original report hash/path and corpus
path. This is artifact publication, not a new execution or hash refresh.

## Confirmed driver findings

Mode changes leave the system-memory surface usable. Video-memory, default
and primary surfaces return `DDERR_SURFACELOST` through IsLost, Lock **and
GetPalette**. Their palette identities/entries cannot be read through GetPalette
while lost. GetColorKey still succeeds and retains `0x35`.

The palette object's partial SetEntries call succeeds during loss. Offscreen
Restore succeeds at 800×600; primary Restore returns `DDERR_WRONGMODE`. After
returning to 640×480, Restore succeeds for every surface, and repeated Restore
succeeds. GetPalette then returns the same palette interface pointer as the
one originally bound, **within this run**. All 256 RGB entries match their input
state, including the intervening partial update. Surfaces sharing A observe that
update; B remains independent. There is no cross-launch pointer assumption.

Explicit index reloads match every sampled input byte. Full palette updates and
surface1's rebind alter the returned palette entries/binding while the index
bytes remain unchanged. Palette entries and indexed pixels are distinct state.
The lost COLORFILL calls still return success in this driver; this does not
exercise the original wrapper's lost-draw branch.

## Native policy validation

The test composes existing GlBlitter APIs; no engine or wire operation changes.
It creates an indexed8 surface, installs verified initial palette inputs and
invalidates contents. Reads/presentation refuse undefined indices. Applying
verified palette state does not make those indices known; a partial first-row
reload also leaves full read refused. A complete explicit reload permits read
and presentation. Subsequent full palette updates/rebinding inputs preserve
indices and produce the expected RGB values computed from those input arrays.

Captured palette entries/index bytes are inputs to the native test. Native RGB
presentation is compared with an input-derived model, **not independently
captured driver RGB output**. Native per-surface palette installation does not
claim shared COM palette identity or automatic driver recovery. Unspecified
post-Restore bytes are only internally hash-checked and never used as expected
restoration contents. Original asset callbacks and the `-1` sentinel reloader
remain unexecuted.

## Reproduction and boundaries

Captures require Wine/Xvfb and run serially. Choose new output paths; retained
reports and corpora cannot be overwritten. Offline checks require no Wine:

```bash
xvfb-run -a -s '-screen 0 1024x768x24' python3 tools/capture-surface-palette-restore.py --report working/tests/palette-new.json --corpus working/tests/palette-new.bin.gz
python3 tests/test-surface-palette-restore.py
python3 tools/check-surface-palette-restore.py --report working/tests/palette-offline-new.json
cmake -S renderer -B working/build/palette-restore -DBUILD_TESTING=OFF
cmake --build working/build/palette-restore --target mnm-renderer
xvfb-run -a python3 tools/check-native-palette-restore.py --report working/tests/palette-native-new.json
```

Original wrapper composition with actual lost draw results, actual asset/sentinel
reload, flag-bearing palettes, detach/final-release recovery, clippers, attached
flip chains, genuine allocation destruction, Windows-driver comparison and live
recovery/replacement remain pending. The captured driver findings do not change
native validity or promote original equivalence.

Primary-source API context, separate from captured findings:
[CreatePalette](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdraw7-createpalette),
[SetPalette](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-setpalette)
and [GetEntries](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawpalette-getentries).
These informed the inputs and COM ownership; retained results support the
Wine-specific loss/Restore findings above.
