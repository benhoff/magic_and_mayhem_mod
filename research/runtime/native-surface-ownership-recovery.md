# Owned surface identities, flipping and recovery

## Bounded result

Chunk 2 connects the GUI-thread `SurfaceBackend` to retained surface aliases,
shared palette objects, two-buffer attachments, storage swaps, explicit loss and
Restore, and the existing recovered retry scheduler. This is offline native
integration. It neither substitutes real COM pointers/HDCs nor replaces game work.

The independently captured standalone Wine Surface2 corpus records 5,647 ordered
six-word rows, 11 flips, surviving Surface1 aliases and normalized IUnknown
identities, caller-released bound palettes, shared partial updates, detach/rebind,
and attached-buffer reacquisition. Input formulas determine expected index bytes
and palette entries; observed outputs are not native replay inputs. Reference
count return values are diagnostic: native caller counts are a separate policy.
The collector reserves Wine exclusively, uses an isolated display/prefix, and
verifies the immutable manifest before and after execution. No original game
instructions run in this probe. The preliminary capture's inherited caps/catalog
metadata is superseded by the final capture; both raw records and hashes remain.

Evidence: `surface-ownership-final-capture-20261006.json`,
`surface-ownership-offline-20261006.json`,
`native-surface-ownership-final-20261006.json`,
`native-surface-ownership-backend-regression-20261006.json`, and
`native-surface-ownership-regressions-20261006.json`. Preliminary native recovery
and capture records remain historical; they cannot establish current freshness.

## Observed identities and storage leases

A Surface1 alias survives releasing the original Surface2 interface. Both
interfaces query the same IUnknown identity within the run. Bound surfaces retain
a palette after its caller releases it; GetPalette reacquires that identity.
Partial object updates reach both bindings; detaching one leaves the sibling
binding intact. Rebinding does not rewrite index bytes. Attached back buffers
survive external releases and can be reacquired. Native aliases normalize these
interfaces to one canonical, globally unique ID, with explicit caller counts and
attachment ownership. Native counts, reference budgets (1,024) and palette-object
budget (256) are intentional admission policies, not recovered COM return values.

For the sampled primary double buffer (640 x 480 indexed8, caps 0x218,
BACKBUFFERCOUNT=1), flipping exchanges native storage and validity. Keys and
palette bindings stay with their surface objects. Null and explicit attached-back
flip targets, flags 0 and WAIT=1, were captured. Native `flip(front, flags)` selects
the sole attached back buffer; it does not expose arbitrary target pointers.

| Held lease | Flip result | Return on original object | Return on opposite object | Flip back / eventual DC return |
| --- | --- | --- | --- | --- |
| Front CPU Lock | Success | NOTLOCKED 0x88760248 | Success | Not needed |
| Back CPU Lock | BUSY 0x887601ae **after swap** | NOTLOCKED | Success | Not needed |
| Front DC | Success | BADDC 0x8876086c | NODC 0x8876024a | BUSY after swap / Success |
| Back DC | BUSY **after swap** | BADDC | NODC | Success / Success |

Confidence is high within these captured schedules on Wine 11.16 software GL.
CPU snapshots and storage tokens move with the backing storage. Cached DC
identity stays with the surface object; the physical DC lease moves with storage.
A failed ReleaseDC preserves both states, and flipping storage back permits the
original cached handle to return. The backend represents these separately and
refuses raster access when cached DC and physical storage do not match. Resident
flips transfer no pixel uploads/readbacks; palette mirror updates are independent.
Flips after an unmatched Unlock during a DC lease refuse before mutation;
physical DC ownership still counts as borrowed even at zero mapping balance.
No Windows driver, timing, mixed leases, different-palette DC flips, longer chains,
or post-flip cached-DC/Lock interleaving equivalence is claimed.

## Restore, retry and supplied reload bytes

Loss invalidates content without discarding keys, aliases, palette identities or
attachments. GetPalette on lost surfaces returns LOST without mutating its output.
Offscreen Restore succeeds across mode changes; primary Restore requires an
explicit compatible mode and returns WRONGMODE otherwise. Successful Restore
leaves content unknown; healthy repeated Restore preserves explicit later writes.
Partial updates only define their written area. Reads/presentation/Lock reject
unknown pixels, and copies demand known source and keyed destination pixels.

The native test replays the existing independently captured indexed driver
recovery states: 6,656 palette entries, 864 defined index bytes and 66 loss/palette
statuses. Unspecified post-Restore bytes and cap-class emulation are excluded.
Standalone Wine COLORFILL can succeed while lost. The recovered wrapper adapter
intentionally admits a lost draw as LOST before calling fill, allowing controlled
recovery composition. That is a native policy, not proof of original wrappers
receiving physical lost-driver errors.

`OwnedSurfaceRetryAdapter` delegates to the unchanged `surface_retry.hpp` model,
using backend draw/Restore/key actions and explicit reload/report callbacks.
All 4,480 independently captured original fault schedules compare freshly;
1,536 budget-limited schedules preserve event prefixes and report exhaustion
without manufacturing success. These traces execute the recovered model offline;
the original execution remains the retained scripted-fault capture.

Sixteen native wrapper/reload combinations cover partial fill's no-retry branch,
full fill's single retry, source-before-destination recovery, exact-key copies,
missing content, and explicit reloads. Additional cases cover busy-budget ownership,
wrong-mode report order and reload failure propagation. Their 432 pixel checks
are input-derived native composition tests, not independent physical original
lost-draw outputs. A supplied indexed BMP traverses the existing sequential BMP
reader, fixed `bitmaps\\cursors.bmp` path, global cursor binding, GetDC/native DIB
raster/ReleaseDC callback. The triggering destination remains unknown. Actual
asset resolver/file I/O and live original GDI invocation are outside this test.

## Remaining boundaries

The next operation chunk is format and pitch closure: indexed/RGB32 draw/key
paths, narrower/wider pixel layouts and independent fixtures for required pitch
and format behavior. This ownership chunk does not complete that work. Also
pending are flagged/logical palettes, long flip chains, broader driver error
precedence/interleavings, negative mapping debt raster, Windows equivalence,
actual asset-loader failure internals, callback reentrancy/lifetime pinning,
physical original lost-draw composition, and live/wire replacement. Native final
release refuses active leases and lost-before-busy copy precedence is an explicit
conservative policy outside independently captured combinations.

Reproduce offline after building renderer tests:

```sh
python3 tools/check-surface-ownership.py --report working/tests/ownership-new.json
python3 -B tests/test-surface-ownership.py
xvfb-run -a python3 tools/check-native-surface-ownership.py \
  --build working/build/renderer --report working/tests/ownership-native-new.json
ctest --test-dir working/build/renderer --output-on-failure
```

Recapture only when no other Wine session is active; use new corpus and report
paths because retained evidence is immutable. Default prefix template comes from
`working/wineprefix-x86_64`; it is disposable input, not original media.
