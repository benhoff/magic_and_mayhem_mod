# Real Surface2 loss and Restore observations

2026-10-06. High confidence within the captured Wine11.16 built-in driver,
Surface2 interface and isolated virtual-desktop setup. These captures execute
DirectDraw APIs without loading the game executable or running the native renderer.
They complement [original wrapper scheduling](original-surface-retry.md), which
executes unchanged original functions against scripted endpoints. Neither result
alone proves original wrappers composed with real driver recovery.

## Method and retained evidence

`tests/surface-driver-restore-reference.c` creates a registered top-level window,
shows/activates it and pumps messages with bounded iteration/time limits. The
collector reserves the existing single-Wine-session lock and refuses active
external Wine sessions. It copies a disposable prefix, forces built-in DirectDraw
and software GL, launches a Wine virtual desktop under isolated Xvfb, and waits
for that prefix's server to exit after cleanup. No original artifact is consumed.

Two retained captures request 16- and 32-bit display modes, each with four
independent surfaces created through IDirectDraw and queried as Surface2:

| ID | Requested caps | Format |
| --- | --- | --- |
| 0 | OFFSCREENPLAIN + SYSTEMMEMORY (`0x840`) | explicit RGB565, 8×6 |
| 1 | OFFSCREENPLAIN + VIDEOMEMORY (`0x4040`) | explicit RGB565, 8×6 |
| 2 | OFFSCREENPLAIN (`0x40`) | explicit RGB565, 8×6 |
| 3 | PRIMARYSURFACE (`0x200`) | driver-selected display format, 640×480 |

An initial successful Restore control, source key `0x5678` and full fill `0x1234`
establish the starting state. The probe changes 640×480 to 800×600, observes
IsLost/key/Lock, attempts fill `0xabcd`, and attempts Restore. It returns to
640×480, observes loss again, restores each surface, repeats Restore on usable
surfaces, then fills `0x9abc` and reads the defined outputs. GetDisplayMode and
successful Lock descriptors confirm the actual dimensions/formats; primary
surfaces are RGB565 in the 16-bit capture and XRGB32 in the 32-bit capture.

Every API result and successful 8×6 Lock sample is recorded as six u32 words.
The collector requires a terminal completion record: Wine explorer's process
exit alone does not prove the child probe completed. Failed development runs
remain under `working/tests/surface-driver-restore/` and are not passing evidence.

Retained reports:

- [16-bit capture](surface-driver-restore-16-capture-20261006.json)
- [32-bit capture](surface-driver-restore-32-capture-20261006.json)
- [offline verification](surface-driver-restore-offline-20261006.json)

## Confirmed findings

In both captures, the system-memory surface remains usable across both mode
changes and every Restore returns success. Video-memory, default offscreen and
primary surfaces report `DDERR_SURFACELOST` (`0x887601c2`) through both IsLost and
Lock after each mode change. Offscreen Restore succeeds even while the display
has different dimensions; primary Restore returns `DDERR_WRONGMODE`
(`0x8876024b`) at 800×600. After returning to 640×480, Restore succeeds for all
four surfaces. A repeated Restore on each usable surface also succeeds.

GetColorKey succeeds and returns the original key throughout, including while
IsLost/Lock report loss and after Restore. This is driver/interface-specific
retention evidence; it does not establish palette retention or justify deleting
the original wrapper's key reapplication.

The direct Blt COLORFILL calls return success even when IsLost/Lock report loss.
After successful Restore, sampled offscreen bytes contain the intervening fill.
Thus this probe cannot use a lost COLORFILL return to exercise an original
wrapper's lost-result branch. Captured allocation retention is an observation,
not a portable restored-byte contract. Native content invalidation remains an
intentional conservative policy requiring explicit writes/reload inputs.

The offline checker verifies capture and source fingerprints, all 2458 ordered
rows, HRESULT/key states, descriptor masks/dimensions, internal sample hashes,
and initial/final defined fill RGB bits. It excludes XRGB's unused high byte
from fill equivalence and imposes no expected bytes on post-Restore samples.
Eight tests cover provenance, completion, ordering, loss results, key retention,
defined pixels and acceptance of changed unspecified restored bytes.

## Reproduction and remaining boundaries

Run captures sequentially, choosing new report paths because retained evidence
must never be overwritten:

```bash
xvfb-run -a -s '-screen 0 1024x768x24' python3 tools/capture-surface-driver-restore.py --display-bpp 16 --report working/tests/restore-16-new.json
xvfb-run -a -s '-screen 0 1024x768x24' python3 tools/capture-surface-driver-restore.py --display-bpp 32 --report working/tests/restore-32-new.json
python3 tests/test-surface-driver-restore.py
python3 tools/check-surface-driver-restore.py --report working/tests/restore-offline-new.json
```

The retained corpus/checker runs entirely offline. Capture requires Wine/Xvfb.
Indexed surfaces and palette retention, clippers, attached flip chains, genuine
allocation destruction, wrong-format recovery, Windows-driver comparison,
asset/sentinel reloaders, unchanged wrapper composition with actual lost draw
results, and live recovery/replacement remain pending. No native implementation
or live status is promoted by this API-only evidence.

Primary-source context (separate from experimental evidence): Microsoft's
[Restore contract](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-restore)
does not promise reloading prior contents. Wine11.16's
[Surface2 Restore implementation](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/dlls/ddraw/surface.c)
delegates to its common Restore routine, which checks primary dimensions/format.
Its [device loss handling](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/dlls/ddraw/ddraw.c)
tracks activation and mode changes. These sources guided controls; captured
HRESULTs and bytes support the findings above.
